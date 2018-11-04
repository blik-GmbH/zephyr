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

	if (chan == SENSOR_CHAN_ACCEL_X) {
		lis3dh_convert(val, drv_data->x_sample);
	} else if (chan == SENSOR_CHAN_ACCEL_Y) {
		lis3dh_convert(val, drv_data->y_sample);
	} else if (chan == SENSOR_CHAN_ACCEL_Z) {
		lis3dh_convert(val, drv_data->z_sample);
	} else if (chan == SENSOR_CHAN_ACCEL_XYZ) {
		lis3dh_convert(val, drv_data->x_sample);
		lis3dh_convert(val + 1, drv_data->y_sample);
		lis3dh_convert(val + 2, drv_data->z_sample);
#if defined(CONFIG_LIS3DH_ENABLE_TEMP)
	} else if (chan == SENSOR_CHAN_TEMP) {
		val->val1 = drv_data->temp_sample;
		val->val2 = 0;
#endif
	} else {
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
	u8_t buf[6];

	/*
	 * since all accel data register addresses are consecutive,
	 * a burst read can be used to read all the samples
	 */
	if (i2c_burst_read(drv_data->i2c, LIS3DH_I2C_ADDRESS,
			   (LIS3DH_REG_ACCEL_X_LSB | LIS3DH_AUTOINCREMENT_ADDR),
			   buf, 6) < 0) {
		SYS_LOG_DBG("Could not read accel axis data");
		return -EIO;
	}

	drv_data->x_sample = (buf[1] << 8) | buf[0];
	drv_data->y_sample = (buf[3] << 8) | buf[2];
	drv_data->z_sample = (buf[5] << 8) | buf[4];

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

	if (i2c_burst_read(drv_data->i2c, LIS3DH_I2C_ADDRESS,
			   (LIS3DH_REG_ADC_3_LSB | LIS3DH_AUTOINCREMENT_ADDR),
			   buf, sizeof(buf)) < 0) {
		SYS_LOG_DBG("Could not read temperature data");
		return -EIO;
	}

	s16_t raw = ((buf[1] << 8) | buf [0]);

	/*
	 * The data is represented as 10bit left-aligned 2's
	 * complement, where the conversion rate is 1 digit/°C.
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

	drv_data->i2c = device_get_binding(CONFIG_LIS3DH_I2C_MASTER_DEV_NAME);
	if (drv_data->i2c == NULL) {
		SYS_LOG_DBG("Could not get pointer to %s device",
		    CONFIG_LIS3DH_I2C_MASTER_DEV_NAME);
		return -EINVAL;
	}

	/* enable accel measurements and set power mode and data rate */
	if (i2c_reg_write_byte(drv_data->i2c, LIS3DH_I2C_ADDRESS,
			       LIS3DH_REG_CTRL1, LIS3DH_ACCEL_EN_BITS |
			       LIS3DH_LP_EN_BIT | LIS3DH_ODR_BITS) < 0) {
		SYS_LOG_DBG("Failed to configure chip.");
	}

	/* set full scale range */
	if (i2c_reg_write_byte(drv_data->i2c, LIS3DH_I2C_ADDRESS,
			       LIS3DH_REG_CTRL4, LIS3DH_FS_BITS) < 0) {
		SYS_LOG_DBG("Failed to set full scale range.");
		return -EIO;
	}

#if defined(CONFIG_LIS3DH_ENABLE_TEMP)
	/* enable block data update (BDU) to use the auxiliary ADC */
	if (i2c_reg_write_byte(drv_data->i2c, LIS3DH_I2C_ADDRESS,
			       LIS3DH_REG_CTRL4,
			       LIS3DH_BDU_MASK | LIS3DH_FS_BITS) < 0) {
		SYS_LOG_DBG("Failed to set BDU-bit.");
		return -EIO;
	}

	/* connect temperature sensor to channel 3 of the ADC and
	 * enable temperature measurement */
	if (i2c_reg_write_byte(drv_data->i2c, LIS3DH_I2C_ADDRESS,
			       LIS3DH_TEMP_CFG_REG,
			       LIS3DH_TEMP_EN_BIT | LIS3DH_ADC_EN_BIT) < 0) {
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

	return 0;
}

struct lis3dh_data lis3dh_driver;

DEVICE_AND_API_INIT(lis3dh, CONFIG_LIS3DH_NAME, lis3dh_init, &lis3dh_driver,
		    NULL, POST_KERNEL, CONFIG_SENSOR_INIT_PRIORITY,
		    &lis3dh_driver_api);
