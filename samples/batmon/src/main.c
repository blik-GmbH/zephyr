/*
 * Copyright (c) 2018 blik GmbH
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>

#include <misc/printk.h>

#include <device.h>
#include <adc.h>
#include <fsl_common.h>
#include <misc/util.h>

#define ADC_DEV_NAME "ADC_0"

#define ADC_DELAY 5
#define ADC_CHAN 23
#define BUF_SIZE 10

static u32_t seq_buf[2][BUF_SIZE];

static struct adc_seq_entry sample = {
	.sampling_delay = ADC_DELAY,
	.channel_id = ADC_CHAN,
	.buffer_length = BUF_SIZE * sizeof(seq_buf[0][0]),
	.buffer = seq_buf[0],
};

static struct adc_seq_table table = {
	.entries = &sample,
	.num_entries = 1,
};

void _print_hex(void *buf, u32_t len) {
	printk("[");
	void *top = buf + len;
	for (; buf < top; buf++) {
		printk("%X", *((char*)(buf)));
	}
	printk("]\n");
}

void main(void)
{
	struct device *dev_adc = device_get_binding(ADC_DEV_NAME);
	if (!dev_adc) {
		printk("Cannot find %s!\n", ADC_DEV_NAME);
		return;
	}
	printk("adc_enable ...\n");
	adc_enable(dev_adc);

	//int val = DCDC->REG0;
	//DCDC->REG0 |= DCDC_REG0_DCDC_VBAT_DIV_CTRL(0);

	printk("Hello World! %s\n", CONFIG_ARCH);

	//----------------------------------------------------------------------------

	static const clock_ip_name_t s_vrefClocks[] = VREF_CLOCKS;
	CLOCK_EnableClock(s_vrefClocks[0]);

	uint8_t trm = VREF->TRM;
	uint8_t sc = VREF->SC;
	printk("VREF R: 0x%02x 0x%02x\n", trm, sc);

	sc = 0b10000010;
	VREF->SC = sc;

	sc = VREF->SC;
	printk("VREF R: 0x%02x 0x%02x\n", trm, sc);

	k_sleep(100);

	sc = VREF->SC;
	printk("VREF R: 0x%02x 0x%02x\n", trm, sc);

	//----------------------------------------------------------------------------

	static const clock_ip_name_t s_dcdcClocks[] = DCDC_CLOCKS;
	CLOCK_EnableClock(s_dcdcClocks[0]);

	int tmp = ADC0->SC1[0];
	printk("ADC0 SC1: 0x%02x\n", tmp);

	tmp = DCDC->REG0;
	printk("DCDC R0: 0x%02x\n", tmp);
	tmp |= DCDC_REG0_DCDC_VBAT_DIV_CTRL(0b11);
	DCDC->REG0 = tmp;

	tmp = DCDC->REG0;
	printk("DCDC R0: 0x%02x\n", tmp);

	/*tmp &= ~(DCDC_REG0_DCDC_VBAT_DIV_CTRL(0b11));
	DCDC->REG0 = tmp;

	tmp = DCDC->REG0;
	printk("DCDC R0: 0x%02x\n", tmp);*/

	//----------------------------------------------------------------------------

	int ret = 0;
	while (1) {
		printk("adc_read ... ");
		ret = adc_read(dev_adc, &table);
		if (ret != 0) {
			printk("error\n");
			break;
		}
		printk("success\n");
		_print_hex(seq_buf[0], BUF_SIZE);
		k_sleep(1000);
	}
	adc_disable(dev_adc);
}
