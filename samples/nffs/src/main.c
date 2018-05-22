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
#include <init.h>
#include <device.h>
#include <power.h>
#include <stdio.h>

/**
 * @brief Erases entire device and returns.
 *
 * Use this if you want the Flash device to be re-formatted upon next boot.
 */
#define ERASE_DEVICE            0
#define FLASH_ERASE_PRIORITY    99
/**
 * @brief Switch determines whether files are written.
 *
 * Set to '0' to disable writes. Reads may fail if no files are written before.
 */
#define WRITE_TO_FILE           0
/**
 * @brief Switch determines whether files are read.
 *
 * Set to '0' if files shall not be read.
 * Setting this to '1' without prior write will result in "could not open file"
 * prints.
 */
#define READ_FROM_FILE          1


const char dirpath0[] = "/mydir";
const char filepath0[] = "/file0.txt";
const char filepath1[] = "/file1.txt";
const char filepath2[] = "/file2.txt";
const char filepath3[] = "/mydir/file3.txt";

const char teststring0[] = "Hello blik!\n";
const char teststring1[] = "Hello blob!\n";
const char teststring2[] = "Hello blak!\n";


void main(void) {
    char buf[sizeof(teststring0)];

    printf("NFFS test\n\n");

    fs_file_t file_p;
    int ret;

    //--------------------------------------------------------------------------

    if (WRITE_TO_FILE) {
        // Create file in root directory
        // Don't forget the forward slash '/' that denotes the root directory.
        // Else, file creation will fail.
        ret = fs_open(&file_p, filepath0);
        if (ret) {
            printf("Could not open file\n");
            k_sleep(100);
            return;
        }
        else {
            printf("File open\n");
        }

        ret = fs_write(&file_p, &teststring0, sizeof(teststring0));
        if (ret != sizeof(teststring0)) {
            printf("Something went wrong during writing\n");
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
    }

    if (READ_FROM_FILE) {
        // Read the contents of file in root directory.
        ret = fs_open(&file_p, filepath0);
        if (ret) {
            printf("Could not open file\n");
            k_sleep(100);
            return;
        }
        else {
            printf("File open\n");
        }

        ret = fs_read(&file_p, &buf, sizeof(teststring0));
        if (ret != sizeof(teststring0)) {
            printf("Something went wrong during read\n");
        }
        else {
            printf("Read from file:\n");
            for (int i = 0; i < sizeof(teststring0); i++) {
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
    }

    //--------------------------------------------------------------------------

    if (WRITE_TO_FILE) {
        // Create another file in the root directory
        ret = fs_open(&file_p, filepath1);
        if (ret) {
            printf("Could not open file\n");
            k_sleep(100);
            return;
        }
        else {
            printf("File open\n");
        }

        ret = fs_write(&file_p, &teststring1, sizeof(teststring1));
        if (ret != sizeof(teststring1)) {
            printf("Something went wrong during writing\n");
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
    }

    if (READ_FROM_FILE) {
        // Try to also read from that file
        ret = fs_open(&file_p, filepath1);
        if (ret) {
            printf("Could not open file\n");
            k_sleep(100);
            return;
        }
        else {
            printf("File open\n");
        }

        ret = fs_read(&file_p, &buf, sizeof(teststring1));
        if (ret != sizeof(teststring1)) {
            printf("Something went wrong during read\n");
        }
        else {
            printf("Read from file:\n");
            for (int i = 0; i < sizeof(teststring1); i++) {
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
    }

    //--------------------------------------------------------------------------

    if (WRITE_TO_FILE) {
        // Create a third file in the root directory
        ret = fs_open(&file_p, filepath2);
        if (ret) {
            printf("Could not open file\n");
            k_sleep(100);
            return;
        }
        else {
            printf("File open\n");
        }

        ret = fs_write(&file_p, &teststring2, sizeof(teststring2));
        if (ret != sizeof(teststring2)) {
            printf("Something went wrong during writing\n");
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
    }

    if (READ_FROM_FILE) {
        // Also try to read from the third file
        ret = fs_open(&file_p, filepath2);
        if (ret) {
            printf("Could not open file\n");
            k_sleep(100);
            return;
        }
        else {
            printf("File open\n");
        }

        ret = fs_read(&file_p, &buf, sizeof(teststring2));
        if (ret != sizeof(teststring2)) {
            printf("Something went wrong during read\n");
        }
        else {
            printf("Read from file:\n");
            for (int i = 0; i < sizeof(teststring2); i++) {
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
    }

    //--------------------------------------------------------------------------

    printf("Subdirectory\n");

    if (WRITE_TO_FILE) {
        // Create a subdirectory AND a file within that subdirectory
        ret = fs_mkdir(dirpath0);
        if (ret) {
            printf("Could not make directory\n");
        }
        else {
            printf("Created directory\n");
        }

        ret = fs_open(&file_p, filepath3);
        if (ret) {
            printf("Could not open file\n");
            k_sleep(100);
            return;
        }
        else {
            printf("File open\n");
        }

        ret = fs_write(&file_p, &teststring0, sizeof(teststring0));
        if (ret != sizeof(teststring0)) {
            printf("Something went wrong during writing\n");
            k_sleep(100);
            return;
        }
        else {
            printf("Wrote string to file\n");
        }

        ret = fs_close(&file_p);
        if (ret) {
            printf("Could not close file\n");
        }
        else {
            printf("File closed\n");
        }
    }

    if (READ_FROM_FILE) {
        // Try to read from the file in the subdirectory
        ret = fs_open(&file_p, filepath3);
        if (ret) {
            printf("Could not open file\n");
            k_sleep(100);
            return;
        }
        else {
            printf("File open\n");
        }

        ret = fs_read(&file_p, &buf, sizeof(teststring0));
        if (ret != sizeof(teststring0)) {
            printf("Something went wrong during read: %d\n", ret);
        }
        else {
            printf("Read from file:\n");
            for (int i = 0; i < sizeof(teststring0); i++) {
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
    }

    //--------------------------------------------------------------------------

    printf("End of application\n");

    printf("\n");
    struct device *flash_dev = device_get_binding(
                CONFIG_SPI_FLASH_W25QXXXX_DRV_NAME);
    ret = device_set_power_state(flash_dev, SYS_PM_LOW_POWER_STATE);

    while(1){
        k_sleep(1000);
    }

    return;
}

/**
 * @brief Erases Flash device before NFFS initialization if #ERASE_DEVICE is set
 */
static int nffs_sample_flash_erase(struct device *dev) {
    if (ERASE_DEVICE) {
        struct device *flash_dev = device_get_binding(
                CONFIG_SPI_FLASH_W25QXXXX_DRV_NAME);
        flash_write_protection_set(flash_dev, false);
        flash_erase(flash_dev, 0, CONFIG_SPI_FLASH_W25QXXXX_FLASH_SIZE);
    }
    return 0;
}

SYS_INIT(nffs_sample_flash_erase, POST_KERNEL, FLASH_ERASE_PRIORITY);
