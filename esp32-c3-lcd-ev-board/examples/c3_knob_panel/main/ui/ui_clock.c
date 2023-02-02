/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <time.h>
#include "lvgl.h"
#include "ui.h"
#include "ui_clock.h"

#include "lv_example_pub.h"
#include "lv_example_func.h"
#include "lv_example_image.h"

static lv_obj_t  *page, *meter = NULL;
static lv_meter_indicator_t *indic_sec ;
static lv_meter_indicator_t *indic_min ;
static lv_meter_indicator_t *indic_hour ;

static time_out_count time_20ms;

static bool clock_layer_enter_cb(struct lv_layer_t *layer);
static bool clock_layer_exit_cb(struct lv_layer_t *layer);
static void clock_layer_timer_cb(lv_timer_t *tmr);

lv_layer_t clock_Layer = {
    .lv_obj_name    = "clock_Layer",
    .lv_obj_parent  = NULL,
    .lv_obj_layer   = NULL,
    .lv_show_layer  = NULL,
    .enter_cb       = clock_layer_enter_cb,
    .exit_cb        = clock_layer_exit_cb,
    .timer_cb       = clock_layer_timer_cb,
};

static void clock_handler()
{
    static time_t time_last = 0;
    time_t now;
    struct tm timeinfo = {0};
    time(&now);
    if (now != time_last) {
        time_last = now;
        localtime_r(&now, &timeinfo);
        printf("time=%d:%d:%d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        lv_meter_set_indicator_end_value(meter, indic_sec, timeinfo.tm_sec);
        lv_meter_set_indicator_end_value(meter, indic_min, timeinfo.tm_min);
        if (timeinfo.tm_hour > 12) {
            timeinfo.tm_hour -= 12;
        }
        if (timeinfo.tm_hour + 9 > 12) { // for scale rotate 300 degree
            lv_meter_set_indicator_end_value(meter, indic_hour, timeinfo.tm_hour + 9 - 12);
        } else {
            lv_meter_set_indicator_end_value(meter, indic_hour, timeinfo.tm_hour + 9);
        }
    }
}

static void clock_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);

    if (LV_EVENT_FOCUSED == code) {
        lv_group_set_editing(lv_group_get_default(), true);
    } else if (LV_EVENT_KEY == code) {
        uint32_t key = lv_event_get_key(e);

    } else if (LV_EVENT_LONG_PRESSED == code) {
        lv_indev_wait_release(lv_indev_get_next(NULL));
        ui_remove_all_objs_from_encoder_group();
        lv_func_goto_layer(&main_Layer);
    }
}

void ui_clock_init(lv_obj_t *parent)
{

    page = lv_obj_create(parent);
    lv_obj_set_size(page, LV_HOR_RES, LV_VER_RES);
    //lv_obj_set_size(page, lv_obj_get_width(lv_obj_get_parent(page)), lv_obj_get_height(lv_obj_get_parent(page)));

    lv_obj_set_style_border_width(page, 0, 0);
    lv_obj_set_style_radius(page, 0, 0);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_center(page);

    meter = lv_meter_create(page);
    lv_obj_set_size(meter, 220, 220);
    lv_obj_center(meter);
    lv_obj_set_style_border_color(meter, lv_color_make(50, 50, 50), 0);
    lv_obj_set_style_bg_color(meter, lv_color_make(210, 150, 150), 0);
    lv_obj_remove_style(meter, NULL, LV_PART_TICKS);
    lv_obj_remove_style(meter, NULL, LV_PART_ITEMS);
    /*Create a scale for the minutes*/
    /*61 ticks in a 360 degrees range (the last and the first line overlaps)*/
    lv_meter_scale_t *scale_min = lv_meter_add_scale(meter);
    lv_meter_set_scale_ticks(meter, scale_min, 61, 1, 10, lv_color_make(200, 200, 200));
    lv_meter_set_scale_range(meter, scale_min, 0, 60, 360, 180);
    /*Create another scale for the hours. It's only visual and contains only major ticks*/
    lv_meter_scale_t *scale_hour = lv_meter_add_scale(meter);
    lv_meter_set_scale_ticks(meter, scale_hour, 12, 0, 0, lv_color_white()); /*12 ticks*/
    lv_meter_set_scale_major_ticks(meter, scale_hour, 1, 2, 20, lv_color_white(), 10); /*Every tick is major*/
    lv_meter_set_scale_range(meter, scale_hour, 1, 12, 330, 300); /*[1..12] values in an almost full circle*/
    LV_IMG_DECLARE(img_needle_hour);
    LV_IMG_DECLARE(img_needle_min);
    LV_IMG_DECLARE(img_needle_sec);
    /*Add a the hands from images*/
    indic_hour = lv_meter_add_needle_img(meter, scale_hour, &img_needle_hour, 5, 15);
    indic_min = lv_meter_add_needle_img(meter, scale_min, &img_needle_min, 4, 15);
    indic_sec = lv_meter_add_needle_img(meter, scale_min, &img_needle_sec, 5, 15); // second needle on the top

    lv_obj_add_event_cb(page, clock_event_cb, LV_EVENT_FOCUSED, NULL);
    lv_obj_add_event_cb(page, clock_event_cb, LV_EVENT_LONG_PRESSED, NULL);
    ui_add_obj_to_encoder_group(page);
}

static bool clock_layer_enter_cb(struct lv_layer_t *layer)
{
    bool ret = false;

    if (NULL == layer->lv_obj_layer) {
        ret = true;
        layer->lv_obj_layer = lv_obj_create(lv_scr_act());
        lv_obj_remove_style_all(layer->lv_obj_layer);
        lv_obj_set_size(layer->lv_obj_layer, LV_HOR_RES, LV_VER_RES);

        ui_clock_init(layer->lv_obj_layer);
        set_time_out(&time_20ms, 20);
    }

    return ret;
}

static bool clock_layer_exit_cb(struct lv_layer_t *layer)
{
    LV_LOG_USER("");
    return true;
}

static void clock_layer_timer_cb(lv_timer_t *tmr)
{
    if (is_time_out(&time_20ms)) {
        clock_handler();
    }
}
