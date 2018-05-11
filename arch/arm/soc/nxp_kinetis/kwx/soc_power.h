/*
 * Copyright (c) 2018 blik GmbH
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _SOC_POWER_H_
#define _SOC_POWER_H_

#ifdef __cplusplus
extern "C" {
#endif

enum power_states {
	SYS_POWER_STATE_CPU_LPS,       /* LLS3 */
	SYS_POWER_STATE_CPU_LPS_1,     /* Not Supported */
	SYS_POWER_STATE_CPU_LPS_2,     /* Not supported */
	SYS_POWER_STATE_DEEP_SLEEP,    /* Not Supported */
	SYS_POWER_STATE_DEEP_SLEEP_1,  /* Not Supported */
	SYS_POWER_STATE_DEEP_SLEEP_2,  /* Not Supported */
	SYS_POWER_STATE_MAX		/* Do nothing */
};

/**
 * @brief Put processor into low power state
 */
int _sys_soc_set_power_state(enum power_states state);

/**
 * @brief Do any SoC or architecture specific post ops after low power states.
 */
void _sys_soc_power_state_post_ops(enum power_states state);

#ifdef __cplusplus
}
#endif

#endif /* _SOC_POWER_H_ */
