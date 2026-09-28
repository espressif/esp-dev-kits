/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <stdint.h>

typedef struct {
    uint16_t width;
    uint16_t height;
} display_info_t ;

bool display_get_info(display_info_t *info);
bool display_init();
