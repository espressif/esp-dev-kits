/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#ifndef LV_EXAMPLE_FUNC_H
#define LV_EXAMPLE_FUNC_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"

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

typedef bool (*lv_layer_enter_cb)(void *layer);
typedef bool (*lv_layer_exit_cb)(void *layer);

typedef struct lv_layer {
    char *lv_obj_name;
    lv_obj_t *lv_obj_parent;
    lv_obj_t *lv_obj_layer;
    struct lv_layer *lv_show_layer;
    lv_layer_enter_cb enter_cb;
    lv_layer_exit_cb exit_cb;
    lv_timer_cb_t timer_cb;
    lv_timer_t *timer_handle;
} lv_layer_t;

typedef struct {
    uint32_t time_base;
    uint32_t timeOut;
} time_out_count;

extern bool is_time_out(time_out_count *tm);

extern bool set_time_out(time_out_count *tm, uint32_t ms);

extern bool reload_time_out(time_out_count *tm);

extern void lv_create_home(lv_layer_t *home_layer);

extern void enter_clock_time();

extern void feed_clock_time();

extern void lv_create_clock(lv_layer_t *clock_layer, uint32_t tmOut);

extern void lv_func_goto_layer(lv_layer_t *dst_layer);

#endif /*LV_EXAMPLE_FUNC_H*/
