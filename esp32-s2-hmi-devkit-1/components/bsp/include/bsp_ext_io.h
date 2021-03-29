/**
 * @file bsp_ext_io.h
 * @brief 
 * @version 0.1
 * @date 2021-02-24
 * 
 * @copyright Copyright 2021 Espressif Systems (Shanghai) Co. Ltd.
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *               http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    union {
        struct {
            uint8_t motion_int:1;
            uint8_t lcd_rst:1;
            uint8_t tp_int:1;
            uint8_t audio_pa:1;
            uint8_t vdd_off:1;
            uint8_t boost_en:1;
            uint8_t rmt_ctrl:1;
            uint8_t lcd_light:1;
        };
        uint8_t val;
    };
} ext_io_t;

#define BSP_EXT_IO_DEFAULT_CONFIG() {   \
        .motion_int = 1,                \
        .lcd_rst = 0,                   \
        .tp_int = 1,                    \
        .audio_pa = 0,                  \
        .vdd_off = 0,                   \
        .boost_en = 0,                  \
        .rmt_ctrl = 0,                  \
        .lcd_light = 0,                 \
    }

#define BSP_EXT_IO_DEFAULT_LEVEL() {    \
        .motion_int = 1,                \
        .lcd_rst = 1,                   \
        .tp_int = 1,                    \
        .audio_pa = 1,                  \
        .vdd_off = 0,                   \
        .boost_en = 0,                  \
        .rmt_ctrl = 1,                  \
        .lcd_light = 1,                 \
    }

#define BSP_EXT_IO_SLEEP_LEVEL() {      \
        .motion_int = 1,                \
        .lcd_rst = 0,                   \
        .tp_int = 1,                    \
        .audio_pa = 0,                  \
        .vdd_off = 1,                   \
        .boost_en = 0,                  \
        .rmt_ctrl = 1,                  \
        .lcd_light = 0,                 \
    }

#ifdef __cplusplus
extern "C" {
#endif
