/*
 * @file main.c
 *
 * @brief Sample application demonstrates file creation, file writing, file
 * reading, and directory creation in NFFS (Newtron Flash File System)
 *
 * @copyright Apache License 2.0
 * @copyright (c) 2018 blik GmbH
 *
 * @author Alexander Preißner <alexander.preissner@blik.io>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <flash.h>
#include <fs.h>
#include <json.h>
#include <device.h>
#include <stdio.h>

/**
 * @brief Erases entire device and returns.
 *
 * Use this if you want the Flash device to be re-formatted upon next boot.
 */
#define ERASE_DEVICE            0
/**
 * @brief Switch determines whether files are also written or only read.
 *
 * Set to '0' if you want to check whether files have been persistently stored
 * on Flash device.
 */
#define WRITE_TO_FILE           0


struct device_configuration
{
    int rf_tx_channel;
    int rf_rx_channel;
    int rf_pan_id;
};

static const struct json_obj_descr device_configuration_descr[] =
{
    JSON_OBJ_DESCR_PRIM(struct device_configuration, rf_tx_channel, JSON_TOK_NUMBER),
    JSON_OBJ_DESCR_PRIM(struct device_configuration, rf_rx_channel, JSON_TOK_NUMBER),
    JSON_OBJ_DESCR_PRIM(struct device_configuration, rf_pan_id, JSON_TOK_NUMBER)
};

static char buf[256];

void main(void)
{
    const char test_phrase[]    = ",\"other_field\":123456}";
    const size_t test_phrase_len = 22;
    //static char buf[64];

    printf("=============================================================\n");
    printf("JSON on NFFS test\n");
    printf("=============================================================\n\n");

	fs_file_t file_p;
	int ret;

	//--------------------------------------------------------------------------


#if ERASE_DEVICE
	// Erase device if configured
    struct device *flash_dev = device_get_binding(CONFIG_SPI_FLASH_W25QXXXX_DRV_NAME);
    flash_write_protection_set(flash_dev, false);
    flash_erase(flash_dev, 0, CONFIG_SPI_FLASH_W25QXXXX_FLASH_SIZE);
    return;
#endif

    printf("Initialize a device configuration\n");
    static struct device_configuration dc = {
            .rf_tx_channel = 15,
            .rf_rx_channel = 16,
            .rf_pan_id = 0x8114
    };

    printf("Calculate length of encoded JSON string:");
    ssize_t enc_len = json_calc_encoded_len(device_configuration_descr, 3, &dc);
    printf(" %u\n", enc_len);

    printf("Encode data into JSON string:\n");
    ret = json_obj_encode_buf(device_configuration_descr, 3, &dc, buf, sizeof(buf));
    if(ret)
    {
        printf("ERROR: JSON encoding failed\n");
        k_sleep(100);
        return;
    }
    else
    {
        for(int i = 0; i < enc_len; i++){
            printf("%c", buf[i]);
        }
        printf("\n");
    }

    struct device_configuration test_dc;
    printf("Parsing JSON string\n");
    ret = json_obj_parse(buf, enc_len, device_configuration_descr, 3, &test_dc);
    if(ret != (1 << 3) - 1)
    {
        printf("ERROR: JSON parsing failed: %d\n", ret);
        k_sleep(100);
        return;
    }
    else
    {
        printf("RF TX Channel: %d\n", test_dc.rf_tx_channel);
        printf("RF RX Channel: %d\n", test_dc.rf_rx_channel);
        printf("RF PAN ID: 0x%04x\n", test_dc.rf_pan_id);
    }

    printf("Add additional unspecified field:\n");
    memcpy(&buf[enc_len - 1], &test_phrase, test_phrase_len);
    enc_len += test_phrase_len;
    for(int i = 0; i < enc_len; i++){
        printf("%c", buf[i]);
    }
    printf("\n");

#if WRITE_TO_FILE
    // Create file in root directory
	// Don't forget the forward slash '/' that denotes the root directory.
    // Else, file creation will fail.
	ret = fs_open(&file_p, "/file.txt");
	if(ret)
	{
	    printf("Could not open file\n");
	    k_sleep(100);
	    return;
	}
	else
	{
	    printf("File open\n");
	}

	ret = fs_write(&file_p, &buf, enc_len);
	if(ret != enc_len)
	{
	    printf("Something went wrong during writing: %d\n", ret);
	    k_sleep(100);
	    return;
	}
	else
	{
	    printf("Wrote string to file\n");
	}
	k_sleep(100);

	ret = fs_close(&file_p);
	if(ret)
	{
	    printf("Could not close file\n");
	}
	else
	{
	    printf("File closed\n");
	}
	k_sleep(100);
#endif

	// Read the contents of file in root directory.
	struct fs_dirent config_file_info;
	ret = fs_stat("/file.txt", &config_file_info);
	if(ret)
	{
	    printf("ERROR: Could not get JSON file info\n");
	    return;
	}
	else
	{
	    printf("JSON file is %u bytes long\n", config_file_info.size);
	}


	ret = fs_open(&file_p, "/file.txt");
    if(ret)
    {
        printf("Could not open file\n");
        k_sleep(100);
        return;
    }
    else
    {
        printf("File open\n");
    }

    ret = fs_read(&file_p, &buf, config_file_info.size);
    if(ret != config_file_info.size)
    {
        printf("Something went wrong during read\n");
    }
    else
    {
        printf("Read from file:\n");
        for(int i = 0; i < config_file_info.size; i++){
            printf("%c", buf[i]);
        }
        printf("\n");
    }

    ret = fs_close(&file_p);
    if(ret)
    {
        printf("Could not close file\n");
    }
    else
    {
        printf("File closed\n");
    }

    struct device_configuration test_dc2;
    printf("Parsing JSON string\n");
    ret = json_obj_parse(buf, config_file_info.size, device_configuration_descr, 3, &test_dc2);
    if(ret != (1 << 3) - 1)
    {
        printf("ERROR: JSON parsing failed: %d\n", ret);
        k_sleep(100);
        return;
    }
    else
    {
        printf("RF TX Channel: %d\n", test_dc2.rf_tx_channel);
        printf("RF RX Channel: %d\n", test_dc2.rf_rx_channel);
        printf("RF PAN ID: 0x%04x\n", test_dc2.rf_pan_id);
    }

	printf("End of application\n");
}
