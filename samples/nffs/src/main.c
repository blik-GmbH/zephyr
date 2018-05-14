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


void main(void)
{
    const char test_phrase[]    = "Hello blik!\n";
    const char test_phrase2[]   = "Hello blob!\n";
    const char test_phrase3[]   = "Hello blak!\n";
    const size_t test_phrase_len = 12;
    char buf[test_phrase_len];

    printf("NFFS test\n\n");

	fs_file_t file_p;
	int ret;

#if ERASE_DEVICE
	// Erase device if configured
    struct device *flash_dev = device_get_binding(CONFIG_SPI_FLASH_W25QXXXX_DRV_NAME);
    flash_write_protection_set(flash_dev, false);
    flash_erase(flash_dev, 0, CONFIG_SPI_FLASH_W25QXXXX_FLASH_SIZE);
    return;
#endif

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

	ret = fs_write(&file_p, &test_phrase, test_phrase_len);
	if(ret != test_phrase_len)
	{
	    printf("Something went wrong during writing\n");
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

    ret = fs_read(&file_p, &buf, test_phrase_len);
    if(ret != test_phrase_len)
    {
        printf("Something went wrong during read\n");
    }
    else
    {
        printf("Read from file:\n");
        for(int i = 0; i < test_phrase_len; i++){
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




#if WRITE_TO_FILE
    // Create another file in the root directory
    ret = fs_open(&file_p, "/file1.txt");
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

    ret = fs_write(&file_p, &test_phrase2, test_phrase_len);
    if(ret != test_phrase_len)
    {
        printf("Something went wrong during writing\n");
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

    // Try to also read from that file
    ret = fs_open(&file_p, "/file1.txt");
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

    ret = fs_read(&file_p, &buf, test_phrase_len);
    if(ret != test_phrase_len)
    {
        printf("Something went wrong during read\n");
    }
    else
    {
        printf("Read from file:\n");
        for(int i = 0; i < test_phrase_len; i++){
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




#if WRITE_TO_FILE
    // Create a third file in the root directory
    ret = fs_open(&file_p, "/file2.txt");
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

    ret = fs_write(&file_p, &test_phrase3, test_phrase_len);
    if(ret != test_phrase_len)
    {
        printf("Something went wrong during writing\n");
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

    // Also try to read from the third file
    ret = fs_open(&file_p, "/file2.txt");
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

    ret = fs_read(&file_p, &buf, test_phrase_len);
    if(ret != test_phrase_len)
    {
        printf("Something went wrong during read\n");
    }
    else
    {
        printf("Read from file:\n");
        for(int i = 0; i < test_phrase_len; i++){
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



    //--------------------------------------------------------------------------

    printf("Subdirectory\n");

#if WRITE_TO_FILE
    // Create a subdirectory AND a file within that subdirectory
    ret = fs_mkdir("/mydir");
    if(ret)
    {
        printf("Could not make directory\n");
    }
    else
    {
        printf("Created directory\n");
    }

    ret = fs_open(&file_p, "/mydir/file4.txt");
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

    ret = fs_write(&file_p, &test_phrase, test_phrase_len);
    if(ret != test_phrase_len)
    {
        printf("Something went wrong during writing\n");
        k_sleep(100);
        return;
    }
    else
    {
        printf("Wrote string to file\n");
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
#endif

    // Try to read from the file in the subdirectory
    ret = fs_open(&file_p, "/mydir/file4.txt");
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

    // char buf[test_phrase_len];
    ret = fs_read(&file_p, &buf, test_phrase_len);
    if(ret != test_phrase_len)
    {
        printf("Something went wrong during read: %d\n", ret);
    }
    else
    {
        printf("Read from file:\n");
        for(int i = 0; i < test_phrase_len; i++){
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

	printf("End of application\n");
}
