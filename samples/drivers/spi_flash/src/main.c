/*
 * Copyright (c) 2016 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <flash.h>
#include <device.h>
#include <stdio.h>

#define FLASH_TEST_REGION_OFFSET 0xff000
#define FLASH_SECTOR_SIZE        4096
#define TEST_DATA_BYTE_0         0x55
#define TEST_DATA_BYTE_1         0xaa
#define TEST_DATA_LEN            2

void main(void)
{
	struct device *flash_dev;
	u8_t buf[TEST_DATA_LEN];

	printf("\nW25QXXXX SPI flash testing\n");
	printf("==========================\n");

	flash_dev = device_get_binding(CONFIG_SPI_FLASH_W25QXXXX_DRV_NAME);

	if (!flash_dev) {
		printf("SPI flash driver was not found!\n");
		return;
	}

	/* Write protection needs to be disabled in W25QXXXX flash before
	 * each write or erase. This is because the flash component turns
	 * on write protection automatically after completion of write and
	 * erase operations.
	 */
	printf("\nTest 1: Flash erase\n");
	flash_write_protection_set(flash_dev, false);
	if (flash_erase(flash_dev,
			FLASH_TEST_REGION_OFFSET,
			FLASH_SECTOR_SIZE) != 0) {
		printf("   Flash erase failed!\n");
	} else {
		printf("   Flash erase succeeded!\n");
	}

	printf("\nTest 2: Flash write\n");
	flash_write_protection_set(flash_dev, false);

	buf[0] = TEST_DATA_BYTE_0;
	buf[1] = TEST_DATA_BYTE_1;
	printf("   Attempted to write %x %x\n", buf[0], buf[1]);
	if (flash_write(flash_dev, FLASH_TEST_REGION_OFFSET, buf,
	    TEST_DATA_LEN) != 0) {
		printf("   Flash write failed!\n");
		return;
	}

	if (flash_read(flash_dev, FLASH_TEST_REGION_OFFSET, buf,
	    TEST_DATA_LEN) != 0) {
		printf("   Flash read failed!\n");
		return;
	}
	printf("   Data read %x %x\n", buf[0], buf[1]);

	if ((buf[0] == TEST_DATA_BYTE_0) && (buf[1] == TEST_DATA_BYTE_1)) {
		printf("   Data read matches with data written. Good!!\n");
	} else {
		printf("   Data read does not match with data written!!\n");
	}


	/*
	 * Power mode test
	 */
	printf("\nTest 3: Flash power modes\n");
	int ret;
	uint32_t pm_state;

	printf("Query Flash power mode\n");
	ret = device_get_power_state(flash_dev, &pm_state);
	if (ret || pm_state != DEVICE_PM_ACTIVE_STATE){
	    printf("ERROR: Power mode should be DEVICE_PM_ACTIVE_STATE\n");
	}
	else{
	    printf("GOOD: Power mode is DEVICE_PM_ACTIVE_STATE\n");
	}

	while (1){
	    printf("15 seconds active state\n");
	    k_sleep(15000);
	    printf("Switch device to power-down mode\n");
	    ret = device_set_power_state(flash_dev, DEVICE_PM_LOW_POWER_STATE);
	    if (ret){
	        printf("ERROR: Couldn't set power-down mode\n");
	    }
	    ret = device_get_power_state(flash_dev, &pm_state);
	    if (ret || pm_state != DEVICE_PM_LOW_POWER_STATE){
            printf("ERROR: Power mode should be DEVICE_PM_LOW_POWER_STATE\n");
        }
        else{
            printf("GOOD: Power mode is DEVICE_PM_LOW_POWER_STATE\n");
        }

	    printf("15 seconds power-down state\n");
        k_sleep(15000);
        printf("Switch device to active state\n");
        ret = device_set_power_state(flash_dev, DEVICE_PM_ACTIVE_STATE);
        if (ret){
            printf("ERROR: Couldn't set active state\n");
        }
        ret = device_get_power_state(flash_dev, &pm_state);
        if (ret || pm_state != DEVICE_PM_ACTIVE_STATE){
            printf("ERROR: Power mode should be DEVICE_PM_ACTIVE_STATE\n");
        }
        else{
            printf("GOOD: Power mode is DEVICE_PM_ACTIVE_STATE\n");
        }
	}
}
