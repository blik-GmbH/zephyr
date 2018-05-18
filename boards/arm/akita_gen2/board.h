/*
 * Copyright (c) 2018, blik GmbH
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __INC_BOARD_H
#define __INC_BOARD_H

#include <soc.h>

#define BUT0_GPIO_NAME	CONFIG_GPIO_MCUX_PORTC_NAME
#define BUT0_GPIO_PIN	2

#define LED0_GPIO_PORT	CONFIG_GPIO_MCUX_PORTA_NAME
#define LED0_GPIO_PIN	19

/*
 * @brief MAC_LSB / MAC_0
 *
 * @note The first 32bit group and least significant part of the 40bit MAC
 *
 * 32bit bitfield:
 * 31-00: MAC
 */
#define AKITA_MAC_0 ((uint32_t)(RSIM->MAC_LSB))

/*
 * @brief MAC_MSB / MAC_1
 *
 * @note The second 32bit group with its lower 8bit used and upper 24bit zero,
 *       and most significant part of the 40bit UID
 *
 * 32bit bitfield:
 * 31-08: zero
 * 07-00: MAC
 */
#define AKITA_MAC_1 ((uint32_t)(RSIM->MAC_MSB))

/*
 * @brief UIDL / UID_0
 *
 * @note The first 32bit group and least significant part of the 80bit UID
 *
 * 32bit bitfield:
 * 31-00: UID
 */
#define AKITA_UID_0 ((uint32_t)(SIM->UIDL))

/*
 * @brief UIDML / UID_1
 *
 * @note The second 32bit group between least and most significant part of the
 *       80bit UID
 *
 * 32bit bitfield:
 * 31-00: UID
 */
#define AKITA_UID_1 ((uint32_t)(SIM->UIDML))

/*
 * @brief UIDMH / UID_2
 *
 * @note The third 32bit group with its lower 16bit used and upper 16bit unused,
 *       and most significant part of the 80bit UID
 *
 * 32bit bitfield:
 * 31-16: reserved
 * 15-00: UID
 */
#define AKITA_UID_2 ((uint32_t)(SIM->UIDMH))

#endif /* __INC_BOARD_H */
