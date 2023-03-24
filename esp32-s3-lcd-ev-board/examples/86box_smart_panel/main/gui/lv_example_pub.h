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
#include <stdbool.h>
#include "esp_err.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "lv_schedule_basic.h"

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

#define USER_MAINMENU_MAX       3

//#define TIME_ENTER_CLOCK_2MIN    (2*60*1000)/5///2min (2*60*1000)/(50 ms)
#define TIME_ENTER_CLOCK_2MIN    (0xFF*60*1000)/1//2min (2*60*1000)/(50 ms)

#define COLOUR_BLACK            0x000000
#define COLOUR_WHITE            0xFFFFFF
#define COLOUR_YELLOW           0xE9BD85
#define COLOUR_GREY_1F          0x1F1F1F
#define COLOUR_GREY_2F          0x2F2F2F

#define COLOUR_GREY_BF          0xBFBFBF
#define COLOUR_GREY_4F          0x4F4F4F
#define COLOUR_GREY_8F          0x8F8F8F

typedef void (* lv_obj_sprite_create)(lv_obj_t *parent);

typedef enum{
    LV_EVENT_EXIT_CLOCK,
    LV_EVENT_LIGHT_RGB_SET,
    LV_EVENT_LIGHT_ONOFF_SET,

    LV_EVENT_AC_SET_ON,
    LV_EVENT_AC_SET_OFF,
    LV_EVENT_AC_TEMP_ADD,
    LV_EVENT_AC_TEMP_DEC,
}LV_EVENT_TYPE;

typedef struct{
    LV_EVENT_TYPE event;
    void * event_data;
}lv_event_info_t;

typedef struct {
    const char *set_list_name;
    lv_obj_sprite_create sprite_create_func;
    lv_obj_t *sprite_parent;
} sprite_create_func_t;

extern lv_layer_t set_layer;
extern lv_layer_t show_set_layer;
extern lv_layer_t main_Layer;
extern lv_layer_t clock_screen_layer;
extern lv_layer_t boot_Layer;

extern lv_style_t style_main, style_knob, style_indicator;

extern uint32_t sys_clock_getSecond();

extern void lv_style_pre_init();

extern esp_err_t app_lvgl_create_queue();

extern esp_err_t app_lvgl_send_event(lv_event_info_t* event);

extern esp_err_t app_lvgl_get_event(lv_event_info_t *result, TickType_t xTicksToWait);

#endif /*LV_EXAMPLE_PUB_H*/
