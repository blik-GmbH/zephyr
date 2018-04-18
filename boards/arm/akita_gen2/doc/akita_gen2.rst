.. _frdm_kw41z:

blik AKITA Gen2
###############

Overview
********

The AKITA Gen2 is a sensornode enabled by the Kinetis |reg| W series
KW41Z/31Z/21Z (KW41Z) family built on ARM |reg| Cortex |reg|-M0+ processor with
integrated 2.4 GHz transceiver supporting Bluetooth |reg| Smart/Bluetooth
|reg| Low Energy
(BLE) v4.2, Generic FSK, IEEE |reg| 802.15.4 and Thread.
It also carries a LIS3DHTR accelerometer, W25Q16 flash memory and is powered
by a CR2477 coincell. The 2.4 GHz radio features a PCB F-antenna.

Hardware
********

- DC-DC configured to Buck mode
- PCB inverted F-type antenna
- SPI Flash for OTA firmware upgrades
- On board ST LIS3DH digital sensor, 3D Accelerometer ( |plusminus| 2g/
  |plusminus| 4g/ |plusminus| 8g)
- Testpoints for USART
- SWD debug and programming port

For more information about the KW41Z SoC and FRDM-KW41Z board:

- `KW41Z Website`_
- `KW41Z Datasheet`_
- `KW41Z Reference Manual`_
- `FRDM-KW41Z Website`_
- `FRDM-KW41Z User Guide`_
- `FRDM-KW41Z Schematics`_

Supported Features
==================

The akita_gen2 board configuration supports the following hardware features:

+-----------+------------+-------------------------------------+
| Interface | Controller | Driver/Component                    |
+===========+============+=====================================+
| NVIC      | on-chip    | nested vector interrupt controller  |
+-----------+------------+-------------------------------------+
| SYSTICK   | on-chip    | systick                             |
+-----------+------------+-------------------------------------+
| PINMUX    | on-chip    | pinmux                              |
+-----------+------------+-------------------------------------+
| GPIO      | on-chip    | gpio                                |
+-----------+------------+-------------------------------------+
| I2C       | on-chip    | i2c                                 |
+-----------+------------+-------------------------------------+
| SPI       | on-chip    | spi                                 |
+-----------+------------+-------------------------------------+
| ADC       | on-chip    | adc                                 |
+-----------+------------+-------------------------------------+
| UART      | on-chip    | serial port-polling;                |
|           |            | serial port-interrupt               |
+-----------+------------+-------------------------------------+
| FLASH     | on-chip    | soc flash                           |
+-----------+------------+-------------------------------------+
| SENSOR    | off-chip   | lis3dhtr polling:                   |
|           |            | lis3dhtr trigger                    |
+-----------+------------+-------------------------------------+

The default configuration can be found in the defconfig file:

	``boards/arm/akita_gen2/akita_gen2_defconfig``

Other hardware features are not currently supported by the port.

System Clock
============

The KW41Z SoC is configured to use the 32 MHz external oscillator on the board
with the on-chip FLL to generate a 40 MHz system clock.

Serial Port
===========

The KW41Z SoC has one UART, which is used for the console.

.. _FRDM-KW41Z Website:
   http://www.nxp.com/products/microcontrollers-and-processors/more-processors/application-specific-mcus-mpus/bluetooth-low-energy-ble/nxp-freedom-development-kit-for-kinetis-kw41z-31z-21z-mcus:FRDM-KW41Z

.. _FRDM-KW41Z User Guide:
   http://www.nxp.com/assets/documents/data/en/user-guides/FRDMKW41ZUG.pdf

.. _FRDM-KW41Z Schematics:
   http://www.nxp.com/assets/downloads/data/en/schematics/FRDM-KW41Z-SCH.pdf

.. _KW41Z Website:
   http://www.nxp.com/products/microcontrollers-and-processors/arm-processors/kinetis-cortex-m-mcus/w-series-wireless-m0-plus-m4/kinetis-kw41z-2.4-ghz-dual-mode-ble-and-802.15.4-wireless-radio-microcontroller-mcu-based-on-arm-cortex-m0-plus-core:KW41Z

.. _KW41Z Datasheet:
   http://www.nxp.com/assets/documents/data/en/data-sheets/MKW41Z512.pdf

.. _KW41Z Reference Manual:
   http://www.nxp.com/assets/documents/data/en/reference-manuals/MKW41Z512RM.pdf

.. _DAPLink FRDM-KW41Z Firmware:
   http://www.nxp.com/assets/downloads/data/en/reference-applications/OpenSDAv2.2_DAPLink_frdmkw41z_rev0241.zip

.. _Segger J-Link OpenSDA V2.1 Firmware:
   https://www.segger.com/downloads/jlink/OpenSDA_V2_1.bin
