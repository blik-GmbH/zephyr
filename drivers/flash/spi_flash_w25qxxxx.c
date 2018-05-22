/*
 * @file spi_flash_w25qxxxx.c
 *
 * @brief Driver implementation for Winbond W25Q series Flash devices.
 *
 * @copyright Apache License 2.0
 * @copyright (c) 2016 Intel Corporation.
 * @copyright (c) 2018 blik GmbH
 *
 * @author Alexander Preißner <alexander.preissner@blik.io>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>

#include <flash.h>
#include <spi.h>
#include <init.h>
#include <string.h>
#include "spi_flash_w25qxxxx_defs.h"
#include "spi_flash_w25qxxxx.h"
#include "flash_priv.h"


/**
 * @brief Flash Pages Layout Table Entry
 *
 * In the W25QXXXX Flash devices the minimum erasable unit is one sector, which
 * is 16 W25QXXXX pages (term according to datasheet).
 * A W25QXXXX page is 256 bytes in size.
 * The number of Zephyr pages can be calculated by dividing the size of the
 * entire Flash (in bytes) by the erasable sector size (in bytes).
 */
static const struct flash_pages_layout flash_w25qxxxx_pages_layout = {
    .pages_count = CONFIG_SPI_FLASH_W25QXXXX_FLASH_SIZE / W25QXXXX_SECTOR_SIZE,
    .pages_size = W25QXXXX_SECTOR_SIZE,
};


static inline int spi_flash_wb_id(struct device *dev)
{
	struct spi_flash_data *const driver_data = dev->driver_data;
	u8_t buf[W25QXXXX_LEN_CMD_AND_ID];
	u32_t temp_data;

	buf[0] = W25QXXXX_CMD_RDID;

	if (spi_transceive(driver_data->spi, buf, W25QXXXX_LEN_CMD_AND_ID,
			   buf, W25QXXXX_LEN_CMD_AND_ID) != 0) {
		return -EIO;
	}

	temp_data = ((u32_t) buf[1]) << 16;
	temp_data |= ((u32_t) buf[2]) << 8;
	temp_data |= (u32_t) buf[3];

	if (temp_data != W25QXXXX_RDID_VALUE) {
		return -ENODEV;
	}

	return 0;
}

static int spi_flash_wb_config(struct device *dev)
{
	struct spi_flash_data *const driver_data = dev->driver_data;
	struct spi_config config;

	config.max_sys_freq = CONFIG_SPI_FLASH_W25QXXXX_SPI_FREQ_0;

	config.config = SPI_WORD(8);

	if (spi_slave_select(driver_data->spi,
			     CONFIG_SPI_FLASH_W25QXXXX_SPI_SLAVE) !=
			     0) {
		return -EIO;
	}

	if (spi_configure(driver_data->spi, &config) != 0) {
		return -EIO;
	}

	return spi_flash_wb_id(dev);
}

static int spi_flash_wb_reg_read(struct device *dev, u8_t *data)
{
	struct spi_flash_data *const driver_data = dev->driver_data;
	u8_t buf[2];

	if (spi_transceive(driver_data->spi, data, 2, buf, 2) != 0) {
		return -EIO;
	}

	memcpy(data, buf, 2);

	return 0;
}

static inline void wait_for_flash_idle(struct device *dev)
{
	u8_t buf[2];

	buf[0] = W25QXXXX_CMD_RDSR;
	spi_flash_wb_reg_read(dev, buf);

	while (buf[1] & W25QXXXX_WIP_BIT) {
		buf[0] = W25QXXXX_CMD_RDSR;
		spi_flash_wb_reg_read(dev, buf);
	}
}

static int spi_flash_wb_reg_write(struct device *dev, u8_t *data)
{
	struct spi_flash_data *const driver_data = dev->driver_data;
	u8_t buf;

	wait_for_flash_idle(dev);

	if (spi_transceive(driver_data->spi, data, 1,
			   &buf /*dummy */, 1) != 0) {
		return -EIO;
	}

	return 0;
}

/**
 * @brief Read from W25Q device
 *
 * @param dev Flash device struct pointer
 * @param offset Start address relative to Flash memory base address
 * @param data Pointer to data buffer
 * @param len Requested number of bytes
 *
 * @return `0` on success, `-ERRNO` else
 */
static int spi_flash_wb_read(struct device *dev, off_t offset, void *data,
        size_t len) {
    struct spi_flash_data * const driver_data = dev->driver_data;
    u8_t *buf = driver_data->buf;

    // Exit if address range out of memory bounds
    if (offset < 0 || (offset + len) > CONFIG_SPI_FLASH_W25QXXXX_FLASH_SIZE) {
        return -EFAULT;
    }

    k_sem_take(&driver_data->sem, K_FOREVER);

    if (spi_flash_wb_config(dev) != 0) {
        k_sem_give(&driver_data->sem);
        return -EIO;
    }

    // If requested data is longer than
    // CONFIG_SPI_FLASH_W25QXXXX_MAX_DATA_LEN, split read access into
    // multiple SPI transactions until requested length is satisfied.
    size_t transaction_len = 0;
    while (len > 0) {
        if (len > CONFIG_SPI_FLASH_W25QXXXX_MAX_DATA_LEN) {
            transaction_len = CONFIG_SPI_FLASH_W25QXXXX_MAX_DATA_LEN;
        }
        else {
            transaction_len = len;
        }

        wait_for_flash_idle(dev);

        buf[0] = W25QXXXX_CMD_READ;
        buf[1] = (u8_t) (offset >> 16);
        buf[2] = (u8_t) (offset >> 8);
        buf[3] = (u8_t) offset;

        memset(buf + W25QXXXX_LEN_CMD_ADDRESS, 0, transaction_len);

        if (spi_transceive(driver_data->spi, buf,
                transaction_len + W25QXXXX_LEN_CMD_ADDRESS, buf,
                transaction_len + W25QXXXX_LEN_CMD_ADDRESS) != 0) {
            k_sem_give(&driver_data->sem);
            return -EIO;
        }

        memcpy(data, buf + W25QXXXX_LEN_CMD_ADDRESS, transaction_len);

        // Update indices and pointers
        offset = offset + transaction_len;
        len = len - transaction_len;
        data = data + transaction_len;

    }

    k_sem_give(&driver_data->sem);

    return 0;
}

/**
 * @brief Writes data to a single W25Q page
 *
 * @param dev Flash device struct pointer
 * @param offset Start address relative to Flash memory base address
 * @param data Pointer to input data buffer
 * @param len Requested number of bytes
 *
 * @return `0` on success, `-ERRNO` else
 *
 * Rejects write accesses that are not within a single 256-byte page according
 * to the device datasheet.
 */
static int spi_flash_wb_write_within_page(struct device *dev, off_t offset,
        const void *data, size_t len)
{
    struct spi_flash_data *const driver_data = dev->driver_data;
    u8_t *buf = driver_data->buf;

    // Exit if address range out of page bounds
    if (offset < 0 ||
        ((offset & (W25QXXXX_PAGE_SIZE - 1)) + len) > W25QXXXX_PAGE_SIZE) {
        return -ENOTSUP;
    }

    k_sem_take(&driver_data->sem, K_FOREVER);

    if (spi_flash_wb_config(dev) != 0) {
        k_sem_give(&driver_data->sem);
        return -EIO;
    }

    wait_for_flash_idle(dev);

    buf[0] = W25QXXXX_CMD_RDSR;
    spi_flash_wb_reg_read(dev, buf);

    if (!(buf[1] & W25QXXXX_WEL_BIT)) {
        k_sem_give(&driver_data->sem);
        return -EIO;
    }

    wait_for_flash_idle(dev);

    buf[0] = W25QXXXX_CMD_PP;
    buf[1] = (u8_t) (offset >> 16);
    buf[2] = (u8_t) (offset >> 8);
    buf[3] = (u8_t) offset;

    memcpy(buf + W25QXXXX_LEN_CMD_ADDRESS, data, len);

    /* Assume write protection has been disabled. Note that W25QXXXX
     * flash automatically turns on write protection at the completion
     * of each write or erase transaction.
     */
    if (spi_write(driver_data->spi, buf, len + W25QXXXX_LEN_CMD_ADDRESS) != 0) {
        k_sem_give(&driver_data->sem);
        return -EIO;
    }

    k_sem_give(&driver_data->sem);

    return 0;
}

static int spi_flash_wb_write_protection_set(struct device *dev, bool enable)
{
    struct spi_flash_data *const driver_data = dev->driver_data;
    u8_t buf = 0;

    k_sem_take(&driver_data->sem, K_FOREVER);

    if (spi_flash_wb_config(dev) != 0) {
        k_sem_give(&driver_data->sem);
        return -EIO;
    }

    wait_for_flash_idle(dev);

    if (enable) {
        buf = W25QXXXX_CMD_WRDI;
    } else {
        buf = W25QXXXX_CMD_WREN;
    }

    if (spi_flash_wb_reg_write(dev, &buf) != 0) {
        k_sem_give(&driver_data->sem);
        return -EIO;
    }

    k_sem_give(&driver_data->sem);

    return 0;
}

/**
 * @brief Write to Flash memory
 *
 * @param dev Flash device struct
 * @param offset Start address relative to Flash memory base address
 * @param data Pointer to input data buffer
 * @param len Requested number of bytes
 *
 * @return `0` on success, `-ERRNO` else
 *
 * Automatically handles page boundary alignment and splits large transactions
 * into multiple smaller transactions to comply to device limitations.
 */
static int spi_flash_wb_write(struct device *dev, off_t offset,
        const void *data, size_t len) {
    int rc;

    // Exit if address range out of memory bounds
    if (offset < 0 || (offset + len) > CONFIG_SPI_FLASH_W25QXXXX_FLASH_SIZE) {
        return -ENODEV;
    }

    // If requested data is longer than W25QXXXX_PAGE_SIZE or
    // CONFIG_SPI_FLASH_W25QXXXX_MAX_DATA_LEN, split write
    // access into multiple SPI transactions until requested length is
    // satisfied.
    size_t transaction_len  = 0;
    off_t relative_offset   = 0;
    while (len > 0) {

        // Align first write access to W25Q page boundaries
        if ( ( (offset & (W25QXXXX_PAGE_SIZE - 1)) + len) > W25QXXXX_PAGE_SIZE) {
            transaction_len = W25QXXXX_PAGE_SIZE
                    - (offset & (W25QXXXX_PAGE_SIZE - 1));
        }
        else if (len > W25QXXXX_PAGE_SIZE) {
            transaction_len = W25QXXXX_PAGE_SIZE;
        }
        else if (len > CONFIG_SPI_FLASH_W25QXXXX_MAX_DATA_LEN) {
            transaction_len = CONFIG_SPI_FLASH_W25QXXXX_MAX_DATA_LEN;
        }
        else {
            transaction_len = len;
        }

        // Disable write protection every time before data is written.
        rc = spi_flash_wb_write_protection_set(dev, false);
        if (rc) {
            return rc;
        }

        // Write transaction
        rc = spi_flash_wb_write_within_page(dev, offset,
                (data + relative_offset), transaction_len);
        if (rc) {
            return rc;
        }

        // Update indices and pointers
        offset = offset + transaction_len;
        len = len - transaction_len;
        relative_offset = relative_offset + transaction_len;
    }

    return 0;
}

static inline int spi_flash_wb_erase_internal(struct device *dev,
					      off_t offset, size_t size)
{
	struct spi_flash_data *const driver_data = dev->driver_data;
	u8_t buf[W25QXXXX_LEN_CMD_ADDRESS];
	u8_t erase_opcode;
	u32_t len;

	if (offset < 0) {
		return -ENOTSUP;
	}

	wait_for_flash_idle(dev);

	/* write enable */
	buf[0] = W25QXXXX_CMD_WREN;
	spi_flash_wb_reg_write(dev, buf);

	wait_for_flash_idle(dev);

	switch (size) {
	case W25QXXXX_SECTOR_SIZE:
		erase_opcode = W25QXXXX_CMD_SE;
		len = W25QXXXX_LEN_CMD_ADDRESS;
		break;
	case W25QXXXX_BLOCK32K_SIZE:
		erase_opcode = W25QXXXX_CMD_BE32K;
		len = W25QXXXX_LEN_CMD_ADDRESS;
		break;
	case W25QXXXX_BLOCK_SIZE:
		erase_opcode = W25QXXXX_CMD_BE;
		len = W25QXXXX_LEN_CMD_ADDRESS;
		break;
	case CONFIG_SPI_FLASH_W25QXXXX_FLASH_SIZE:
		erase_opcode = W25QXXXX_CMD_CE;
		len = 1;
		break;
	default:
		return -EIO;

	}

	buf[0] = erase_opcode;
	buf[1] = (u8_t) (offset >> 16);
	buf[2] = (u8_t) (offset >> 8);
	buf[3] = (u8_t) offset;

	/* Assume write protection has been disabled. Note that W25QXXXX
	 * flash automatically turns on write protection at the completion
	 * of each write or erase transaction.
	 */
	return spi_write(driver_data->spi, buf, len);
}

static int spi_flash_wb_erase(struct device *dev, off_t offset, size_t size)
{
	struct spi_flash_data *const driver_data = dev->driver_data;
	u8_t *buf = driver_data->buf;
	int ret = 0;
	u32_t new_offset = offset;
	u32_t size_remaining = size;

	if ((offset < 0) || ((offset & W25QXXXX_SECTOR_MASK) != 0) ||
	    ((size + offset) > CONFIG_SPI_FLASH_W25QXXXX_FLASH_SIZE) ||
	    ((size & W25QXXXX_SECTOR_MASK) != 0)) {
		return -ENODEV;
	}

	k_sem_take(&driver_data->sem, K_FOREVER);

	if (spi_flash_wb_config(dev) != 0) {
		k_sem_give(&driver_data->sem);
		return -EIO;
	}

	buf[0] = W25QXXXX_CMD_RDSR;
	spi_flash_wb_reg_read(dev, buf);

	if (!(buf[1] & W25QXXXX_WEL_BIT)) {
		k_sem_give(&driver_data->sem);
		return -EIO;
	}

	while ((size_remaining >= W25QXXXX_SECTOR_SIZE) && (ret == 0)) {
		if (size_remaining == CONFIG_SPI_FLASH_W25QXXXX_FLASH_SIZE) {
			ret = spi_flash_wb_erase_internal(dev, offset, size);
			break;
		}

		if (size_remaining >= W25QXXXX_BLOCK_SIZE) {
			ret = spi_flash_wb_erase_internal(dev, new_offset,
							  W25QXXXX_BLOCK_SIZE);
			new_offset += W25QXXXX_BLOCK_SIZE;
			size_remaining -= W25QXXXX_BLOCK_SIZE;
			continue;
		}

		if (size_remaining >= W25QXXXX_BLOCK32K_SIZE) {
			ret = spi_flash_wb_erase_internal(dev, new_offset,
							  W25QXXXX_BLOCK32K_SIZE);
			new_offset += W25QXXXX_BLOCK32K_SIZE;
			size_remaining -= W25QXXXX_BLOCK32K_SIZE;
			continue;
		}

		if (size_remaining >= W25QXXXX_SECTOR_SIZE) {
			ret = spi_flash_wb_erase_internal(dev, new_offset,
							  W25QXXXX_SECTOR_SIZE);
			new_offset += W25QXXXX_SECTOR_SIZE;
			size_remaining -= W25QXXXX_SECTOR_SIZE;
			continue;
		}
	}

	k_sem_give(&driver_data->sem);

	return ret;
}

/**
 * @brief Creates Flash Pages Layout Table
 *
 * The Flash Pages Layout Table describes the layout of erasable pages within
 * a Flash device.
 * The information is used by file systems to determine formatting and when to
 * erase what.
 *
 * @param dev Flash device struct
 * @param layout Pointer to an array of #flash_pages_layout descriptor structs
 * @param layout_size Pointer to a variable holding the size of the Pages
 * Layout Table
 */
void spi_flash_wb_page_layout(struct device *dev,
                       const struct flash_pages_layout **layout,
                       size_t *layout_size)
{
    *layout = &flash_w25qxxxx_pages_layout;
    *layout_size = 1;

    return;
}

static const struct flash_driver_api spi_flash_api = {
	.read = spi_flash_wb_read,
	.write = spi_flash_wb_write,
	.erase = spi_flash_wb_erase,
	.write_protection = spi_flash_wb_write_protection_set,
#if defined(CONFIG_FLASH_PAGE_LAYOUT)
	.page_layout = spi_flash_wb_page_layout,
#endif
	.write_block_size = 1,
};

static int spi_flash_init(struct device *dev)
{
	struct device *spi_dev;
	struct spi_flash_data *data = dev->driver_data;
	int ret;

	spi_dev = device_get_binding(CONFIG_SPI_FLASH_W25QXXXX_SPI_NAME);
	if (!spi_dev) {
		return -EIO;
	}

	data->spi = spi_dev;

	k_sem_init(&data->sem, 1, UINT_MAX);

	ret = spi_flash_wb_config(dev);
	if (!ret) {
		dev->driver_api = &spi_flash_api;
	}

	return ret;
}

static struct spi_flash_data spi_flash_memory_data;

DEVICE_INIT(spi_flash_memory, CONFIG_SPI_FLASH_W25QXXXX_DRV_NAME, spi_flash_init,
	    &spi_flash_memory_data, NULL, POST_KERNEL,
	    CONFIG_SPI_FLASH_W25QXXXX_INIT_PRIORITY);
