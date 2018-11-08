/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __SENSOR_LIS3DH_H__
#define __SENSOR_LIS3DH_H__

#include <device.h>
#include <misc/util.h>
#include <zephyr/types.h>
#include <gpio.h>

#define LIS3DH_I2C_ADDRESS		CONFIG_LIS3DH_I2C_ADDR

#define LIS3DH_AUTOINCREMENT_ADDR	BIT(7)

#define LIS3DH_REG_CTRL1		0x20
#define LIS3DH_ACCEL_X_EN_BIT		BIT(0)
#define LIS3DH_ACCEL_Y_EN_BIT		BIT(1)
#define LIS3DH_ACCEL_Z_EN_BIT		BIT(2)
#define LIS3DH_ACCEL_EN_BITS (LIS3DH_ACCEL_X_EN_BIT | \
		LIS3DH_ACCEL_Y_EN_BIT | LIS3DH_ACCEL_Z_EN_BIT)

#define LIS3DH_TEMP_CFG_REG		0x1F
#define LIS3DH_TEMP_EN_BIT		BIT(6)
#define LIS3DH_ADC_EN_BIT		BIT(7)

#if defined(CONFIG_LIS3DH_POWER_MODE_LOW)
#define LIS3DH_TEMP_MAG		256
#elif defined(CONFIG_LIS3DH_POWER_MODE_NORMAL)
#define LIS3DH_TEMP_MAG		64
#endif

#if defined(CONFIG_LIS3DH_POWER_MODE_LOW)
#define LIS3DH_LP_EN_BIT	BIT(3)
#elif defined(CONFIG_LIS3DH_POWER_MODE_NORMAL)
#define LIS3DH_LP_EN_BIT	0
#endif

#if defined(CONFIG_LIS3DH_ODR_1)
	#define LIS3DH_ODR_IDX		1
#elif defined(CONFIG_LIS3DH_ODR_2)
	#define LIS3DH_ODR_IDX		2
#elif defined(CONFIG_LIS3DH_ODR_3)
	#define LIS3DH_ODR_IDX		3
#elif defined(CONFIG_LIS3DH_ODR_4)
	#define LIS3DH_ODR_IDX		4
#elif defined(CONFIG_LIS3DH_ODR_5)
	#define LIS3DH_ODR_IDX		5
#elif defined(CONFIG_LIS3DH_ODR_6)
	#define LIS3DH_ODR_IDX		6
#elif defined(CONFIG_LIS3DH_ODR_7)
	#define LIS3DH_ODR_IDX		7
#elif defined(CONFIG_LIS3DH_ODR_8)
	#define LIS3DH_ODR_IDX		8
#elif defined(CONFIG_LIS3DH_ODR_9_NORMAL) || defined(CONFIG_LIS3DH_ODR_9_LOW)
	#define LIS3DH_ODR_IDX		9
#endif

#define LIS3DH_ODR_SHIFT		4
#define LIS3DH_ODR_BITS			(LIS3DH_ODR_IDX << LIS3DH_ODR_SHIFT)
#define LIS3DH_ODR_MASK			(BIT_MASK(4) << LIS3DH_ODR_SHIFT)

#define LIS3DH_REG_CTRL3		0x22
#define LIS3DH_EN_DRDY1_INT1_SHIFT	4
#define LIS3DH_EN_DRDY1_INT1		BIT(LIS3DH_EN_DRDY1_INT1_SHIFT)

#define LIS3DH_REG_CTRL4		0x23
#define LIS3DH_FS_SHIFT			4
#define LIS3DH_FS_MASK			(BIT_MASK(2) << LIS3DH_FS_SHIFT)
#define LIS3DH_BDU_MASK			BIT(7)

#if defined(CONFIG_LIS3DH_FIFO_MODE_BYPASS)
	#define LIS3DH_FIFO_MODE	0
#elif defined(CONFIG_LIS3DH_FIFO_MODE_FIFO)
	#define LIS3DH_FIFO_MODE	1
#elif defined(CONFIG_LIS3DH_FIFO_MODE_STREAM)
	#define LIS3DH_FIFO_MODE	2
#elif defined(CONFIG_LIS3DH_FIFO_MODE_STREAMTOFIFO)
	#define LIS3DH_FIFO_MODE	3
#endif

#define LIS3DH_REG_FIFO_CTRL	0x2E
#define LIS3DH_FIFO_MODE_SHIFT	6
#define LIS3DH_FIFO_MODE_BITS	(LIS3DH_FIFO_MODE << LIS3DH_FIFO_MODE_SHIFT)
#define LIS3DH_FIFO_MODE_MASK	(BIT_MASK(2) << LIS3DH_FIFO_MODE_SHIFT)
#define LIS3DH_FIFO_WATERMARKLVL_MASK	BIT_MASK(4)
#define LIS3DH_LIR_INT2_SHIFT		1
#define LIS3DH_EN_LIR_INT2		BIT(LIS3DH_LIR_INT2_SHIFT)
#define LIS3DH_WATERMARK_LVL_MASK	BIT_MASK(5)
#define LIS3DH_WATERMARK_LVL		CONFIG_LIS3DH_FIFO_WATERMARK_LVL

#if defined(CONFIG_LIS3DH_ACCEL_RANGE_2G)
	#define LIS3DH_FS_IDX		0
#elif defined(CONFIG_LIS3DH_ACCEL_RANGE_4G)
	#define LIS3DH_FS_IDX		1
#elif defined(CONFIG_LIS3DH_ACCEL_RANGE_8G)
	#define LIS3DH_FS_IDX		2
#elif defined(CONFIG_LIS3DH_ACCEL_RANGE_16G)
	#define LIS3DH_FS_IDX		3
#endif

#define LIS3DH_FS_BITS			(LIS3DH_FS_IDX << LIS3DH_FS_SHIFT)
#define LIS3DH_ACCEL_SCALE		(SENSOR_G * (4 << LIS3DH_FS_IDX))

#define LIS3DH_REG_ACCEL_X_LSB		0x28
#define LIS3DH_REG_ACCEL_Y_LSB		0x2A
#define LIS3DH_REG_ACCEL_Z_LSB		0x2C
#define LIS3DH_REG_ACCEL_X_MSB		0x29
#define LIS3DH_REG_ACCEL_Y_MSB		0x2B
#define LIS3DH_REG_ACCEL_Z_MSB		0x2D

#define LIS3DH_REG_CTRL5		0x24
#define LIS3DH_FIFO_EN_BIT		BIT(6)
#define LIS3DH_LIR_INT2_SHIFT		1
#define LIS3DH_EN_LIR_INT2		BIT(LIS3DH_LIR_INT2_SHIFT)

#define LIS3DH_REG_CTRL6		0x25
#define LIS3DH_EN_ANYM_INT2_SHIFT	5
#define LIS3DH_EN_ANYM_INT2		BIT(LIS3DH_EN_ANYM_INT2_SHIFT)

#define LIS3DH_REG_STATUS		0x27

#define LIS3DH_REG_FIFO_SRC		0x2F
#define LIS3DH_FIFO_FLAG_WTM		BIT(7)
#define LIS3DH_FIFO_FLAG_OVR		BIT(6)
#define LIS3DH_FIFO_FLAG_EMPTY		BIT(5)
#define LIS3DH_FIFO_SAMPLES_MASK	BIT_MASK(5)

#define LIS3DH_REG_INT2_CFG		0x34
#define LIS3DH_REG_INT2_SRC		0x35
#define LIS3DH_REG_INT2_THS		0x36
#define LIS3DH_REG_INT2_DUR		0x37

#define LIS3DH_AOI_CFG			BIT(7)
#define LIS3DH_6D_CFG			BIT(6)
#define LIS3DH_INT_CFG_ZHIE_ZUPE	BIT(5)
#define LIS3DH_INT_CFG_ZLIE_ZDOWNE	BIT(4)
#define LIS3DH_INT_CFG_YHIE_YUPE	BIT(3)
#define LIS3DH_INT_CFG_YLIE_YDOWNE	BIT(2)
#define LIS3DH_INT_CFG_XHIE_XUPE	BIT(1)
#define LIS3DH_INT_CFG_XLIE_XDOWNE	BIT(0)

#define LIS3DH_BUF_SZ			7
#define LIS3DH_DATA_OFS			0

#define LIS3DH_REG_ADC_1_LSB		0x08
#define LIS3DH_REG_ADC_2_LSB		0x0A
#define LIS3DH_REG_ADC_3_LSB		0x0C
#define LIS3DH_REG_ADC_1_MSB		0x09
#define LIS3DH_REG_ADC_2_MSB		0x0B
#define LIS3DH_REG_ADC_3_MSB		0x0D



struct lis3dh_data {
	struct device *i2c;
#if defined(CONFIG_LIS3DH_FIFO_ENABLE)
	s16_t x_sample[32];
	s16_t y_sample[32];
	s16_t z_sample[32];
	u8_t fifo_flag_samples;
#else
	s16_t x_sample;
	s16_t y_sample;
	s16_t z_sample;
#endif

#if defined(CONFIG_LIS3DH_ENABLE_TEMP)
	s16_t temp_sample;
#endif

#if defined(CONFIG_LIS3DH_TRIGGER) || defined(CONFIG_LIS3DH_TRIGGER_MANUAL)
	struct device *gpio;
	struct gpio_callback gpio_int1_cb;
	struct gpio_callback gpio_int2_cb;

	sensor_trigger_handler_t handler_anymotion;
	sensor_trigger_handler_t handler_drdy;
	atomic_t trig_flags;
	enum sensor_channel chan_drdy;

#if defined(CONFIG_LIS3DH_TRIGGER_OWN_THREAD)
	K_THREAD_STACK_MEMBER(thread_stack, CONFIG_LIS3DH_THREAD_STACK_SIZE);
	struct k_thread thread;
	struct k_sem gpio_sem;
#elif defined(CONFIG_LIS3DH_TRIGGER_GLOBAL_THREAD) || defined(CONFIG_LIS3DH_TRIGGER_MANUAL)
	struct k_work work;
	struct device *dev;
#endif

#endif /* CONFIG_LIS3DH_TRIGGER */
};

int lis3dh_sample_fetch(struct device *dev, enum sensor_channel chan);
int lis3dh_sample_fetch_accel(struct device *dev);
#if defined(CONFIG_LIS3DH_ENABLE_TEMP)
int lis3dh_sample_fetch_temp(struct device *dev);
#endif

#if defined(CONFIG_LIS3DH_TRIGGER) || defined(CONFIG_LIS3DH_TRIGGER_MANUAL)
int lis3dh_trigger_set(struct device *dev,
		       const struct sensor_trigger *trig,
		       sensor_trigger_handler_t handler);

int lis3dh_init_interrupt(struct device *dev);

int lis3dh_acc_slope_config(struct device *dev, enum sensor_attribute attr,
                            const struct sensor_value *val);
#endif

#if defined(CONFIG_LIS3DH_FIFO_ENABLE)
void lis3dh_fifo_flags_get(struct device *dev);
#endif

#define SYS_LOG_DOMAIN "LIS3DH"
#define SYS_LOG_LEVEL CONFIG_SYS_LOG_SENSOR_LEVEL
#include <logging/sys_log.h>
#endif /* __SENSOR_LIS3DH__ */
