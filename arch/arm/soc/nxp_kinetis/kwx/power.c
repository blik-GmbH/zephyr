/*
 * Copyright (c) 2018 blik GmbH
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define SYS_LOG_LEVEL SYS_LOG_LEVEL_DEBUG
#include <logging/sys_log.h>
#include <arch/arm/cortex_m/cmsis.h>

#include <zephyr.h>
#include <power.h>
#include <soc_power.h>
#include <fsl_smc.h>

static int _issue_lls3(void)
{
	uint8_t reg;
	int ret = SYS_PM_LOW_POWER_STATE;

	/* shift out all the uart chars before suspending */
	while (!(BIT(22) & LPUART0->STAT)) {
	}

	/* disable flash prefetch */
	MCM->PLACR &= ~(MCM_PLACR_DFCS_MASK | MCM_PLACR_EFDS_MASK);
	MCM->PLACR |= MCM_PLACR_DFCS_MASK;

	/* set to allow LLS3x, VLLSx, and VLPS */
	SMC->PMPROT |= kSMC_AllowPowerModeLls | kSMC_AllowPowerModeVlp |
		       kSMC_AllowPowerModeVlls;

	/* set PMCTRL for LLSX */
	reg = SMC->PMCTRL;
	reg &= ~SMC_PMCTRL_STOPM_MASK;
	reg |= kSMC_StopLls;
	SMC->PMCTRL = reg;

	/* Configure stop to target LLS3 specifically */
	SMC->STOPCTRL = kSMC_StopSub3;

	/* enable wake irq for rtc - ME5 */
	LLWU->ME = LLWU_ME_WUME5_MASK;

	/*
	 * Set deep sleep and do a memory barrier followed by WFI to
	 * enter low power mode
	 */
	SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
	SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

	(void)SMC->PMCTRL;
	__DSB();
	__WFI();
	__ISB();

	if (SMC->PMCTRL & SMC_PMCTRL_STOPA_MASK) {
		ret = SYS_PM_NOT_HANDLED;
	}

	/* clear deep sleep so we don't suspend if idle does a WFI */
	SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;

	/* re-enable flash prefetch */
	MCM->PLACR &= ~(MCM_PLACR_DFCS_MASK | MCM_PLACR_EFDS_MASK);
	MCM->PLACR |= MCM_PLACR_EFDS_MASK;

	return ret;
}

int _sys_soc_set_power_state(enum power_states state)
{
	int ret = SYS_PM_NOT_HANDLED;

	switch (state) {
	case SYS_POWER_STATE_CPU_LPS:
		ret = _issue_lls3();
		break;
	default:
		/* Unsupported State */
		SYS_LOG_ERR("Unsupported State\n");
		break;
	}

	return ret;
}

void _sys_soc_power_state_post_ops(enum power_states state)
{
	switch (state) {
	case SYS_POWER_STATE_CPU_LPS:
		irq_unlock(0);
		break;
	default:
		/* Unsupported State */
		SYS_LOG_ERR("Unsupported State\n");
		break;
	}
}
