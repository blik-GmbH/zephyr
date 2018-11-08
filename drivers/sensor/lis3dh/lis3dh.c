/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <i2c.h>
#include <init.h>
#include <sensor.h>
#include <misc/__assert.h>

#include "lis3dh.h"

K_SEM_DEFINE(sem_lis3dh_fifo_ovr, 0, 1);
K_SEM_DEFINE(sem_lis3dh_fifo_wtm, 0, 1);
K_SEM_DEFINE(sem_lis3dh_fifo_empty, 0, 1);
K_SEM_DEFINE(sem_lis3dh_fifo_fetched, 0, 32);

static void lis3dh_convert(struct sensor_value *val, s64_t raw_val)
{
	/* val = raw_val * LIS3DH_ACCEL_SCALE / (10^6 * (2^16 - 1)) */
	raw_val = raw_val * LIS3DH_ACCEL_SCALE / 1000000;
	val->val1 = raw_val / 0xFFFF;
	val->val2 = (raw_val % 0xFFFF) * 1000000 / 0xFFFF;

	/* normalize val to make sure val->val2 is positive */
	if (val->val2 < 0) {
		val->val1 -= 1;
		val->val2 += 1000000;
	}
}

static int lis3dh_channel_get(struct device *dev,
			      enum sensor_channel chan,
			      struct sensor_value *val)
{
	struct lis3dh_data *drv_data = dev->driver_data;
	u8_t fifo_position;

	switch (chan) {
#if defined(CONFIG_LIS3DH_FIFO_ENABLE)

	/* return one value of the FIFO. To read the whole
	 * FIFO sensor_channel_get has to be called 32 times
	 */
	case SENSOR_CHAN_ACCEL_X:
		/* Take semaphore to read next value from FIFO in the next call
		 * channel get
		 */
		if (k_sem_take(&sem_lis3dh_fifo_fetched, K_MSEC(1)) != 0) {

			SYS_LOG_DBG("Can not take fifo_fetched sem.");
			return -EIO;
		}
		/*save value of semaphore*/
		fifo_position = k_sem_count_get(&sem_lis3dh_fifo_fetched);
		lis3dh_convert(val, drv_data->x_sample[fifo_position]);
		break;

	case SENSOR_CHAN_ACCEL_Y:
		/* Take semaphore to read next value from FIFO in the next call
		 * channel get
		 */
		if (k_sem_take(&sem_lis3dh_fifo_fetched, K_MSEC(1)) != 0) {

			SYS_LOG_DBG("Can not take fifo_fetched sem.");
			return -EIO;
		}
		/*save value of semaphore*/
		fifo_position = k_sem_count_get(&sem_lis3dh_fifo_fetched);
		lis3dh_convert(val, drv_data->y_sample[fifo_position]);
		break;

	case SENSOR_CHAN_ACCEL_Z:
		/* Take semaphore to read next value from FIFO in the next call
		 * channel get
		 */
		if (k_sem_take(&sem_lis3dh_fifo_fetched, K_MSEC(1)) != 0) {

			SYS_LOG_DBG("Can not take fifo_fetched sem.");
			return -EIO;
		}
		/*save value of semaphore*/
		fifo_position = k_sem_count_get(&sem_lis3dh_fifo_fetched);
		lis3dh_convert(val, drv_data->z_sample[fifo_position]);
		break;

	case SENSOR_CHAN_ACCEL_XYZ:
		/* Take semaphore to read next value from FIFO in the next call
		 * channel get
		 */
		if (k_sem_take(&sem_lis3dh_fifo_fetched, K_MSEC(1)) != 0) {
			SYS_LOG_DBG("Can not take fifo_fetched sem.");
			return -EIO;
		}
		/*save value of semaphore*/
		fifo_position = k_sem_count_get(&sem_lis3dh_fifo_fetched);

		lis3dh_convert(val, drv_data->x_sample[fifo_position]);
		lis3dh_convert(val + 1, drv_data->y_sample[fifo_position]);
		lis3dh_convert(val + 2, drv_data->z_sample[fifo_position]);
		break;
#else
	case SENSOR_CHAN_ACCEL_X:
		lis3dh_convert(val, drv_data->x_sample);
		break;
	case  SENSOR_CHAN_ACCEL_Y:
		lis3dh_convert(val, drv_data->y_sample);
		break;
	case SENSOR_CHAN_ACCEL_Z:
		lis3dh_convert(val, drv_data->z_sample);
		break;
	case SENSOR_CHAN_ACCEL_XYZ:
		lis3dh_convert(val, drv_data->x_sample);
		lis3dh_convert(val + 1, drv_data->y_sample);
		lis3dh_convert(val + 2, drv_data->z_sample);
		break;
#endif
#if defined(CONFIG_LIS3DH_ENABLE_TEMP)
	case SENSOR_CHAN_TEMP:
		val->val1 = drv_data->temp_sample;
		val->val2 = 0;
		break;
#endif
	default:
		return -ENOTSUP;
	}

	return 0;
}

int lis3dh_sample_fetch(struct device *dev, enum sensor_channel chan)
{
	switch (chan) {
	case SENSOR_CHAN_ACCEL_XYZ:
		lis3dh_sample_fetch_accel(dev);
		break;
#if defined(CONFIG_LIS3DH_ENABLE_TEMP)
	case SENSOR_CHAN_TEMP:
		lis3dh_sample_fetch_temp(dev);
		break;
#endif
	case SENSOR_CHAN_ALL:
		lis3dh_sample_fetch_accel(dev);
#if defined(CONFIG_LIS3DH_ENABLE_TEMP)
		lis3dh_sample_fetch_temp(dev);
#endif
		break;
	default:
		return -ENOTSUP;
	}

	return 0;
}

int lis3dh_sample_fetch_accel(struct device *dev)
{
	struct lis3dh_data *drv_data = dev->driver_data;
	int rc;
#if defined(CONFIG_LIS3DH_FIFO_ENABLE)
	u8_t fifo_bytes = 192;

	u8_t buf[fifo_bytes];
	/*check if FIFO is full*/
	lis3dh_fifo_flags_get(dev);

	/* only read FIFO when the watermark level has been reached.
	 * Alternatively the overrun flag can be checked.
	 */
	if (k_sem_take(&sem_lis3dh_fifo_ovr, K_NO_WAIT) != 0) {
		SYS_LOG_DBG(
		"Tried to read FIFO that is not full.");
		return -EIO;
	}

	/*
	 * since all accel data register addresses are consecutive,
	 * a burst read can be used to read all the samples
	 */
	rc = i2c_burst_read(drv_data->i2c, LIS3DH_I2C_ADDRESS,
			   (LIS3DH_REG_ACCEL_X_LSB | LIS3DH_AUTOINCREMENT_ADDR),
			   buf, fifo_bytes);
	if (rc != 0) {
		SYS_LOG_DBG("Could not read accel axis data");
		return -EIO;
	}

	/*Write the 192 byte long buffer to 32* 2 byte array*/
	for (u16_t i = 0; i < 32; i++) {
		u16_t k = 6*i;

		drv_data->x_sample[i] = (buf[k+1] << 8) | buf[k];
		drv_data->y_sample[i] = (buf[k+3] << 8) | buf[k+2];
		drv_data->z_sample[i] = (buf[k+5] << 8) | buf[k+4];

	}

	/* if the FIFO is used in FIFO Mode, it has to be reset
	 * by setting it to Bypass Mode and back to FIFO Mode to
	 * continue the writing to the FIFO
	 */
#if defined(CONFIG_LIS3DH_FIFO_MODE_FIFO)
	/* set to Bypass mode*/
	rc = i2c_reg_write_byte(drv_data->i2c, LIS3DH_I2C_ADDRESS,
			LIS3DH_REG_FIFO_CTRL,
			(LIS3DH_FIFO_MODE_MASK & 0U))
	if (rc != 0) {
		SYS_LOG_DBG("Failed to reset FIFO Mode");
	}
	/*set back to FIFO mode*/
	rc = i2c_reg_write_byte(drv_data->i2c, LIS3DH_I2C_ADDRESS,
				LIS3DH_REG_FIFO_CTRL,
				(LIS3DH_FIFO_MODE_MASK & LIS3DH_FIFO_MODE_BITS))
	if (rc != 0) {
		SYS_LOG_DBG("Failed to reset FIFO Mode");
	}

#endif



	/*set the fifo_fetched semaphore to 32 to indicate that
	 * a full fifo sample was fetched
	 */
	k_sem_init(&sem_lis3dh_fifo_fetched, 32, 32);

#else
	u8_t buf[6];

	/*
	 * since all accel data register addresses are consecutive,
	 * a burst read can be used to read all the samples
	 */
	rc = i2c_burst_read(drv_data->i2c, LIS3DH_I2C_ADDRESS,
			   (LIS3DH_REG_ACCEL_X_LSB | LIS3DH_AUTOINCREMENT_ADDR),
			   buf, 6)
	if (rc != 0) {
		SYS_LOG_DBG("Could not read accel axis data");
		return -EIO;
	}

	drv_data->x_sample = (buf[1] << 8) | buf[0];
	drv_data->y_sample = (buf[3] << 8) | buf[2];
	drv_data->z_sample = (buf[5] << 8) | buf[4];
#endif

	return 0;
}

static int lis3dh_acc_config(struct device *dev, enum sensor_channel chan,
			     enum sensor_attribute attr,
                             const struct sensor_value *val)
{
	switch (attr) {
#if defined(CONFIG_LIS3DH_TRIGGER) || defined(CONFIG_LIS3DH_TRIGGER_MANUAL)
	case SENSOR_ATTR_SLOPE_TH:
	case SENSOR_ATTR_SLOPE_DUR:
		return lis3dh_acc_slope_config(dev, attr, val);
#endif
	default:
		SYS_LOG_DBG("Accel attribute not supported.");
		return -ENOTSUP;
	}

	return 0;
}

static int lis3dh_attr_set(struct device *dev, enum sensor_channel chan,
                           enum sensor_attribute attr,
                           const struct sensor_value *val)
{
	switch (chan) {
	case SENSOR_CHAN_ACCEL_X:
	case SENSOR_CHAN_ACCEL_Y:
	case SENSOR_CHAN_ACCEL_Z:
	case SENSOR_CHAN_ACCEL_XYZ:
		return lis3dh_acc_config(dev, chan, attr, val);
	default:
		SYS_LOG_WRN("attr_set() not supported on this channel.");
		return -ENOTSUP;
	}

	return 0;
}

#if defined(CONFIG_LIS3DH_ENABLE_TEMP)
int lis3dh_sample_fetch_temp(struct device *dev)
{
	struct lis3dh_data *drv_data = dev->driver_data;
	u8_t buf[2];
	int rc;

	rc = i2c_burst_read(drv_data->i2c, LIS3DH_I2C_ADDRESS,
			   (LIS3DH_REG_ADC_3_LSB | LIS3DH_AUTOINCREMENT_ADDR),
			   buf, sizeof(buf));
	if (rc != 0) {
		SYS_LOG_DBG("Could not read temperature data");
		return -EIO;
	}

	s16_t raw = ((buf[1] << 8) | buf [0]);

	/*
	 * The data is represented as left-aligned 2's
	 * complement.
	 * In low power mode the value is 8 bit,
	 * in normal mode 10 bit.
	 * The conversion rate is 1 digit/°C.
	 */
	drv_data->temp_sample = raw / LIS3DH_TEMP_MAG;

	return 0;
}
#endif

static const struct sensor_driver_api lis3dh_driver_api = {
	.attr_set = lis3dh_attr_set,
#if defined(CONFIG_LIS3DH_TRIGGER) || defined(CONFIG_LIS3DH_TRIGGER_MANUAL)
	.trigger_set = lis3dh_trigger_set,
#endif
	.sample_fetch = lis3dh_sample_fetch,
	.channel_get = lis3dh_channel_get,
};

int lis3dh_init(struct device *dev)
{
	struct lis3dh_data *drv_data = dev->driver_data;
	int rc;

	drv_data->i2c = device_get_binding(CONFIG_LIS3DH_I2C_MASTER_DEV_NAME);
	if (drv_data->i2c == NULL) {
		SYS_LOG_DBG("Could not get pointer to %s device",
		    CONFIG_LIS3DH_I2C_MASTER_DEV_NAME);
		return -EINVAL;
	}

	/* enable accel measurements and set power mode and data rate */
	rc = i2c_reg_write_byte(drv_data->i2c, LIS3DH_I2C_ADDRESS,
			       LIS3DH_REG_CTRL1, LIS3DH_ACCEL_EN_BITS |
			       LIS3DH_LP_EN_BIT | LIS3DH_ODR_BITS);
	if (rc != 0) {
		SYS_LOG_DBG("Failed to configure chip.");
	}

	/* set full scale range */
	rc = i2c_reg_write_byte(drv_data->i2c, LIS3DH_I2C_ADDRESS,
			       LIS3DH_REG_CTRL4, LIS3DH_FS_BITS);
	if (rc != 0) {
		SYS_LOG_DBG("Failed to set full scale range.");
		return -EIO;
	}

#if defined(CONFIG_LIS3DH_ENABLE_TEMP)
	/* enable block data update (BDU) to use the auxiliary ADC */
	rc = i2c_reg_write_byte(drv_data->i2c, LIS3DH_I2C_ADDRESS,
				LIS3DH_REG_CTRL4,
				LIS3DH_BDU_MASK | LIS3DH_FS_BITS);
	if (rc != 0) {
		SYS_LOG_DBG("Failed to set BDU-bit.");
		return -EIO;
	}

	/* connect temperature sensor to channel 3 of the ADC and
	 * enable temperature measurement */
	rc = i2c_reg_write_byte(drv_data->i2c, LIS3DH_I2C_ADDRESS,
			       LIS3DH_TEMP_CFG_REG,
			       LIS3DH_TEMP_EN_BIT | LIS3DH_ADC_EN_BIT);
	if (rc != 0) {
		SYS_LOG_DBG("Failed to enable temperature measurements.");
		return -EIO;
	}
#endif

#if defined(CONFIG_LIS3DH_TRIGGER) || defined(CONFIG_LIS3DH_TRIGGER_MANUAL)
	if (lis3dh_init_interrupt(dev) < 0) {
		SYS_LOG_DBG("Failed to initialize interrupts.");
		return -EIO;
	}
#endif
#if defined(CONFIG_LIS3DH_FIFO_ENABLE)

	/*enable FIFO*/
	rc = i2c_reg_write_byte(drv_data->i2c, LIS3DH_I2C_ADDRESS,
				       LIS3DH_REG_CTRL5,
				       LIS3DH_FIFO_EN_BIT);
	if (rc != 0) {
		SYS_LOG_DBG("Failed to enable FIFO");
	}

	/*set FIFO MODE and Watermark Level*/
	rc = i2c_reg_write_byte(drv_data->i2c, LIS3DH_I2C_ADDRESS,
			LIS3DH_REG_FIFO_CTRL,
			(LIS3DH_FIFO_MODE_MASK & LIS3DH_FIFO_MODE_BITS)
			| (LIS3DH_WATERMARK_LVL & LIS3DH_WATERMARK_LVL_MASK));
	if (rc != 0) {
		SYS_LOG_DBG("Failed to set FIFO Mode");
	}


#endif
	return 0;
}

#if defined(CONFIG_LIS3DH_FIFO_ENABLE)
void lis3dh_fifo_flags_get(struct device *dev)
{
	u8_t reg_val;
	struct lis3dh_data *drv_data = dev->driver_data;

	if (i2c_reg_read_byte(drv_data->i2c, LIS3DH_I2C_ADDRESS,
					    LIS3DH_REG_FIFO_SRC, &reg_val) < 0){
		SYS_LOG_DBG("Failed to read FIFO status register");
		return;
	}
	/* set semaphores according to flags*/
	if (reg_val & LIS3DH_FIFO_FLAG_WTM)  {
		k_sem_give(&sem_lis3dh_fifo_wtm);
	} else{
		k_sem_reset(&sem_lis3dh_fifo_wtm);
	}

	if (reg_val & LIS3DH_FIFO_FLAG_OVR) {
		k_sem_give(&sem_lis3dh_fifo_ovr);
	} else{
		k_sem_reset(&sem_lis3dh_fifo_ovr);
	}

	if (reg_val & LIS3DH_FIFO_FLAG_EMPTY) {
		k_sem_give(&sem_lis3dh_fifo_empty);
	} else{
		k_sem_reset(&sem_lis3dh_fifo_empty);
	}

	/* write the amount of samples in the fifo to
	 * dev->fifo_flag_samples
	 */
	drv_data->fifo_flag_samples = (reg_val & LIS3DH_FIFO_SAMPLES_MASK);
	SYS_LOG_DBG("fifo flags: %d\n", reg_val);

}
#endif

struct lis3dh_data lis3dh_driver;

DEVICE_AND_API_INIT(lis3dh, CONFIG_LIS3DH_NAME, lis3dh_init, &lis3dh_driver,
		    NULL, POST_KERNEL, CONFIG_SENSOR_INIT_PRIORITY,
		    &lis3dh_driver_api);
