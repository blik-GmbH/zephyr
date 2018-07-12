zephyr_library()
zephyr_library_sources(pinmux.c)
zephyr_library_include_directories(${PROJECT_SOURCE_DIR}/drivers)

zephyr_compile_definitions(
    -DFLASH_AREA_NFFS_OFFSET=0
    -DFLASH_AREA_NFFS_SIZE=8192
)
