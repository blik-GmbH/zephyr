#ifndef _PB_SYSHDR_H_
#define _PB_SYSHDR_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#ifdef PB_ENABLE_MALLOC
#include <stdlib.h>
#endif

/* The minimal stdint that ships with zephyr doesn't define any least types */
typedef int8_t int_least8_t;
typedef uint8_t uint_least8_t;
typedef int16_t int_least16_t;
typedef uint16_t uint_least16_t;

#endif
