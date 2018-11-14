/*
 * Copyright (c) 2018, blik GmbH
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <init.h>
#include <pinmux.h>
#include <fsl_port.h>

static int akita_gen2_pinmux_init(struct device *dev)
{
	ARG_UNUSED(dev);

	struct device *porta = device_get_binding(CONFIG_PINMUX_MCUX_PORTA_NAME);
	struct device *portb = device_get_binding(CONFIG_PINMUX_MCUX_PORTB_NAME);
	struct device *portc = device_get_binding(CONFIG_PINMUX_MCUX_PORTC_NAME);

	/* LIS3DH INT1: TP 9 */
	pinmux_pin_set(porta, 16, PORT_PCR_MUX(kPORT_MuxAsGpio));
	/* LIS3DH INT2: TP 11 */
	pinmux_pin_set(porta, 17, PORT_PCR_MUX(kPORT_MuxAsGpio));
	/* TP 6 */
	pinmux_pin_set(porta, 18, PORT_PCR_MUX(kPORT_MuxAsGpio));

#if CONFIG_ADC_0
	/* ADC0 SE5: TP 5 */
	pinmux_pin_set(porta, 19, PORT_PCR_MUX(kPORT_PinDisabledOrAnalog));
#endif

	/* LED */
	pinmux_pin_set(portb, 0, PORT_PCR_MUX(kPORT_MuxAsGpio));
	/* TP 4 */
	pinmux_pin_set(portb, 1, PORT_PCR_MUX(kPORT_MuxAsGpio));
	/* TP 3 */
	pinmux_pin_set(portb, 2, PORT_PCR_MUX(kPORT_MuxAsGpio));
	/* TP 2 */
	pinmux_pin_set(portb, 3, PORT_PCR_MUX(kPORT_MuxAsGpio));
	/* TP 1 */
	pinmux_pin_set(portb, 18, PORT_PCR_MUX(kPORT_MuxAsGpio));
	/* TP 27 */
	pinmux_pin_set(portc, 1, PORT_PCR_MUX(kPORT_MuxAsGpio));

#if CONFIG_I2C_1
	/* I2C1 SCL */
	pinmux_pin_set(portc, 2, PORT_PCR_MUX(kPORT_MuxAlt3));
	/* I2C1 SDA */
	pinmux_pin_set(portc, 3, PORT_PCR_MUX(kPORT_MuxAlt3));
#endif

	/* TP 15 */
	pinmux_pin_set(portc, 4, PORT_PCR_MUX(kPORT_MuxAsGpio));
	/* TP 12 */
	pinmux_pin_set(portc, 5, PORT_PCR_MUX(kPORT_MuxAsGpio));

#ifdef CONFIG_UART_MCUX_LPUART_0
	/* UART0 RX: TP 19 */
	pinmux_pin_set(portc,  6, PORT_PCR_MUX(kPORT_MuxAlt4));
	/* UART0 TX: TP 18 */
	pinmux_pin_set(portc,  7, PORT_PCR_MUX(kPORT_MuxAlt4));
#endif

#ifdef CONFIG_SPI_0
	/* SPI0 SCK: TP 23 */
	pinmux_pin_set(portc, 16, PORT_PCR_MUX(kPORT_MuxAlt2));
	/* SPI0 SOUT: TP 24 */
	pinmux_pin_set(portc, 17, PORT_PCR_MUX(kPORT_MuxAlt2));
	/* SPI0 SIN: TP 25 */
	pinmux_pin_set(portc, 18, PORT_PCR_MUX(kPORT_MuxAlt2));
	/* SPI0 PCS0: TP 26 */
	pinmux_pin_set(portc, 19, PORT_PCR_MUX(kPORT_MuxAlt2));
#endif

	return 0;
}

SYS_INIT(akita_gen2_pinmux_init, PRE_KERNEL_1, CONFIG_PINMUX_INIT_PRIORITY);
