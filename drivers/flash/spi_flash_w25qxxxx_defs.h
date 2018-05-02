/*
 * Copyright (c) 2016 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 *
 * @brief This file has the WinBond SPI flash private definitions
 */

#ifndef __SPI_FLASH_W25QXXXX_DEFS_H__
#define __SPI_FLASH_W25QXXXX_DEFS_H__

/* Status Registers
 *    S7     S6     S5     S4     S3     S2     S1     S0
 *  +-------------------------------------------------------+
 *  | SRP0 | SEC  |  TB  | BP2  | BP1  | BP0  | WEL  | BUSY |
 *  +-------------------------------------------------------+
 *
 * BUSY - Erase/Write In Progress - 1 device is executing a command, 0 ready for command
 * WEL  - Write Enable Latch - 1 write enable is received, 0 completeion of
 * a Write Disable, Page Program, Erase, Write Status Register
 *
 *    S15    S14    S13    S12    S11    S10    S9     S8
 *  +-------------------------------------------------------+
 *  | SUS  | CMP  | LB3  | LB2  | LB1  | xxx  | QE   | SRP1 |
 *  +-------------------------------------------------------+
 *
 *    S23        S22    S21    S20    S19    S18    S17    S16
 *  +----------------------------------------------------------+
 *  | HOLD/RST | DRV1 | DRV0 | xxx  | xxx  | WPS  | xxx  | xxx |
 *  +----------------------------------------------------------+
 */

#if CONFIG_SPI_FLASH_W25QXXXX_DEVICE_W25QXXDV
#define W25QXXXX_RDID_VALUE  (0x00ef4015)
#elif CONFIG_SPI_FLASH_W25QXXXX_DEVICE_W25QXXFW
#define W25QXXXX_RDID_VALUE  (0x00ef6015)
#endif
#define W25QXXXX_MAX_LEN_REG_CMD      (6)
#define W25QXXXX_OPCODE_LEN           (1)
#define W25QXXXX_ADDRESS_WIDTH        (3)
#define W25QXXXX_LEN_CMD_ADDRESS      (4)
#define W25QXXXX_LEN_CMD_AND_ID       (4)

/* relevant status register bits */
#define W25QXXXX_WIP_BIT         (0x1 << 0)
#define W25QXXXX_WEL_BIT         (0x1 << 1)
#define W25QXXXX_SRWD_BIT        (0x1 << 7)
#define W25QXXXX_TB_BIT          (0x1 << 3)
#define W25QXXXX_SR_BP_OFFSET    (2)

/* relevant security register bits */
#define W25QXXXX_SECR_WPSEL_BIT  (0x1 << 7)
#define W25QXXXX_SECR_EFAIL_BIT  (0x1 << 6)
#define W25QXXXX_SECR_PFAIL_BIT  (0x1 << 5)

/* supported erase size */
#define W25QXXXX_SECTOR_SIZE     (0x1000)
#define W25QXXXX_BLOCK32K_SIZE   (0x8000)
#define W25QXXXX_BLOCK_SIZE      (0x10000)

#define W25QXXXX_SECTOR_MASK     (0xFFF)

/* ID comands */
#define W25QXXXX_CMD_RDID        0x9F
#define W25QXXXX_CMD_RES         0xAB
#define W25QXXXX_CMD_REMS        0x90
#define W25QXXXX_CMD_QPIID       0xAF
#define W25QXXXX_CMD_UNID        0x4B

/*Register comands */
#define W25QXXXX_CMD_WRSR        0x01
#define W25QXXXX_CMD_RDSR        0x05
#define W25QXXXX_CMD_RDSR2       0x35
#define W25QXXXX_CMD_WRSCUR      0x2F
#define W25QXXXX_CMD_RDSCUR      0x48

/* READ comands */
#define W25QXXXX_CMD_READ        0x03
#define W25QXXXX_CMD_2READ       0xBB
#define W25QXXXX_CMD_4READ       0xEB
#define W25QXXXX_CMD_FASTREAD    0x0B
#define W25QXXXX_CMD_DREAD       0x3B
#define W25QXXXX_CMD_QREAD       0x6B
#define W25QXXXX_CMD_RDSFDP      0x5A

/* Program comands */
#define W25QXXXX_CMD_WREN        0x06
#define W25QXXXX_CMD_WRDI        0x04
#define W25QXXXX_CMD_PP          0x02
#define W25QXXXX_CMD_4PP         0x32
#define W25QXXXX_CMD_WRENVSR     0x50

/* Erase comands */
#define W25QXXXX_CMD_SE          0x20
#define W25QXXXX_CMD_BE32K       0x52
#define W25QXXXX_CMD_BE          0xD8
#define W25QXXXX_CMD_CE          0x60

/* Mode setting comands */
#define W25QXXXX_CMD_DP          0xB9
#define W25QXXXX_CMD_RDP         0xAB

/* Reset comands */
#define W25QXXXX_CMD_RSTEN       0x66
#define W25QXXXX_CMD_RST         0x99
#define W25QXXXX_CMD_RSTQIO      0xF5

/* Security comands */
#define W25QXXXX_CMD_ERSR        0x44
#define W25QXXXX_CMD_PRSR        0x42

/* Suspend/Resume comands */
#define W25QXXXX_CMD_PGM_ERS_S   0x75
#define W25QXXXX_CMD_PGM_ERS_R   0x7A

#define W25QXXXX_CMD_NOP         0x00

#endif /*__SPI_FLASH_W25QXXXX_DEFS_H__*/
