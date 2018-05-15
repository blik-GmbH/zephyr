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

static void trigger_handler(struct device *dev, struct sensor_trigger *trigger)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(trigger);
	printf("You moved it!");
	k_sleep(100);
 }

static void trigger_sleep(struct device *dev, struct sensor_trigger *trigger)
{
	printf("ENTERING SLEEP MODE...\n");
	if (sensor_trigger_set(dev, trigger, NULL) < 0) {
		printf("Could not set trigger\n");
	}
}

static void trigger_wake(struct device *dev, struct sensor_trigger *trigger)
{
	printf("WOKE UP. Checking interrupts...\n");
	if (sensor_trigger_set(dev, trigger, trigger_handler) < 0) {
		printf("Could not set trigger\n");
	}
}

void main(void)
{
	struct sensor_value accel[3];
	struct device *dev;
	int rc;

	struct sensor_value threshold = {
		.val1 = 2,
		.val2 = 0,
	};
	/* Trigger as soon as one measurment is above threshold */
	struct sensor_value duration = {
		.val1 = 1,
		.val2 = 0,
	};
	struct sensor_trigger trig = {
		.type = SENSOR_TRIG_DELTA,
		.chan = SENSOR_CHAN_ACCEL_XYZ,
	};

	dev = device_get_binding(CONFIG_LIS3DH_NAME);
	if (dev == NULL) {
		printf("Could not get lis3dh device\n");
	}

	if (sensor_attr_set(dev, SENSOR_CHAN_ACCEL_XYZ,
	        SENSOR_ATTR_SLOPE_TH, &threshold) < 0) {
		printf("Could not set threshold\n");
	}

	if (sensor_attr_set(dev, SENSOR_CHAN_ACCEL_XYZ,
	        SENSOR_ATTR_SLOPE_DUR, &duration) < 0) {
		printf("Could not set duration.\n");
	}

	if (sensor_trigger_set(dev, &trig, trigger_handler) < 0) {
		printf("Could not set trigger\n");
	}

	bool sleep = true;

	while (1)
	{

		if (sleep) {
			trigger_sleep(dev, &trig);
			sleep = false;
                        k_sleep(10000);
		} else {
			trigger_wake(dev, &trig);
			sleep = true;
			k_sleep(100);
		}

		rc = sensor_sample_fetch_chan(dev, SENSOR_CHAN_ALL);
		if (rc != 0) {
			printf("sensor_sample_fetch error: %d\n", rc);
			break;
		}

		rc = sensor_channel_get(dev, SENSOR_CHAN_ACCEL_XYZ, accel);
		if (rc != 0) {
			printf("sensor_channel_get accel: %d\n", rc);
			break;
		}

		printf("AX=%10.6f AY=%10.6f AZ=%10.6f\n",
		       sensor_value_to_double(&accel[0]),
		       sensor_value_to_double(&accel[1]),
		       sensor_value_to_double(&accel[2]));

		k_sleep(1000);
	}
}
