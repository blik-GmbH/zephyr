if(BOARD STREQUAL nrf51_pca10028)
    set(CONF_FILE nrf5x.conf)
elseif(BOARD STREQUAL nrf52_pca10040)
    set(CONF_FILE nrf5x.conf)
elseif(BOARD STREQUAL nrf52840_pca10056)
    set(CONF_FILE nrf5x.conf)
endif()

include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(NONE)

if(BOARD STREQUAL qemu_x86)
    zephyr_compile_definitions(
        -DTEST_FLASH_OFFSET=0
        -DFLASH_AREA_NFFS_OFFSET=0
        -DFLASH_AREA_NFFS_SIZE=1048576
    )
elseif(BOARD STREQUAL nrf51_pca10028)
    zephyr_compile_definitions(
        -DTEST_FLASH_OFFSET=0x20000
    )
elseif(BOARD STREQUAL nrf52_pca10040)
    zephyr_compile_definitions(
        -DTEST_FLASH_OFFSET=0x20000
    )
elseif(BOARD STREQUAL nrf52840_pca10056)
    zephyr_compile_definitions(
        -DTEST_FLASH_OFFSET=0x20000
    )
elseif(BOARD STREQUAL akita_gen2_devboard)
    zephyr_compile_definitions(
        -DTEST_FLASH_OFFSET=0
        -DFLASH_AREA_NFFS_OFFSET=0
        -DFLASH_AREA_NFFS_SIZE=4096
    )
endif()

if(NOT TEST)
    target_compile_definitions(app PRIVATE
        -DTEST_basic
	)
else()
    target_compile_definitions(app PRIVATE
        -DTEST_${TEST}
	)
endif()


target_link_libraries(app NFFS)
target_sources(app PRIVATE src/main.c)
target_sources(app PRIVATE src/nffs_test_system_01.c)
target_sources(app PRIVATE src/nffs_test_utils.c)
target_sources(app PRIVATE src/test_append.c)
target_sources(app PRIVATE src/test_cache_large_file.c)
target_sources(app PRIVATE src/test_corrupt_block.c)
target_sources(app PRIVATE src/test_corrupt_scratch.c)
target_sources(app PRIVATE src/test_gc_on_oom.c)
target_sources(app PRIVATE src/test_gc.c)
target_sources(app PRIVATE src/test_incomplete_block.c)
target_sources(app PRIVATE src/test_large_system.c)
target_sources(app PRIVATE src/test_large_unlink.c)
target_sources(app PRIVATE src/test_large_write.c)
target_sources(app PRIVATE src/test_long_filename.c)
target_sources(app PRIVATE src/test_lost_found.c)
target_sources(app PRIVATE src/test_many_children.c)
target_sources(app PRIVATE src/test_mkdir.c)
target_sources(app PRIVATE src/test_open.c)
target_sources(app PRIVATE src/test_overwrite_many.c)
target_sources(app PRIVATE src/test_overwrite_one.c)
target_sources(app PRIVATE src/test_overwrite_three.c)
target_sources(app PRIVATE src/test_overwrite_two.c)
target_sources(app PRIVATE src/test_performance.c)
target_sources(app PRIVATE src/test_read.c)
target_sources(app PRIVATE src/test_readdir.c)
target_sources(app PRIVATE src/test_rename.c)
target_sources(app PRIVATE src/test_split_file.c)
target_sources(app PRIVATE src/test_unlink.c)
target_sources(app PRIVATE src/test_wear_level.c)
