/*
 * Copyright (c) 2018 blik GmbH
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file main.c
 *
 * @copyright 2018 blik GmbH
 *
 * @author Alexander Preißner <alexander.preissner@blik.io>
 *
 * This is a sample application that demonstrates how to use the KW41Z
 * Watchdog Timer.
 */

/*----------------------------------------------------------------------------*/
/*                                HEADERS                                     */
/*----------------------------------------------------------------------------*/

#include <zephyr.h>

#include <misc/printk.h>

#include <fsl_common.h>
#include <misc/util.h>


/*----------------------------------------------------------------------------*/
/*                                MACROS                                      */
/*----------------------------------------------------------------------------*/

/**
 * @brief Watchdog Timer reload sequence
 *
 * The exact sequence 0x55 and 0xAA MUST be written to the SRVCOP register
 * in order to reload the Watchdog Timer.
 */
#if defined(CONFIG_WATCHDOG)
#define WDT_RELOAD()    do { \
    SIM->SRVCOP = SIM_SRVCOP_SRVCOP(0x55); \
    SIM->SRVCOP = SIM_SRVCOP_SRVCOP(0xAA); \
} while(0)
#else
#define WDT_RELOAD()
#endif

/**
 * @brief Switch defining whether watchdog timer is reloaded or not in this
 * sample.
 *
 * If this variable is '0' "Hello blik!" should be printed to STDOUT every
 * 8.192 seconds.
 */
#define RELOAD_WATCHDOG     0


/**
 * @brief Function for Main thread
 */
void main(void) {
    // Print what this sample is about
    printk("Hello blik!\n");

    while(RELOAD_WATCHDOG)
    {
        WDT_RELOAD();

        k_sleep(1000);
    }

    return;
}
