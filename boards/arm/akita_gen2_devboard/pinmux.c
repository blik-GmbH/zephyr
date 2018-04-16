/*
 * Copyright (c) 2017, NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <init.h>
#include <pinmux.h>
#include <fsl_port.h>

static int akita_gen2_devboard_pinmux_init(struct device *dev)
{
	ARG_UNUSED(dev);

#ifdef CONFIG_PINMUX_MCUX_PORTA
	struct device *porta =
		device_get_binding(CONFIG_PINMUX_MCUX_PORTA_NAME);
#endif
#ifdef CONFIG_PINMUX_MCUX_PORTB
	struct device *portb =
		device_get_binding(CONFIG_PINMUX_MCUX_PORTB_NAME);
#endif
#ifdef CONFIG_PINMUX_MCUX_PORTC
	struct device *portc =
		device_get_binding(CONFIG_PINMUX_MCUX_PORTC_NAME);
#endif

	/* Accelerometer: Anymotion interrupt pin */
	pinmux_pin_set(porta, 17, PORT_PCR_MUX(kPORT_MuxAsGpio));

#if CONFIG_I2C_1
	/* I2C1 SCL, SDA */
	pinmux_pin_set(portc,  2, PORT_PCR_MUX(kPORT_MuxAlt3)
					| PORT_PCR_PS_MASK);
	pinmux_pin_set(portc,  3, PORT_PCR_MUX(kPORT_MuxAlt3)
					| PORT_PCR_PS_MASK);
#endif

#ifdef CONFIG_SPI_0
	/* SPI0 MISO, MOSI, SCK, CS0, CS1 */
	pinmux_pin_set(portc, 18, PORT_PCR_MUX(kPORT_MuxAlt2));
	pinmux_pin_set(portc, 17, PORT_PCR_MUX(kPORT_MuxAlt2));
	pinmux_pin_set(portc, 16, PORT_PCR_MUX(kPORT_MuxAlt2));
	pinmux_pin_set(portc, 19, PORT_PCR_MUX(kPORT_MuxAlt2));
#endif

#if CONFIG_ADC_0
	/* ADC0_SE3 */
	pinmux_pin_set(portb,  2, PORT_PCR_MUX(kPORT_PinDisabledOrAnalog));
#endif

#ifdef CONFIG_UART_MCUX_LPUART_0
	/* UART0 RX, TX */
	pinmux_pin_set(portc,  6, PORT_PCR_MUX(kPORT_MuxAlt4));
	pinmux_pin_set(portc,  7, PORT_PCR_MUX(kPORT_MuxAlt4));
#endif

	return 0;
}

SYS_INIT(akita_gen2_devboard_pinmux_init, PRE_KERNEL_1, CONFIG_PINMUX_INIT_PRIORITY);
