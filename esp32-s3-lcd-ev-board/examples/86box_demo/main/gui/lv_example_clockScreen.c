/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "bsp/esp-bsp.h"
#include "lv_example_pub.h"
#include "lv_example_image.h"

static bool clock_screen_layer_enter_cb(void *layer);
static bool clock_screen_layer_exit_cb(void *layer);
static void clock_screen_layer_timer_cb(lv_timer_t *tmr);

lv_layer_t clock_screen_layer = {
    .lv_obj_name    = "clock_screen_layer",
    .lv_obj_layer   = NULL,
    .enter_cb       = clock_screen_layer_enter_cb,
    .exit_cb        = clock_screen_layer_exit_cb,
    .timer_cb       = clock_screen_layer_timer_cb,
};

static lv_obj_t *label_clock_show;

static time_out_count time_10Sec_move, time_500ms;

static void btn_exit_clock_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_SHORT_CLICKED) {
        lv_func_goto_layer(&main_Layer);
    }
}

static void btn_press_clock_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_SHORT_CLICKED) {
        reload_time_out(&time_10Sec_move);
    }
    if (code == LV_EVENT_PRESSED) {
        lv_obj_set_style_text_color(label_clock_show, lv_color_hex(COLOUR_GREY), 0);
    }
}

static bool clock_screen_layer_enter_cb(void *layer)
{
    bool ret = false;
    char timeBuf[20];
    lv_layer_t *create_layer = layer;

    if (NULL == create_layer->lv_obj_layer) {
        ret = true;
        create_layer->lv_obj_layer = lv_obj_create(lv_scr_act());
        lv_obj_remove_style_all(create_layer->lv_obj_layer);
        lv_obj_set_size(create_layer->lv_obj_layer, BSP_LCD_H_RES, BSP_LCD_V_RES);

        lv_obj_t *btn_clock_exit = lv_btn_create(create_layer->lv_obj_layer);
        lv_obj_set_size(btn_clock_exit, BSP_LCD_H_RES, BSP_LCD_V_RES);
        lv_obj_set_style_radius(btn_clock_exit, 0, 0);
        lv_obj_set_style_bg_color(btn_clock_exit, lv_color_hex(0x0000), 0);//lv_color_hex(0x1F1F1F)
        lv_obj_add_event_cb(btn_clock_exit, btn_exit_clock_event_cb, LV_EVENT_ALL, btn_clock_exit);

        label_clock_show = lv_label_create(create_layer->lv_obj_layer);
        lv_obj_remove_style_all(label_clock_show);
        lv_obj_set_style_text_color(label_clock_show, lv_color_white(), 0);
        lv_obj_set_style_text_font(label_clock_show, &font_num50, 0);

        int temp = sys_clock_getSecond();
        sprintf(timeBuf, "%02d:%02d", (temp / 60) % 24, temp % 60);
        lv_label_set_text(label_clock_show, timeBuf);
        lv_obj_align(label_clock_show, LV_ALIGN_CENTER, 0, 0);

        lv_obj_t *btn_clock_run = lv_btn_create(label_clock_show);
        lv_obj_set_size(btn_clock_run, 140, 80);
        lv_obj_set_style_radius(btn_clock_run, 0, 0);
        lv_obj_set_style_opa(btn_clock_run, LV_OPA_TRANSP, 0);
        //lv_obj_set_style_bg_color(btn_clock_run, lv_color_hex(0x001F1F), 0);//lv_color_hex(0x1F1F1F)
        lv_obj_add_event_cb(btn_clock_run, btn_press_clock_event_cb, LV_EVENT_ALL, btn_clock_run);
    }

    set_time_out(&time_500ms, 500);
    set_time_out(&time_10Sec_move, 10 * 1000);

    return ret;
}

static bool clock_screen_layer_exit_cb(void *layer)
{
    LV_LOG_USER("exit");
    return true;
}

static void clock_screen_layer_timer_cb(lv_timer_t *tmr)
{
    if (is_time_out(&time_10Sec_move)) {
        lv_obj_set_style_text_color(label_clock_show, lv_color_white(), 0);
        lv_obj_align(label_clock_show, LV_ALIGN_TOP_LEFT, lv_rand(0, 350), lv_rand(0, 400));
    }

    if (is_time_out(&time_500ms)) {
        char timeBuf[20];
        int temp = sys_clock_getSecond();
        sprintf(timeBuf, "%02d:%02d", (temp / 60) % 24, temp % 60);
        lv_label_set_text(label_clock_show, timeBuf);
    }
}
