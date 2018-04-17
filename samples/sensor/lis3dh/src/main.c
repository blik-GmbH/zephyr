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

void main(void)
{
	struct sensor_value accel[3];
	struct sensor_value temp;
	struct device *dev;
	int rc;

	dev = device_get_binding(CONFIG_LIS3DH_NAME);
	if (dev == NULL) {
		printf("Could not get lis3dh device\n");
	}

	while (1)
	{
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
		rc = sensor_channel_get(dev, SENSOR_CHAN_TEMP, &temp);
		if (rc != 0) {
			printf("sensor_channel_get temp: %d\n", rc);
			break;
		}

		/* Print accel x,y,z */
		printf("AX=%10.6f AY=%10.6f AZ=%10.6f, TEMP=%f\n",
		        sensor_value_to_double(&accel[0]),
		        sensor_value_to_double(&accel[1]),
		        sensor_value_to_double(&accel[2]),
		        sensor_value_to_double(&temp));

		k_sleep(1000);
	}
}
