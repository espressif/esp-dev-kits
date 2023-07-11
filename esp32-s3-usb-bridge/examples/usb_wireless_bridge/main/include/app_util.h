/* SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

# pragma once

#include <sys/param.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*(x)))
#endif

#define GET_BYTE(n, b)          (((n) >> ((b) * 8)) & 0xFF)

#define EUB_ASSERT(condition)            do {                           \
                                                if (!(condition)) {     \
                                                    eub_abort();        \
                                                }                       \
                                            } while(0)

void __attribute__((noreturn)) eub_abort(void);
