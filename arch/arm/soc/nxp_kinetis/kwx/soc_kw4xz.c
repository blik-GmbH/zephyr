/*
 * Copyright (c) 2017, NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <kernel.h>
#include <device.h>
#include <init.h>
#include <soc.h>
#include <uart.h>
#include <linker/sections.h>
#include <fsl_common.h>
#include <fsl_clock.h>
#include <arch/cpu.h>

#define ER32KSEL_OSC32KCLK	(0)
#define ER32KSEL_RTC		(2)
#define ER32KSEL_LPO1KHZ	(3)

#define LPUART0SRC_OSCERCLK	(1)

#define CLKDIV1_DIVBY2		(1)

/*
 * KW41Z Flash configuration fields
 * These 16 bytes, which must be loaded to address 0x400, include default
 * protection and security settings.
 * They are loaded at reset to various Flash Memory module (FTFE) registers.
 *
 * The structure is:
 * -Backdoor Comparison Key for unsecuring the MCU - 8 bytes
 * -Program flash protection bytes, 4 bytes, written to FPROT0-3
 * -Flash security byte, 1 byte, written to FSEC
 * -Flash nonvolatile option byte, 1 byte, written to FOPT
 * -Reserved, 1 byte, (Data flash protection byte for FlexNVM)
 * -Reserved, 1 byte, (EEPROM protection byte for FlexNVM)
 *
 */
u8_t __kinetis_flash_config_section __kinetis_flash_config[] = {
	/* Backdoor Comparison Key (unused) */
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	/* Program flash protection; 1 bit/region - 0=protected, 1=unprotected
	 */
	0xFF, 0xFF, 0xFF, 0xFF,
	/*
	 * Flash security: Backdoor key disabled, Mass erase enabled,
	 *                 Factory access enabled, MCU is secure
	 */
	0xFF,
	/* Flash nonvolatile option: NMI enabled, EzPort enabled, Normal boot */
	0xFF,
	/* Reserved for FlexNVM feature (unsupported by this MCU) */
	0xFF, 0xFF};

static const osc_config_t oscConfig = {
	.freq = CONFIG_OSC_XTAL0_FREQ,

#if defined(CONFIG_OSC_EXTERNAL)
	.workMode = kOSC_ModeExt,
#elif defined(CONFIG_OSC_LOW_POWER)
	.workMode = kOSC_ModeOscLowPower,
#elif defined(CONFIG_OSC_HIGH_GAIN)
	.workMode = kOSC_ModeOscHighGain,
#else
#error "An oscillator mode must be defined"
#endif
};

static const sim_clock_config_t simConfig = {
	.er32kSrc = ER32KSEL_OSC32KCLK,
	.clkdiv1 = SIM_CLKDIV1_OUTDIV4(CLKDIV1_DIVBY2),
};

/* This function comes from the MCUX SDK:
 * ext/hal/nxp/mcux/devices/MKW41Z4/clock_config.c
 */
static void CLOCK_SYS_FllStableDelay(void)
{
	u32_t i = 30000U;
	while (i--) {
		__NOP();
	}
}

static ALWAYS_INLINE void clkInit(void)
{
	CLOCK_SetSimSafeDivs();

	CLOCK_InitOsc0(&oscConfig);
	CLOCK_SetXtal0Freq(CONFIG_OSC_XTAL0_FREQ);

	CLOCK_BootToFeeMode(kMCG_OscselOsc, CONFIG_MCG_FRDIV, kMCG_Dmx32Default,
			    kMCG_DrsMid, CLOCK_SYS_FllStableDelay);

	CLOCK_SetInternalRefClkConfig(kMCG_IrclkEnable, kMCG_IrcSlow,
				      CONFIG_MCG_FCRDIV);

	CLOCK_SetSimConfig(&simConfig);

#if CONFIG_UART_MCUX_LPUART_0
	CLOCK_SetLpuartClock(LPUART0SRC_OSCERCLK);
#endif
}

static int kwx_init(struct device *arg)
{
	ARG_UNUSED(arg);

	int oldLevel; /* old interrupt lock level */

	/* disable interrupts */
	oldLevel = irq_lock();

	/* Configure or disable the watchdog */
#if defined(CONFIG_WATCHDOG)
#if defined(CONFIG_WDT_MKW4XZ_32MS)
	/* 1kHz LPC clock, 2^5 cycles = 32ms */
    SIM->COPC = SIM_COPC_COPCLKSEL(00) | SIM_COPC_COPCLKS(0) | SIM_COPC_COPT(01);
#elif defined(CONFIG_WDT_MKW4XZ_256MS)
    /* 1kHz LPC clock, 2^8 cycles = 256ms */
    SIM->COPC = SIM_COPC_COPCLKSEL(00) | SIM_COPC_COPCLKS(0) | SIM_COPC_COPT(10);
#elif defined(CONFIG_WDT_MKW4XZ_1024MS)
    /* 1kHz LPC clock, 2^10 cycles = 1024ms */
    SIM->COPC = SIM_COPC_COPCLKSEL(00) | SIM_COPC_COPCLKS(0) | SIM_COPC_COPT(11);
#elif defined(CONFIG_WDT_MKW4XZ_8192MS)
	/* 1kHz LPC clock, 2^13 cycles = 8192ms */
	SIM->COPC = SIM_COPC_COPCLKSEL(00) | SIM_COPC_COPCLKS(1) | SIM_COPC_COPT(01);
#elif defined(CONFIG_WDT_MKW4XZ_65536MS)
    /* 1kHz LPC clock, 2^16 cycles = 65536ms */
    SIM->COPC = SIM_COPC_COPCLKSEL(00) | SIM_COPC_COPCLKS(1) | SIM_COPC_COPT(10);
#elif defined(CONFIG_WDT_MKW4XZ_262144MS)
    /* 1kHz LPC clock, 2^18 cycles = 262144ms */
    SIM->COPC = SIM_COPC_COPCLKSEL(00) | SIM_COPC_COPCLKS(1) | SIM_COPC_COPT(11);
#else
#error "Unknown Watchdog Timer timeout period"
#endif
#else
	SIM->COPC = 0;
#endif

	/* Initialize system clock to 40 MHz */
	clkInit();

	/*
	 * install default handler that simply resets the CPU
	 * if configured in the kernel, NOP otherwise
	 */
	NMI_INIT();

	/* restore interrupt state */
	irq_unlock(oldLevel);
	return 0;
}

SYS_INIT(kwx_init, PRE_KERNEL_1, 0);
