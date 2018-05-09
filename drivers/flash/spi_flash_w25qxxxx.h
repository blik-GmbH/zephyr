/*
 * Copyright (c) 2016 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 *
 * @brief This file defines the private data structures for spi flash driver
 */

#ifndef __SPI_FLASH_W25QXXXX_H__
#define __SPI_FLASH_W25QXXXX_H__


struct spi_flash_data {
	struct device *spi;
	u8_t buf[CONFIG_SPI_FLASH_W25QXXXX_MAX_DATA_LEN +
		    W25QXXXX_LEN_CMD_ADDRESS];
	struct k_sem sem;
};


#endif /* __SPI_FLASH_W25QXXXX_H__ */
