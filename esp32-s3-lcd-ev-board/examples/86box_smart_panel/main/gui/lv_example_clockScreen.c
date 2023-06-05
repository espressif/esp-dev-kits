/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include "bsp/esp-bsp.h"
#include <time.h>
#include <sys/time.h>
#include "settings.h"

#include "lv_example_pub.h"
#include "lv_example_image.h"

static const char *TAG = "MENU_CLOCK";

#define TRUE_TIME       1

static bool clock_screen_layer_enter_cb(void *create_layer);
static bool clock_screen_layer_exit_cb(void *create_layer);
static void clock_screen_layer_timer_cb(lv_timer_t *tmr);

lv_layer_t clock_screen_layer = {
    .lv_obj_name    = "clock_screen_layer",
    .lv_obj_layer   = NULL,
    .enter_cb       = clock_screen_layer_enter_cb,
    .exit_cb        = clock_screen_layer_exit_cb,
    .timer_cb       = clock_screen_layer_timer_cb,
};

static lv_obj_t *central;
static lv_obj_t *lvMinute, * lvHour, * lvSecond;
static lv_obj_t *label_set_time;
static lv_obj_t *btn_clock_exit;

static time_out_count time_500ms;

static void btn_exit_clock_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_SHORT_CLICKED) {
        lv_obj_clear_flag(central, LV_OBJ_FLAG_HIDDEN);
        lv_func_goto_layer(&main_Layer);
    }
}

static void update_time(void *arg)
{
    char textBuf[50];

    static time_t unix_time;
    static struct tm *time_info;

#if TRUE_TIME
    unix_time = time(NULL);
    time_info = localtime(&unix_time);

    int year = time_info->tm_year + 1900;
    int month = time_info->tm_mon + 1;
    int day = time_info->tm_mday;
    // int weekday = time_info->tm_wday;
    int clock_hour = time_info->tm_hour;
    int clock_minutes = time_info->tm_min;
    int clock_second = time_info->tm_sec;
#else
    static uint8_t clock_minutes = 59;
    static uint8_t clock_hour = 8;
    static uint8_t clock_second = 2 ;

    clock_second += 5;
    if (clock_second >= 60) {
        //if (++clock_second >= 60){
        clock_second = 0;
        clock_minutes += 5;
        if (clock_minutes >= 60) {
            //if (++clock_minutes >= 60){
            clock_minutes = 0;
            if (++clock_hour >= 12) {
                clock_hour = 0;
            }
        }
    }
#endif
    if (lvHour != NULL) {
        uint16_t h = (clock_hour >= 12 ? (clock_hour - 12) : (clock_hour)) * 300 + clock_minutes / 12 % 12 * 60;
        lv_img_set_angle(lvHour, h);
        lv_img_set_angle(lvMinute, clock_minutes * 6 * 10);
        lv_img_set_angle(lvSecond, clock_second * 6 * 10);
    }

    if (label_set_time != NULL) {
        sprintf(textBuf, "%04d-%02d-%02d %02d:%02d:%02d", \
                year, month, day, \
                clock_hour, clock_minutes, clock_second);
        lv_label_set_text_fmt(label_set_time, textBuf);
    }
}

static void screen_create_clock(lv_obj_t *parent)
{
    lv_obj_t *img = lv_img_create(parent);
    lv_img_set_src(img, &new_watch_bg);

    lv_obj_set_size(img, 240, 240);
    //lv_obj_set_auto_realign(img, true);
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);

    lvHour = lv_img_create(parent);
    lv_img_set_src(lvHour, &watch_hour);
    lv_img_set_pivot(lvHour, 5, 62);//66
    lv_obj_align(lvHour, LV_ALIGN_CENTER, 0, -29);

    lvMinute = lv_img_create(parent);
    lv_img_set_src(lvMinute, &watch_min);
    lv_img_set_pivot(lvMinute, 5, 84);//90
    lv_obj_align(lvMinute, LV_ALIGN_CENTER, 0, -40);

    lvSecond = lv_img_create(parent);
    lv_img_set_src(lvSecond, &watch_sec);
    lv_img_set_pivot(lvSecond, 5, 96);//104
    lv_obj_align(lvSecond, LV_ALIGN_CENTER, 0, -44);

    label_set_time = lv_label_create(parent);
    //lv_obj_remove_style_all(label_set_time);
    lv_obj_set_style_text_color(label_set_time, lv_color_hex(COLOUR_GREY_8F), 0);
    lv_obj_align(label_set_time, LV_ALIGN_BOTTOM_MID, 0, 0);

}

static bool clock_screen_layer_enter_cb(void *create_layer)
{
    bool ret = false;

    lv_layer_t *layer = create_layer;
    if (NULL == layer->lv_obj_layer) {
        ret = true;

        layer->lv_obj_layer = lv_obj_create(lv_scr_act());
        lv_obj_remove_style_all(layer->lv_obj_layer);
        lv_obj_set_size(layer->lv_obj_layer, LV_HOR_RES, LV_VER_RES);

        lv_obj_t *btn_background = lv_btn_create(layer->lv_obj_layer);
        lv_obj_set_size(btn_background, LV_HOR_RES, LV_VER_RES);
        lv_obj_set_style_radius(btn_background, 0, 0);
        lv_obj_set_style_bg_color(btn_background, lv_color_hex(COLOUR_BLACK), 0);

        central = lv_obj_create(layer->lv_obj_layer);
        lv_obj_set_pos(central, 0, 0);
        lv_obj_set_style_border_width(central, 0, 0);
        lv_obj_set_size(central, LV_HOR_RES, LV_VER_RES);
        lv_obj_set_style_radius(central, 0, 0);
        lv_obj_set_style_bg_color(central, lv_color_hex(COLOUR_GREY_1F), 0);
        screen_create_clock(central);

        btn_clock_exit = lv_btn_create(layer->lv_obj_layer);
        lv_obj_set_size(btn_clock_exit, LV_HOR_RES, LV_VER_RES);
        lv_obj_set_style_opa(btn_clock_exit, LV_OPA_0, 0);
        lv_obj_add_event_cb(btn_clock_exit, btn_exit_clock_event_cb, LV_EVENT_ALL, btn_clock_exit);

        update_time(NULL);
        set_time_out(&time_500ms, 500);
    }
    return ret;
}

static bool clock_screen_layer_exit_cb(void *create_layer)
{
    LV_LOG_USER("exit");
    return true;
}

static void clock_screen_layer_timer_cb(lv_timer_t *tmr)
{
    lv_event_info_t lvgl_event;

    feed_clock_time();
    if (is_time_out(&time_500ms)) {
        update_time(NULL);
    }

    if (pdPASS == app_lvgl_get_event(&lvgl_event, 0)) {
        switch (lvgl_event.event) {
        case LV_EVENT_EXIT_CLOCK:
            ESP_LOGI(TAG, "exit clock:%d", lvgl_event.event);
            lv_event_send(btn_clock_exit, LV_EVENT_SHORT_CLICKED, btn_clock_exit);
            break;
        default:
            break;
        }
    }
}
