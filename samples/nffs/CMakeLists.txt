include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(NONE)

zephyr_compile_definitions(
    -DFLASH_AREA_NFFS_OFFSET=0
    -DFLASH_AREA_NFFS_SIZE=8192
)

target_link_libraries(app NFFS)
FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})
