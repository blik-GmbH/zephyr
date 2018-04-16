/*
 * Copyright (c) 2017, NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __INC_BOARD_H
#define __INC_BOARD_H

#include <soc.h>

/* Push button switch 3 */
#define SW3_GPIO_NAME	CONFIG_GPIO_MCUX_PORTC_NAME
#define SW3_GPIO_PIN	4

/* Push button switch 4 */
#define SW4_GPIO_NAME	CONFIG_GPIO_MCUX_PORTC_NAME
#define SW4_GPIO_PIN	5

/* Push button switch 0. There is no physical switch on the board with this
 * name, so create an alias to SW3 to make the basic button sample work.
 */
#define SW0_GPIO_NAME	SW3_GPIO_NAME
#define SW0_GPIO_PIN	SW3_GPIO_PIN

#endif /* __INC_BOARD_H */
