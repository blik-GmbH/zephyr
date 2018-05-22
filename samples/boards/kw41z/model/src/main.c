/*
 * Copyright (c) 2018 blik GmbH
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
#include <drivers/system_timer.h>
#include <kernel_structs.h>

static struct device *rtc_dev;

static struct k_thread work1_thread;
static K_THREAD_STACK_DEFINE(work1_stack, 1024);

static struct k_thread work2_thread;
static K_THREAD_STACK_DEFINE(work2_stack, 1024);

static struct k_thread rtc_thread;
static K_THREAD_STACK_DEFINE(rtc_stack, 1024);

K_SEM_DEFINE(rtc_sem, 0, 1);
static uint32_t ticks_slept;

void resume_threads(struct device *dev)
{
	ARG_UNUSED(dev);

	printk("resuming threads\n");
	k_thread_resume(&work1_thread);
	k_thread_resume(&work2_thread);
}

static void setup_rtc(void)
{
	struct rtc_config cfg;

	printk("configuring rtc wake\n");
	cfg.init_val = 1;
	cfg.alarm_enable = 1;
	cfg.alarm_val = 5;
	cfg.cb_fn = resume_threads;
	rtc_enable(rtc_dev);
	rtc_set_config(rtc_dev, &cfg);
}

void work1_func(void)
{
	while (1) {
		printk("entering %s\n", __func__);
		k_sleep(1000);
		printk("suspend %s\n", __func__);
		k_thread_suspend(_current);
	}
}

void work2_func(void)
{
	while (1) {
		printk("entering %s\n", __func__);
		k_sleep(2220);
		k_sem_give(&rtc_sem);
		printk("suspend %s\n", __func__);
		k_thread_suspend(_current);
	}
}

void rtc_func(void)
{
	rtc_dev = device_get_binding(CONFIG_RTC_0_NAME);
	while (1) {
		printk("entering %s\n", __func__);
		setup_rtc();
		printk("rtc idle\n");
		k_sem_take(&rtc_sem, K_FOREVER);
	}
}

void main(void)

{
	printk("%s\n", __func__);
	k_thread_create(&work1_thread, work1_stack, 1024,
		(k_thread_entry_t)work1_func, NULL, NULL, NULL, 2, NULL,
		K_FOREVER);

	k_thread_create(&work2_thread, work2_stack, 1024,
		(k_thread_entry_t)work2_func, NULL, NULL, NULL, 2, NULL,
		K_FOREVER);

	k_thread_create(&rtc_thread, rtc_stack, 1024,
		(k_thread_entry_t)rtc_func, NULL, NULL, NULL, 1, NULL,
		K_FOREVER);

	k_thread_start(&work1_thread);
	k_thread_start(&work2_thread);
	k_thread_start(&rtc_thread);
}

int _sys_soc_suspend(s32_t ticks)
{
	int ret = SYS_PM_NOT_HANDLED;

	printk("ticks %d\n", ticks);
	// One tick equals 10ms time.
	// Do not sleep, if k_sleep requests between 0 and 3 seconds.
	if (ticks < 300 && ticks > 0) {
		return ret;
	}

	printk("suspending\n");
	ret = _sys_soc_set_power_state(SYS_POWER_STATE_CPU_LPS);

	if (ret != SYS_PM_NOT_HANDLED) {
		ticks_slept = ticks;
		_sys_soc_power_state_post_ops(SYS_POWER_STATE_CPU_LPS);
		return SYS_POWER_STATE_CPU_LPS;
	}

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
