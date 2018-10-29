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

#if defined(CONFIG_LIS3DH_TRIGGER) || defined(CONFIG_LIS3DH_TRIGGER_MANUAL)

static void trigger_handler(struct device *dev, struct sensor_trigger *trigger)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(trigger);
	printf("You moved it!");
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
		printk("Could not get lis3dh device\n");
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
		printk("Could not set threshold\n");
	}

	if (sensor_attr_set(dev, SENSOR_CHAN_ACCEL_XYZ,
	        SENSOR_ATTR_SLOPE_DUR, &duration) < 0) {
		printk("Could not set duration.\n");
	}


	if (sensor_trigger_set(dev, &trig, trigger_handler) < 0) {
		printk("Could not set trigger\n");
	}
#endif
	/* The first measurement isn't ready quite yet */
	k_sleep(100);

	while (1)
	{
		rc = sensor_sample_fetch_chan(dev, SENSOR_CHAN_ACCEL_XYZ);
		if (rc != 0) {
			printk("sensor_sample_fetch error: %d\n", rc);
			break;
		}

#if defined(CONFIG_LIS3DH_ENABLE_TEMP)
		rc = sensor_channel_get(dev, SENSOR_CHAN_TEMP, &temp);
		if (rc != 0) {
			printk("sensor_channel_get temp: %d\n", rc);
			break;
				}
#endif

#if !defined(CONFIG_LIS3DH_FIFO_ENABLE)
		rc = sensor_channel_get(dev, SENSOR_CHAN_ACCEL_XYZ, accel);
		if (rc != 0) {
					printk("sensor_channel_get accel: %d\n", rc);
					break;
				}
		printk(" AX= %d.%d   AY= %d.%d   AZ= %d.%d \n",
							accel[0].val1, accel[0].val2,
							accel[1].val1, accel[1].val2,
							accel[2].val1, accel[2].val2);
#endif

#if defined(CONFIG_LIS3DH_FIFO_ENABLE)
		for(int i = 0; i < 32; i++)
		{

			sensor_channel_get(dev, SENSOR_CHAN_ACCEL_XYZ, accel);
			printk(" AX= %d.%d   AY= %d.%d   AZ= %d.%d \n",
					accel[0].val1, accel[0].val2,
					accel[1].val1, accel[1].val2,
					accel[2].val1, accel[2].val2);
		}
#endif


		k_sleep(1000);



	}
}
