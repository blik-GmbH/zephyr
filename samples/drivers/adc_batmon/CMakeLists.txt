
include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(NONE)

target_sources(app PRIVATE src/main.c)
target_sources(app PRIVATE $ENV{ZEPHYR_BASE}/ext/hal/nxp/mcux/drivers/fsl_vref.c)
