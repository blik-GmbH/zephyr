/*
 * Copyright (c) 2016 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <kernel.h>
#include <soc.h>
#include <zephyr.h>
#include <power.h>
#include <soc_power.h>
#include <misc/printk.h>
#include <string.h>
#include <board.h>
#include <device.h>
#include <rtc.h>
#include <sys_clock.h>
#include <drivers/system_timer.h>
#include <kernel_structs.h>

static struct device *rtc_dev;
static uint32_t ticks_slept;

extern s32_t _NanoIdleValGet(void);
#define DEMO_DESCRIPTION	\
	"Demo Description\n"	\
	"Application creates idle \n"\

/* Print Demo Discription String */
static void display_demo_description(void)
{
	printk(DEMO_DESCRIPTION);
}

static void set_rtc_alarm(u32_t val)
{
	struct rtc_config cfg;

	cfg.init_val = 1;
	cfg.alarm_enable = 1;
	cfg.alarm_val = 1 + val;
	cfg.cb_fn = NULL;

	rtc_set_config(rtc_dev, &cfg);
}

void main(void)
{
	int i;

	rtc_dev = device_get_binding(CONFIG_RTC_0_NAME);

	printk("\n\n***Power Management Demo***\n");
	display_demo_description();

	while (1) {
		printk("\nApplication Thread\n");
		printk("doing work:");
		for (i = 0; i < 20; i++) {
			k_busy_wait(100000);
			printk(".");
		}
		printk("\n");

		/* this will idle long enough to cause suspend */
		k_sleep(5000);
	}
}

int _sys_soc_suspend(s32_t ticks)
{
	int ret = SYS_PM_NOT_HANDLED;

	printk("suspending ticks %d\n", ticks);
	// One tick equals 10ms time.
	// To convert ticks to ms, divide by one hundred.
	// Ticks can have sub-second resolution. The RTC has second resolution.
	// To always set the RTC to the next full second, we add 99 ticks.
	set_rtc_alarm((ticks + 99) / 100);
	ret = _sys_soc_set_power_state(SYS_POWER_STATE_CPU_LPS);

	if (ret != SYS_PM_NOT_HANDLED) {
		ticks_slept = ticks;
		_sys_soc_power_state_post_ops(SYS_POWER_STATE_CPU_LPS);

		return SYS_POWER_STATE_CPU_LPS;
	}

	printk("failed to enter sleep\n");
	return ret;
}

void _sys_soc_resume(void)
{
	/* only account for ticks if a positive value */
	if (ticks_slept != -1) {
		_sys_idle_elapsed_ticks = ticks_slept;
		_sys_clock_tick_announce();
	}
}
