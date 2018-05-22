/*
 * @file main.c
 *
 * @brief Sample application that demonstrates creation, encoding, storage,
 * read-back, and parsing of a JSON-formatted device configuration on an
 * NFFS-formatted SPI Flash device.
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
#include <string.h>

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

//------------------------------------------------------------------------------

#define MEM_SLAB_SIZE           1024
K_MEM_SLAB_DEFINE(mem_slab, MEM_SLAB_SIZE, 2, 4);

//------------------------------------------------------------------------------

/**
 * @brief C struct containing device configuration items
 */
struct device_configuration_struct {
    uint32_t rf_tx_channel;
    uint32_t rf_rx_channel;
    uint32_t rf_pan_id;
    int32_t rf_tx_power;
    uint32_t rf_tx_interval;
    uint32_t rf_rx_interval;
    uint32_t rf_nd_interval;
    int32_t rf_ca_1;
    int32_t rf_ca_2;
    int32_t rf_ca_3;
    uint32_t pktopt;
};

/**
 * @brief List of JSON object descriptors for #device_configuration_struct
 */
static const struct json_obj_descr device_configuration_descr[] = {
JSON_OBJ_DESCR_PRIM(struct device_configuration_struct,
        rf_tx_channel, JSON_TOK_NUMBER),
JSON_OBJ_DESCR_PRIM(struct device_configuration_struct,
        rf_rx_channel, JSON_TOK_NUMBER),
JSON_OBJ_DESCR_PRIM(struct device_configuration_struct,
        rf_pan_id, JSON_TOK_NUMBER),
JSON_OBJ_DESCR_PRIM(struct device_configuration_struct,
        rf_tx_power, JSON_TOK_NUMBER),
JSON_OBJ_DESCR_PRIM(struct device_configuration_struct,
        rf_tx_interval, JSON_TOK_NUMBER),
JSON_OBJ_DESCR_PRIM(struct device_configuration_struct,
        rf_rx_interval, JSON_TOK_NUMBER),
JSON_OBJ_DESCR_PRIM(struct device_configuration_struct,
        rf_nd_interval, JSON_TOK_NUMBER),
JSON_OBJ_DESCR_PRIM(struct device_configuration_struct,
        rf_ca_1, JSON_TOK_NUMBER),
JSON_OBJ_DESCR_PRIM(struct device_configuration_struct,
        rf_ca_2, JSON_TOK_NUMBER),
JSON_OBJ_DESCR_PRIM(struct device_configuration_struct,
        rf_ca_3, JSON_TOK_NUMBER),
JSON_OBJ_DESCR_PRIM(struct device_configuration_struct,
        pktopt, JSON_TOK_NUMBER) };

// Extra appended JSON object simulates extension of device configuration in
// later firmware revisionS and what happens on older devices.
const char excess_json_string[] = ",\"other_field\":123456}";
const size_t excess_json_string_len = sizeof (excess_json_string);

void main(void) {
    printf("=============================================================\n");
    printf("JSON on NFFS test\n");
    printf("=============================================================\n\n");

    fs_file_t file_p;
    int ret;

    //--------------------------------------------------------------------------
    // Erase Flash device
    //--------------------------------------------------------------------------

#if ERASE_DEVICE
    // Erase device if configured
    struct device *flash_dev = device_get_binding(
            CONFIG_SPI_FLASH_W25QXXXX_DRV_NAME);
    flash_write_protection_set(flash_dev, false);
    flash_erase(flash_dev, 0, CONFIG_SPI_FLASH_W25QXXXX_FLASH_SIZE);
    return;
#endif

    //--------------------------------------------------------------------------
    // Initialize device configuration
    //--------------------------------------------------------------------------
    printf("Initialize a device configuration\n");
    static struct device_configuration_struct dc = {
            .rf_tx_channel = 1,
            .rf_rx_channel = 2,
            .rf_pan_id = 0x8114,
            .rf_tx_power = 3,
            .rf_tx_interval = 4,
            .rf_rx_interval = 5,
            .rf_nd_interval = 6,
            .rf_ca_1 = 7,
            .rf_ca_2 = 8,
            .rf_ca_3 = 9,
            .pktopt = 0xcafe};

    //--------------------------------------------------------------------------
    // Encode as JSON string
    //--------------------------------------------------------------------------
    printf("Calculate length of encoded JSON string:");
    ssize_t enc_len = json_calc_encoded_len(device_configuration_descr,
            ARRAY_SIZE(device_configuration_descr), &dc);
    printf(" %u\n", enc_len);

    // Allocate a buffer of appropriate size
    uint8_t *buf;
    if (k_mem_slab_alloc(&mem_slab, (void *) &buf, 100) != 0) {
        printf("Memory slab allocation timed out\n");
        k_sleep(100);
        return;
    }
    else {
        printf("Allocated a memory slab\n");
    }

    printf("Encode data into JSON string:\n");
    ret = json_obj_encode_buf(device_configuration_descr,
            ARRAY_SIZE(device_configuration_descr), &dc, buf, MEM_SLAB_SIZE);
    if (ret) {
        printf("ERROR: JSON encoding failed\n");
        k_sleep(100);
        return;
    }
    else {
        for (int i = 0; i < enc_len; i++) {
            printf("%c", buf[i]);
        }
        printf("\n");
    }

    //--------------------------------------------------------------------------
    // Parse JSON string as a test
    //--------------------------------------------------------------------------
    struct device_configuration_struct test_dc;
    printf("Parsing JSON string\n");
    ret = json_obj_parse(buf, enc_len, device_configuration_descr,
            ARRAY_SIZE(device_configuration_descr), &test_dc);
    if (ret != (1 << ARRAY_SIZE(device_configuration_descr)) - 1) {
        printf("ERROR: JSON parsing failed: %d\n", ret);
        k_sleep(100);
        return;
    }
    else {
        printf("RF TX Channel: %d\n", test_dc.rf_tx_channel);
        printf("RF RX Channel: %d\n", test_dc.rf_rx_channel);
        printf("RF PAN ID: 0x%04x\n", test_dc.rf_pan_id);
    }

    //--------------------------------------------------------------------------
    // Add an unspecified object to JSON string
    //--------------------------------------------------------------------------
    printf("Add additional unspecified JSON object to string:\n");
    memcpy(&buf[enc_len - 1], &excess_json_string, excess_json_string_len);
    enc_len += excess_json_string_len;
    for (int i = 0; i < enc_len; i++) {
        printf("%c", buf[i]);
    }
    printf("\n");

    //--------------------------------------------------------------------------
    // Write JSON string to a file on Flash device
    //--------------------------------------------------------------------------
#if WRITE_TO_FILE
    // Create file in root directory
    // Don't forget the forward slash '/' that denotes the root directory.
    // Else, file creation will fail.
    ret = fs_open(&file_p, "/config.txt");
    if (ret) {
        printf("Could not open file\n");
        k_sleep(100);
        return;
    }
    else {
        printf("File open\n");
    }

    ret = fs_write(&file_p, buf, enc_len);
    if (ret != enc_len) {
        printf("Something went wrong during writing: %d\n", ret);
        k_sleep(100);
        return;
    }
    else {
        printf("Wrote string to file\n");
    }
    k_sleep(100);

    ret = fs_close(&file_p);
    if (ret) {
        printf("Could not close file\n");
    }
    else {
        printf("File closed\n");
    }
    k_sleep(100);
#endif

    //--------------------------------------------------------------------------
    // Read JSON string back from file
    //--------------------------------------------------------------------------
    memset(buf, 0, sizeof (buf));
    // Need to find out the file size in order to read right number of bytes.
    struct fs_dirent config_file_info;
    ret = fs_stat("/config.txt", &config_file_info);
    if (ret) {
        printf("ERROR: Could not get JSON file info\n");
        return;
    }
    else {
        printf("JSON file is %u bytes long\n", config_file_info.size);
    }

    ret = fs_open(&file_p, "/config.txt");
    if (ret) {
        printf("Could not open file\n");
        k_sleep(100);
        return;
    }
    else {
        printf("File open\n");
    }

    ret = fs_read(&file_p, buf, config_file_info.size);
    if (ret != config_file_info.size) {
        printf("Something went wrong during read: %d\n", ret);
        k_sleep(100);
        return;
    }
    else {
        printf("Read from file:\n");
        for (int i = 0; i < config_file_info.size; i++) {
            printf("%c", buf[i]);
        }
        printf("\n");
    }

    ret = fs_close(&file_p);
    if (ret) {
        printf("Could not close file\n");
    }
    else {
        printf("File closed\n");
    }

    //--------------------------------------------------------------------------
    // Parse JSON string that was read from file
    //--------------------------------------------------------------------------
    struct device_configuration_struct test_dc2;
    printf("Parsing JSON string\n");
    ret = json_obj_parse(buf, config_file_info.size, device_configuration_descr,
            ARRAY_SIZE(device_configuration_descr), &test_dc2);
    if (ret != (1 << ARRAY_SIZE(device_configuration_descr)) - 1) {
        printf("ERROR: JSON parsing failed: %d\n", ret);
        k_sleep(100);
        return;
    }
    else {
        printf("RF TX Channel: %u\n", test_dc2.rf_tx_channel);
        printf("RF RX Channel: %u\n", test_dc2.rf_rx_channel);
        printf("RF PAN ID: 0x%04x\n", test_dc2.rf_pan_id);
        printf("RF TX Power: %d\n", test_dc2.rf_tx_power);
        printf("RF TX Interval: %u\n", test_dc2.rf_tx_interval);
        printf("RF RX Interval: %u\n", test_dc2.rf_rx_interval);
        printf("RF ND Interval: %u\n", test_dc2.rf_nd_interval);
        printf("RF CA Param 1: %d\n", test_dc2.rf_ca_1);
        printf("RF CA Param 2: %d\n", test_dc2.rf_ca_2);
        printf("RF CA Param 3: %d\n", test_dc2.rf_ca_3);
        printf("PKTOPT: 0x%08x\n", test_dc2.pktopt);
    }

    //--------------------------------------------------------------------------
    // Free memory slab
    //--------------------------------------------------------------------------
    k_mem_slab_free(&mem_slab, (void *) &buf);

    printf("End of application\n");
}
