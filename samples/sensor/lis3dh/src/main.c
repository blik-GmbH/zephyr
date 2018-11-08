/*
 * Copyright (c) 2016 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <sensor.h>
#include <gpio.h>
#include <device.h>
#include <stdio.h>
#include <adc.h>
#include <logging/sys_log.h>


#if defined(CONFIG_LIS3DH_TRIGGER) || defined(CONFIG_LIS3DH_TRIGGER_MANUAL)
static void trigger_handler(struct device *dev, struct sensor_trigger *trigger)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(trigger);
	SYS_LOG_INF("You moved it!");
 }
#endif
void main(void)
{
	struct sensor_value accel[3];
#if defined(CONFIG_LIS3DH_ENABLE_TEMP)
	struct sensor_value temp;
#endif
	struct device *dev;
	int rc;

	dev = device_get_binding(CONFIG_LIS3DH_NAME);
	if (dev == NULL) {
		SYS_LOG_INF("Could not get lis3dh device");
	}
#if defined(CONFIG_LIS3DH_TRIGGER) || defined(CONFIG_LIS3DH_TRIGGER_MANUAL)
	/* Take a minimal threshold to trigger easily */
	struct sensor_value threshold = {
		.val1 = 1,
		.val2 = 0,
	};
	/* Trigger as soon as one measurement is above threshold */
	struct sensor_value duration = {
		.val1 = 2,
		.val2 = 0,
	};
	struct sensor_trigger trig = {
		.type = SENSOR_TRIG_DELTA,
		.chan = SENSOR_CHAN_ACCEL_XYZ,
	};


	if (sensor_attr_set(dev, SENSOR_CHAN_ACCEL_XYZ,
	        SENSOR_ATTR_SLOPE_TH, &threshold) < 0) {
		SYS_LOG_DBG("Could not set threshold");
	}

	if (sensor_attr_set(dev, SENSOR_CHAN_ACCEL_XYZ,
	        SENSOR_ATTR_SLOPE_DUR, &duration) < 0) {
		SYS_LOG_DBG("Could not set duration");
	}


	if (sensor_trigger_set(dev, &trig, trigger_handler) < 0) {
		SYS_LOG_DBG("Could not set trigger");
	}
#endif
	/* The first measurement isn't ready quite yet */
	k_sleep(100);

	while (1)
	{
		rc = sensor_sample_fetch_chan(dev, SENSOR_CHAN_ACCEL_XYZ);
		if (rc != 0) {
			SYS_LOG_DBG("sensor_sample_fetch error: %d", rc);
			break;

		}

#if defined(CONFIG_LIS3DH_ENABLE_TEMP)
		rc = sensor_channel_get(dev, SENSOR_CHAN_TEMP, &temp);
		if (rc != 0) {
			SYS_LOG_DBG("sensor_channel_get temp error: %d", rc);
			break;
				}
#endif

#if !defined(CONFIG_LIS3DH_FIFO_ENABLE)
		rc = sensor_channel_get(dev, SENSOR_CHAN_ACCEL_XYZ, accel);
		if (rc != 0) {
			SYS_LOG_DBG("sensor_channel_get accel error: %d", rc);
			break;
		}
		SYS_LOG_INF("%d.%d %d.%d %d.%d",
			accel[0].val1, accel[0].val2,
			accel[1].val1, accel[1].val2,
			accel[2].val1, accel[2].val2);
#endif

#if defined(CONFIG_LIS3DH_FIFO_ENABLE)
		for (int i = 0; i < 32; i++) {

			sensor_channel_get(dev, SENSOR_CHAN_ACCEL_XYZ, accel);
			SYS_LOG_INF("%d.%d, %d.%d, %d.%d",
			accel[0].val1, accel[0].val2,
			accel[1].val1, accel[1].val2,
			accel[2].val1, accel[2].val2);
		}
#endif
		k_busy_wait(70000);

	}
}
