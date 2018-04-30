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
 * @author Franco Saworski <franco.saworski@blik.io>
 *
 * This is a sample application that demonstrates how to use the internal
 * voltage divider of the NXP Kinetis MKW41Z DCDC converter to monitor supply
 * voltage at the DCDC input.
 * Supply voltages in millivolts together with some more debug information
 * are printed to the serial output.
 * This application sample was targeted to run on the FRDM-KW41Z development
 * board.
 * There are two options of voltage reference, the MCU-internal 1.195V VREF,
 * and an external 1.8V that is routed from the DCDC output back to VDDA.
 * You can select either option by setting or clearing the USE_INTERNAL_VREF
 * and USE_EXTERNAL_1P8V preprocessor macros.
 */

/*----------------------------------------------------------------------------*/
/*                                HEADERS                                     */
/*----------------------------------------------------------------------------*/

#include <zephyr.h>

#include <misc/printk.h>

#include <device.h>
#include <adc.h>
#include <fsl_common.h>
#include <fsl_vref.h>
#include <misc/util.h>

/*----------------------------------------------------------------------------*/
/*                                MACROS                                      */
/*----------------------------------------------------------------------------*/

// Set or clear to select one of the two voltage reference options.
#define USE_INTERNAL_VREF   1
#define USE_EXTERNAL_1P8V   0

#define ADC_DEV_NAME        "ADC_0"

#define ADC_DELAY           5
/**
 * @brief Set ADC channel constant to 23
 *
 * Output of internal DCDC voltage divider is internally routed to single-ended
 * ADC channel AD23.
 */
#define ADC_CHAN            23
#define ADC_SAMPLE_BUF_SIZE 1
#define ADC_RESOLUTION      12

/*----------------------------------------------------------------------------*/
/*                               VARIABLES                                    */
/*----------------------------------------------------------------------------*/

/**
 * @brief Buffer to place ADC samples
 */
static u32_t adc_sample_buf[ADC_SAMPLE_BUF_SIZE];

/**
 * @brief Single entry for the ADC sequence table
 */
static struct adc_seq_entry sample = {
        .sampling_delay = ADC_DELAY,
        .channel_id = ADC_CHAN,
        .buffer_length = ADC_SAMPLE_BUF_SIZE * sizeof(adc_sample_buf[0]),
        .buffer = adc_sample_buf,
};

/**
 * @brief Initialization of ADC sequence table
 *
 * Zephyr's ADC API allows to not only read a single sample from the ADC but
 * to define a sequence of samples that the ADC shall take.
 * The sequence with associated settings and buffers is recorded in such a
 * sequence table that is then handed over to the ADC driver.
 */
static struct adc_seq_table table = {
        .entries = &sample,
        .num_entries = 1,
};

/**
 * @brief Helper function for printing byte buffers in hexadecimal format
 */
void _print_hex(void *buf, u32_t len) {
    printk("[");
    void *top = buf + len;
    for (; buf < top; buf++) {
        printk("%X", *((char*) (buf)));
    }
    printk("]\n");
}

/**
 * @brief Function for Main thread
 */
void main(void) {
    // Get device handle for device "ADC_0" and enable the device.
    struct device *dev_adc = device_get_binding(ADC_DEV_NAME);
    if (!dev_adc) {
        printk("Cannot find %s!\n", ADC_DEV_NAME);
        return;
    }
    printk("adc_enable ...\n");
    adc_enable(dev_adc);

    // Print what this sample is about
    printk("NXP Kinetis MCU battery monitoring sample using the internal "\
            "DCDC converter's voltage divider\n\n");

    //--------------------------------------------------------------------------

    if(USE_INTERNAL_VREF)
    {
        printk("Using internal VREF\n\n");

        //----------------------------------------------------------------------

        // Initialize VREF with low-power buffer mode on
        vref_config_t vref_config;
        VREF_GetDefaultConfig(&vref_config);
        vref_config.bufferMode = kVREF_ModeLowPowerBuffer;
        VREF_Init(VREF, &vref_config);

        // Wait for reference voltage to be stable
        k_sleep(100);

        // Read SC register again. Voltage stable bit should be set.
        uint8_t sc = VREF->SC;
        printk("VREF R: 0x%02x\n", sc);

        //----------------------------------------------------------------------

        // Enable clock of the DCDC module.
        // This is the clock of the digital control module. The DCDC converter
        // can operate without it just depending on external pin states.
        static const clock_ip_name_t s_dcdcClocks[] = DCDC_CLOCKS;
        CLOCK_EnableClock(s_dcdcClocks[0]);

        // Read DCDC Register 0 and print it
        uint32_t dcdc_r0 = DCDC->REG0;
        printk("DCDC R0: 0x%02x\n", dcdc_r0);

        // Set internal voltage divider to VBAT/4
        dcdc_r0 = (dcdc_r0 & ~DCDC_REG0_DCDC_VBAT_DIV_CTRL_MASK)
                | DCDC_REG0_DCDC_VBAT_DIV_CTRL(0x03U);
        DCDC->REG0 = dcdc_r0;

        // Read back register and print again
        dcdc_r0 = DCDC->REG0;
        printk("DCDC R0: 0x%02x\n", dcdc_r0);
    }
    else if(USE_EXTERNAL_1P8V)
    {
        printk("Using external 1.8V as reference voltage\n\n");

        //----------------------------------------------------------------------

        // Enable clock of the DCDC module.
        // This is the clock of the digital control module. The DCDC converter
        // can operate without it just depending on external pin states.
        static const clock_ip_name_t s_dcdcClocks[] = DCDC_CLOCKS;
        CLOCK_EnableClock(s_dcdcClocks[0]);

        // Read DCDC Register 0 and print it
        uint32_t dcdc_r0 = DCDC->REG0;
        printk("DCDC R0: 0x%02x\n", dcdc_r0);

        // Set internal voltage divider to VBAT/2
        dcdc_r0 = (dcdc_r0 & ~DCDC_REG0_DCDC_VBAT_DIV_CTRL_MASK)
                | DCDC_REG0_DCDC_VBAT_DIV_CTRL(0x02U);
        DCDC->REG0 = dcdc_r0;

        // Read back register and print again
        dcdc_r0 = DCDC->REG0;
        printk("DCDC R0: 0x%02x\n", dcdc_r0);

        //----------------------------------------------------------------------

        // Read ADC0 SC2 register and print it
        uint32_t adc0_sc2 = ADC0->SC2;
        printk("ADC0 SC2: 0x%08x\n", adc0_sc2);

        // Select VALT as ADC voltage reference, which is 1.8V DCDC output on
        // FRDM-KW41Z
        adc0_sc2 = (adc0_sc2 & ~ADC_SC2_REFSEL_MASK) | ADC_SC2_REFSEL(0x01U);
        ADC0->SC2 = adc0_sc2;

        // Read back ADC0 SC2 register and print again
        adc0_sc2 = ADC0->SC2;
        printk("ADC0 SC2: 0x%08x\n", adc0_sc2);
    }

    //--------------------------------------------------------------------------

    // Read battery voltage forever
    uint32_t ret = 0;
    while(1) {
        printk("adc_read ... ");
        ret = adc_read(dev_adc, &table);
        if (ret != 0) {
            printk("error\n");
            break;
        }
        printk("success\n");
        _print_hex(adc_sample_buf, ADC_SAMPLE_BUF_SIZE * sizeof(adc_sample_buf[0]));
        uint32_t millivolts;
        if(USE_INTERNAL_VREF)
        {
            // Internal reference voltage is 1.195V if not set differently
            // Voltage divider was set to VBAT/4
            millivolts = (uint32_t)((((uint32_t)(adc_sample_buf[0]) * 1000.0) / (1 << ADC_RESOLUTION)) * 1.195 * 4);
        }
        else if(USE_EXTERNAL_1P8V)
        {
            // External reference voltage routed from VDD_1P8OUT to VDDA is
            // 1.8V on FRDM-KW41Z.
            // Voltage divider was set to VBAT/2
            millivolts = (uint32_t)((((uint32_t)(adc_sample_buf[0]) * 1000.0) / (1 << ADC_RESOLUTION)) * 1.8 * 2);
        }
        printk("Voltage: %d mV\n", millivolts);
        k_sleep(1000);
    }

    //--------------------------------------------------------------------------

    adc_disable(dev_adc);
}
