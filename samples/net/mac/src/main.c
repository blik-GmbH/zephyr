/*
 * Copyright (c) 2016 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <stdio.h>

#include "MKW41Z4.h"

void read_and_print_mac()
{
	const uint32_t mac_msb = RSIM->MAC_MSB;
	const uint32_t mac_lsb = RSIM->MAC_LSB;

	uint64_t mac = ((mac_msb << 32) | mac_lsb);

	uint8_t *m = &mac;
	printf("MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
	       m[5], m[4], m[3], m[2], m[1], m[0]);
	printf("\n");
}

void main(void)
{
	read_and_print_mac();
}
