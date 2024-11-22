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
#define USER_CTRLBTN_MAX        7

#define TIME_ENTER_CLOCK_2MIN    (2*60*1000)/6//2min (2*60*1000)/(50 ms)

#define COLOUR_BLACK            0xFFFFFF
#define COLOUR_GREY             0xBFBFBF
#define COLOUR_WHITE            0xFFFFFF
#define COLOUR_YELLOW           0xE9BD85
#define COLOUR_BG_GREY          0x3F3F3F
#define COLOUR_BG1_GREY         0x2F2F2F

typedef struct {
    const lv_img_dsc_t *addr_weather_icon;
    const char *status;
    const char *location;
    const uint8_t temperature;
    const char *tempRange;
} weather_info_list_t;

typedef struct {
    const lv_img_dsc_t *addr_open;
    const lv_img_dsc_t *addr_close;
    const char *btn_name;
} warm_fan_info_list_t;

typedef struct {
    const lv_img_dsc_t *addr_open;
    const lv_img_dsc_t *addr_close;
    const char *btn_name;
} water_heat_info_list_t;

extern lv_layer_t show_set_layer;
extern lv_layer_t test_layer;
extern lv_layer_t weather_Layer;
extern lv_layer_t main_Layer;
extern lv_layer_t ctrl_Layer;
extern lv_layer_t clock_screen_layer;
extern lv_layer_t show_music_layer;

extern lv_style_t style_main;
extern lv_style_t style_indicator;
extern lv_style_t style_pressed_color;

extern lv_style_t style_scrollbar;
extern lv_style_t style_btn;
extern lv_style_t style_btn_pr;
extern lv_style_t style_btn_chk;
extern lv_style_t style_btn_dis;
extern lv_style_t style_title;
extern lv_style_t style_artist;
extern lv_style_t style_time;

extern const warm_fan_info_list_t warm_openclose_icon[7];
extern const weather_info_list_t weather_icon_list[8];

extern uint32_t sys_clock_getSecond();
extern void lv_style_pre_init();

#endif /*LV_EXAMPLE_PUB_H*/
