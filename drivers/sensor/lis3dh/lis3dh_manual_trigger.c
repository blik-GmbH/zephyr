/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <device.h>
#include <i2c.h>
#include <misc/util.h>
#include <kernel.h>
#include <sensor.h>

#include "lis3dh.h"

#define START_TRIG_INT1			BIT(0)
#define START_TRIG_INT2			BIT(1)
#define TRIGGED_INT1			BIT(4)
#define TRIGGED_INT2			BIT(5)

#define LIS3DH_ANYM_CFG (LIS3DH_6D_CFG | LIS3DH_INT_CFG_ZHIE_ZUPE | \
		LIS3DH_INT_CFG_YHIE_YUPE | LIS3DH_INT_CFG_XHIE_XUPE)

static int lis3dh_trigger_drdy_set(struct device *dev, enum sensor_channel chan,
				   sensor_trigger_handler_t handler)
{
	struct lis3dh_data *lis3dh = dev->driver_data;
	int status;

	gpio_pin_disable_callback(lis3dh->gpio, CONFIG_LIS3DH_INT1_GPIO_PIN);

	/* cancel potentially pending trigger */
	atomic_clear_bit(&lis3dh->trig_flags, TRIGGED_INT1);

	status = i2c_reg_update_byte(lis3dh->i2c, LIS3DH_I2C_ADDRESS,
				     LIS3DH_REG_CTRL3, LIS3DH_EN_DRDY1_INT1,
				     (0 << LIS3DH_EN_DRDY1_INT1_SHIFT));

	lis3dh->handler_drdy = handler;
	if ((handler == NULL) || (status < 0)) {
		return status;
	}

	lis3dh->chan_drdy = chan;

	/* serialize start of int1 in thread to synchronize output sampling
	 * and first interrupt. this avoids concurrent bus context access.
	 */
	atomic_set_bit(&lis3dh->trig_flags, START_TRIG_INT1);
#if defined(CONFIG_LIS3DH_TRIGGER_OWN_THREAD)
	k_sem_give(&lis3dh->gpio_sem);
#elif defined(CONFIG_LIS3DH_TRIGGER_GLOBAL_THREAD) || defined(CONFIG_LIS3DH_TRIGGER_MANUAL)
	k_work_submit(&lis3dh->work);
#endif

	return 0;
}

static int lis3dh_start_trigger_int1(const struct lis3dh_data *lis3dh)
{
	int status;
	u8_t raw[LIS3DH_BUF_SZ];
	u8_t ctrl1 = 0;

	/* power down temporarly to align interrupt & data output sampling */
	status = i2c_reg_read_byte(lis3dh->i2c, LIS3DH_I2C_ADDRESS,
				   LIS3DH_REG_CTRL1, &ctrl1);
	if (unlikely(status < 0)) {
		return status;
	}
	status = i2c_reg_write_byte(lis3dh->i2c, LIS3DH_I2C_ADDRESS,
				    LIS3DH_REG_CTRL1,
				    ctrl1 & ~LIS3DH_ODR_MASK);
	if (unlikely(status < 0)) {
		return status;
	}

	SYS_LOG_DBG("ctrl1=0x%x @tick=%u", ctrl1, k_cycle_get_32());

	/* empty output data */
	status = i2c_burst_read(lis3dh->i2c, LIS3DH_I2C_ADDRESS,
				(LIS3DH_REG_STATUS | LIS3DH_AUTOINCREMENT_ADDR),
				raw, sizeof(raw));

	if (unlikely(status < 0)) {
		return status;
	}

	gpio_pin_enable_callback(lis3dh->gpio, CONFIG_LIS3DH_INT1_GPIO_PIN);

	/* re-enable output sampling */
	status = i2c_reg_write_byte(lis3dh->i2c, LIS3DH_I2C_ADDRESS,
				    LIS3DH_REG_CTRL1, ctrl1);
	if (unlikely(status < 0)) {
		return status;
	}

	return i2c_reg_update_byte(lis3dh->i2c, LIS3DH_I2C_ADDRESS,
				   LIS3DH_REG_CTRL3, LIS3DH_EN_DRDY1_INT1,
				   (1 << LIS3DH_EN_DRDY1_INT1_SHIFT));
}

#define LIS3DH_EN_INT2_CFG (GPIO_DIR_IN | GPIO_INT |	  \
                            GPIO_INT_LEVEL | GPIO_INT_ACTIVE_HIGH |	  \
                            GPIO_INT_DEBOUNCE)

static int lis3dh_enable_gpio_int2(const struct lis3dh_data *lis3dh)
{
	int status;

	unsigned int key = irq_lock();

	status = gpio_pin_configure(lis3dh->gpio, CONFIG_LIS3DH_INT2_GPIO_PIN,
	                            LIS3DH_EN_INT2_CFG);
	if (status < 0) {
		SYS_LOG_ERR("Could not configure gpio %d",
			    CONFIG_LIS3DH_INT2_GPIO_PIN);
		return status;
	}

	status = gpio_pin_enable_callback(lis3dh->gpio,
	                                  CONFIG_LIS3DH_INT2_GPIO_PIN);
	if (status < 0) {
		SYS_LOG_ERR("Could not enable callback: %d", status);
		return status;
	}

	irq_unlock(key);

	return 0;
}

#define LIS3DH_DIS_INT2_CFG (GPIO_DIR_IN)
static int lis3dh_disable_gpio_int2(const struct lis3dh_data *lis3dh)
{
	int status;

	unsigned int key = irq_lock();

	status = gpio_pin_disable_callback(lis3dh->gpio,
	                                   CONFIG_LIS3DH_INT2_GPIO_PIN);
	if (status < 0) {
		SYS_LOG_ERR("Could not disable gpio int2 callback (%d)", status);
		return status;
	}

	gpio_pin_configure(lis3dh->gpio, CONFIG_LIS3DH_INT2_GPIO_PIN,
				    LIS3DH_DIS_INT2_CFG);
	irq_unlock(key);

	return 0;
}

static int lis3dh_trigger_anym_set(struct device *dev,
				   sensor_trigger_handler_t handler)
{
	struct lis3dh_data *lis3dh = dev->driver_data;
	int status;

	gpio_pin_disable_callback(lis3dh->gpio, CONFIG_LIS3DH_INT2_GPIO_PIN);

	lis3dh->handler_anymotion = handler;

	/* Setting the handler to NULL will disable the interrupt so
	 * that no interrupt wakes up the SoC
	 */
	if (lis3dh->handler_anymotion == NULL) {
		status = lis3dh_disable_gpio_int2(lis3dh);
	} else {
		status = lis3dh_enable_gpio_int2(lis3dh);
	}

	return status;
}

int lis3dh_trigger_set(struct device *dev,
		       const struct sensor_trigger *trig,
		       sensor_trigger_handler_t handler)
{
	if (trig->type == SENSOR_TRIG_DATA_READY &&
	    trig->chan == SENSOR_CHAN_ACCEL_XYZ) {
		return lis3dh_trigger_drdy_set(dev, trig->chan, handler);
	} else if (trig->type == SENSOR_TRIG_DELTA) {
		return lis3dh_trigger_anym_set(dev, handler);
	}

	return -ENOTSUP;
}

int lis3dh_acc_slope_config(struct device *dev, enum sensor_attribute attr,
			    const struct sensor_value *val)
{
	struct lis3dh_data *lis3dh = dev->driver_data;
	int status;

	if (attr == SENSOR_ATTR_SLOPE_TH) {
		u8_t range_g, reg_val;
		u32_t slope_th_ums2;

		status = i2c_reg_read_byte(lis3dh->i2c, LIS3DH_I2C_ADDRESS,
					   LIS3DH_REG_CTRL4, &reg_val);
		if (status < 0) {
			return status;
		}

		/* fs reg value is in the range 0 (2g) - 3 (16g) */
		range_g = 2 * (1 << ((LIS3DH_FS_MASK & reg_val)
				     >> LIS3DH_FS_SHIFT));

		slope_th_ums2 = val->val1 * 1000000 + val->val2;

		/* make sure the provided threshold does not exceed range */
		if ((slope_th_ums2 - 1) > (range_g * SENSOR_G)) {
			return -EINVAL;
		}

		/* 7 bit full range value */
		reg_val = 128 / range_g * (slope_th_ums2 - 1) / SENSOR_G;

		SYS_LOG_INF("int2_ths=0x%x range_g=%d ums2=%u", reg_val,
			    range_g, slope_th_ums2 - 1);

		status = i2c_reg_write_byte(lis3dh->i2c, LIS3DH_I2C_ADDRESS,
					    LIS3DH_REG_INT2_THS, reg_val);
	} else { /* SENSOR_ATTR_SLOPE_DUR */
		/*
		 * slope duration is measured in number of samples:
		 * N/ODR where N is the register value
		 */
		if (val->val1 < 0 || val->val1 > 127) {
			return -ENOTSUP;
		}

		SYS_LOG_INF("int2_dur=0x%x", val->val1);

		status = i2c_reg_write_byte(lis3dh->i2c, LIS3DH_I2C_ADDRESS,
					    LIS3DH_REG_INT2_DUR, val->val1);
	}

	return status;
}

static void lis3dh_gpio_int1_callback(struct device *dev,
				      struct gpio_callback *cb, u32_t pins)
{
	struct lis3dh_data *lis3dh =
		CONTAINER_OF(cb, struct lis3dh_data, gpio_int1_cb);

	ARG_UNUSED(pins);

	atomic_set_bit(&lis3dh->trig_flags, TRIGGED_INT1);

#if defined(CONFIG_LIS3DH_TRIGGER_OWN_THREAD)
	k_sem_give(&lis3dh->gpio_sem);
#elif defined(CONFIG_LIS3DH_TRIGGER_GLOBAL_THREAD) || defined(CONFIG_LIS3DH_TRIGGER_MANUAL)
	k_work_submit(&lis3dh->work);
#endif
}

static void lis3dh_gpio_int2_callback(struct device *dev,
				      struct gpio_callback *cb, u32_t pins)
{
	struct lis3dh_data *lis3dh =
		CONTAINER_OF(cb, struct lis3dh_data, gpio_int2_cb);

	ARG_UNUSED(pins);

	lis3dh_disable_gpio_int2(lis3dh);

	atomic_set_bit(&lis3dh->trig_flags, TRIGGED_INT2);

#if defined(CONFIG_LIS3DH_TRIGGER_OWN_THREAD)
	k_sem_give(&lis3dh->gpio_sem);
#elif defined(CONFIG_LIS3DH_TRIGGER_GLOBAL_THREAD) || defined(CONFIG_LIS3DH_TRIGGER_MANUAL)
	k_work_submit(&lis3dh->work);
#endif
}

static void lis3dh_thread_cb(void *arg)
{
	struct device *dev = arg;
	struct lis3dh_data *lis3dh = dev->driver_data;

	if (unlikely(atomic_test_and_clear_bit(&lis3dh->trig_flags,
					       START_TRIG_INT1))) {
		int status = lis3dh_start_trigger_int1(lis3dh);

		if (unlikely(status < 0)) {
			SYS_LOG_ERR("lis3dh_start_trigger_int1: %d", status);
		}
		return;
	}

	if (atomic_test_and_clear_bit(&lis3dh->trig_flags,
				      TRIGGED_INT1)) {
		struct sensor_trigger drdy_trigger = {
			.type = SENSOR_TRIG_DATA_READY,
			.chan = lis3dh->chan_drdy,
		};

		if (likely(lis3dh->handler_drdy != NULL)) {
			lis3dh->handler_drdy(dev, &drdy_trigger);
		}

		return;
	}

	if (atomic_test_and_clear_bit(&lis3dh->trig_flags,
				      TRIGGED_INT2)) {
		struct sensor_trigger anym_trigger = {
			.type = SENSOR_TRIG_DELTA,
			.chan = lis3dh->chan_drdy,
		};
		u8_t reg_val;
		int status;

		/* clear interrupt 2 to de-assert int2 line */
		status = i2c_reg_read_byte(lis3dh->i2c, LIS3DH_I2C_ADDRESS,
		                           LIS3DH_REG_INT2_SRC, &reg_val);
		if (unlikely(status < 0)) {
			SYS_LOG_ERR("Could not clear interrupt (%d)\n", status);
			return;
		}

		if (likely(lis3dh->handler_anymotion != NULL)) {
			lis3dh->handler_anymotion(dev, &anym_trigger);
		}

		SYS_LOG_DBG("@tick=%u int2_src=0x%x", k_cycle_get_32(),
			    reg_val);

		return;
	}
}

#ifdef CONFIG_LIS3DH_TRIGGER_OWN_THREAD
static void lis3dh_thread(void *arg1, void *unused2, void *unused3)
{
	struct device *dev = arg1;
	struct lis3dh_data *lis3dh = dev->driver_data;

	ARG_UNUSED(unused2);
	ARG_UNUSED(unused3);

	while (1) {
		k_sem_take(&lis3dh->gpio_sem, K_FOREVER);
		lis3dh_thread_cb(dev);
	}
}
#endif

#if defined(CONFIG_LIS3DH_TRIGGER_GLOBAL_THREAD) || defined(CONFIG_LIS3DH_TRIGGER_MANUAL)
static void lis3dh_work_cb(struct k_work *work)
{
	struct lis3dh_data *lis3dh =
		CONTAINER_OF(work, struct lis3dh_data, work);

	lis3dh_thread_cb(lis3dh->dev);
}
#endif

#define LIS3DH_INT1_CFG                 (GPIO_DIR_IN | GPIO_INT |		\
					 GPIO_INT_EDGE | GPIO_INT_ACTIVE_HIGH |	\
					 GPIO_INT_DEBOUNCE)

int lis3dh_init_interrupt(struct device *dev)
{
	struct lis3dh_data *lis3dh = dev->driver_data;
	int status;

	/* setup data ready gpio interrupt */
	lis3dh->gpio = device_get_binding(CONFIG_LIS3DH_GPIO_DEV_NAME);
	if (lis3dh->gpio == NULL) {
		SYS_LOG_ERR("Cannot get pointer to %s device",
			    CONFIG_LIS3DH_GPIO_DEV_NAME);
		return -EINVAL;
	}

	/* data ready int1 gpio configuration */
	status = gpio_pin_configure(lis3dh->gpio, CONFIG_LIS3DH_INT1_GPIO_PIN,
				    LIS3DH_INT1_CFG);
	if (status < 0) {
		SYS_LOG_ERR("Could not configure gpio %d",
			    CONFIG_LIS3DH_INT1_GPIO_PIN);
		return status;
	}

	gpio_init_callback(&lis3dh->gpio_int1_cb,
			   lis3dh_gpio_int1_callback,
			   BIT(CONFIG_LIS3DH_INT1_GPIO_PIN));

	status = gpio_add_callback(lis3dh->gpio, &lis3dh->gpio_int1_cb);
	if (status < 0) {
		SYS_LOG_ERR("Could not add gpio int1 callback");
		return status;
	}

	/* Setting the trigger will later configure the pin for interrupts */
	status = gpio_pin_configure(lis3dh->gpio, CONFIG_LIS3DH_INT2_GPIO_PIN,
				    LIS3DH_DIS_INT2_CFG);
	if (status < 0) {
		SYS_LOG_ERR("Could not configure gpio %d",
			    CONFIG_LIS3DH_INT2_GPIO_PIN);
		return status;
	}

	gpio_init_callback(&lis3dh->gpio_int2_cb,
			   lis3dh_gpio_int2_callback,
			   BIT(CONFIG_LIS3DH_INT2_GPIO_PIN));

	/* callback is going to be enabled by trigger setting function */
	status = gpio_add_callback(lis3dh->gpio, &lis3dh->gpio_int2_cb);
	if (status < 0) {
		SYS_LOG_ERR("Could not add gpio int2 callback (%d)", status);
		return status;
	}

#if defined(CONFIG_LIS3DH_TRIGGER_OWN_THREAD)
	k_sem_init(&lis3dh->gpio_sem, 0, UINT_MAX);

	k_thread_create(&lis3dh->thread, lis3dh->thread_stack,
			CONFIG_LIS3DH_THREAD_STACK_SIZE,
			(k_thread_entry_t)lis3dh_thread, dev, NULL, NULL,
			K_PRIO_COOP(CONFIG_LIS3DH_THREAD_PRIORITY), 0, 0);
#elif defined(CONFIG_LIS3DH_TRIGGER_GLOBAL_THREAD) || defined(CONFIG_LIS3DH_TRIGGER_MANUAL)
	lis3dh->work.handler = lis3dh_work_cb;
	lis3dh->dev = dev;
#endif
	/* disable interrupt 2 in case of warm (re)boot */
	status = i2c_reg_write_byte(lis3dh->i2c, LIS3DH_I2C_ADDRESS,
				    LIS3DH_REG_INT2_CFG, 0);
	if (status < 0) {
		SYS_LOG_ERR("Interrupt 2 disable reg write failed (%d)",
			    status);
		return status;
	}

	status = i2c_reg_write_byte(lis3dh->i2c, LIS3DH_I2C_ADDRESS,
				    LIS3DH_REG_INT2_THS, 0);
	if (status < 0) {
		SYS_LOG_ERR("Failed to set INT2_THS (%d)", status);
		return status;
	}
	status = i2c_reg_write_byte(lis3dh->i2c, LIS3DH_I2C_ADDRESS,
				    LIS3DH_REG_INT2_DUR, 0);
	if (status < 0) {
		SYS_LOG_ERR("Failed to set INT2_DUR (%d)", status);
		return status;
	}

	/* latch int2 line interrupt */
	status = i2c_reg_write_byte(lis3dh->i2c, LIS3DH_I2C_ADDRESS,
				    LIS3DH_REG_CTRL5, LIS3DH_EN_LIR_INT2);
	if (status < 0) {
		SYS_LOG_ERR("INT2 latch enable reg write failed (%d)", status);
		return status;
	}


	/* enable interrupt 2 on int2 line */
	status = i2c_reg_write_byte(lis3dh->i2c, LIS3DH_I2C_ADDRESS,
	                          LIS3DH_REG_CTRL6, LIS3DH_EN_ANYM_INT2);
	if (status < 0) {
		SYS_LOG_ERR("INT2 enable failed (%d)", status);
		return status;
	}

	/* configure interrupt mode for int2*/
	return i2c_reg_write_byte(lis3dh->i2c, LIS3DH_I2C_ADDRESS,
				  LIS3DH_REG_INT2_CFG, LIS3DH_ANYM_CFG);
}
