/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"

#include "lv_example_pub.h"

static const char *TAG = "LVGL_PUB";

lv_style_t style_scrollbar;
lv_style_t style_btn;
lv_style_t style_btn_pr;
lv_style_t style_btn_chk;
lv_style_t style_btn_dis;
lv_style_t style_title;
lv_style_t style_artist;
lv_style_t style_time;

lv_style_t style_main;
lv_style_t style_indicator;
lv_style_t style_pressed_color;

uint32_t sys_clock_getSecond()
{
    uint32_t sec = (lv_tick_get() + 1234567) / 1000;
    return sec;
}

void lv_style_pre_init()
{
    static const lv_font_t *font_small;
    static const lv_font_t *font_medium;

    ESP_LOGI(TAG, "style init");
#if LV_DEMO_MUSIC_LARGE
    font_small = &lv_font_montserrat_16;
    font_medium = &lv_font_montserrat_22;
#else
    font_small = &lv_font_montserrat_12;
    font_medium = &lv_font_montserrat_16;
#endif

    static const lv_coord_t grid_cols[] = {LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
#if LV_DEMO_MUSIC_LARGE
    static const lv_coord_t grid_rows[] = {35,  30, LV_GRID_TEMPLATE_LAST};
#else
    static const lv_coord_t grid_rows[] = {22,  17, LV_GRID_TEMPLATE_LAST};
#endif

    lv_style_init(&style_scrollbar);
    lv_style_set_width(&style_scrollbar,  4);
    lv_style_set_bg_opa(&style_scrollbar, LV_OPA_COVER);
    lv_style_set_bg_color(&style_scrollbar, lv_color_hex3(0xeee));
    lv_style_set_radius(&style_scrollbar, LV_RADIUS_CIRCLE);
    lv_style_set_pad_right(&style_scrollbar, 4);

    lv_style_init(&style_btn);
    lv_style_set_bg_opa(&style_btn, LV_OPA_TRANSP);
    lv_style_set_grid_column_dsc_array(&style_btn, grid_cols);
    lv_style_set_grid_row_dsc_array(&style_btn, grid_rows);
    lv_style_set_grid_row_align(&style_btn, LV_GRID_ALIGN_CENTER);
    lv_style_set_layout(&style_btn, LV_LAYOUT_GRID);

    lv_style_init(&style_btn_pr);
    lv_style_set_bg_opa(&style_btn_pr, LV_OPA_COVER);
    lv_style_set_bg_color(&style_btn_pr,  lv_color_hex(0x4c4965));

    lv_style_init(&style_btn_chk);
    lv_style_set_bg_opa(&style_btn_chk, LV_OPA_COVER);
    lv_style_set_bg_color(&style_btn_chk, lv_color_hex(0x4c4965));

    lv_style_init(&style_btn_dis);
    lv_style_set_text_opa(&style_btn_dis, LV_OPA_40);
    lv_style_set_img_opa(&style_btn_dis, LV_OPA_40);

    lv_style_init(&style_title);
    lv_style_set_text_font(&style_title, font_medium);
    lv_style_set_text_color(&style_title, lv_color_hex(0xffffff));

    lv_style_init(&style_artist);
    lv_style_set_text_font(&style_artist, font_small);
    lv_style_set_text_color(&style_artist, lv_color_hex(0xb1b0be));

    lv_style_init(&style_time);
    lv_style_set_text_font(&style_time, font_medium);
    lv_style_set_text_color(&style_time, lv_color_hex(0xffffff));

    /*Create a transition*/
    static const lv_style_prop_t props[] = {LV_STYLE_BG_COLOR, 0};
    static lv_style_transition_dsc_t transition_dsc;
    lv_style_transition_dsc_init(&transition_dsc, props, lv_anim_path_linear, 300, 0, NULL);

    lv_style_init(&style_main);
    lv_style_set_bg_opa(&style_main, LV_OPA_COVER);
    lv_style_set_bg_color(&style_main, lv_color_hex(0xFFFFFF));
    lv_style_set_radius(&style_main, LV_RADIUS_CIRCLE);
    lv_style_set_pad_ver(&style_main, -2); /*Makes the indicator larger*/

    lv_style_init(&style_indicator);//click
    lv_style_set_bg_opa(&style_indicator, LV_OPA_COVER);
    lv_style_set_bg_color(&style_indicator, lv_color_hex(COLOUR_YELLOW));
    lv_style_set_radius(&style_indicator, LV_RADIUS_CIRCLE);
    lv_style_set_transition(&style_indicator, &transition_dsc);

    lv_style_init(&style_pressed_color);//slider
    lv_style_set_bg_color(&style_pressed_color, lv_color_hex(COLOUR_YELLOW));
}
