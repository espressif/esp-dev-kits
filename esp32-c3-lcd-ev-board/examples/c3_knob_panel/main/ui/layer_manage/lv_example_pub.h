/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#ifndef LV_EXAMPLE_PUB_H
#define LV_EXAMPLE_PUB_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "esp_err.h"
#include "esp_log.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/


/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

typedef struct {
    uint32_t time_base;
    uint32_t timeOut;
} time_out_count;

extern bool is_time_out(time_out_count *tm);

extern bool set_time_out(time_out_count *tm, uint32_t ms);

extern bool reload_time_out(time_out_count *tm);

extern void lv_home_create(void);

#endif /*LV_EXAMPLE_PUB_H*/
