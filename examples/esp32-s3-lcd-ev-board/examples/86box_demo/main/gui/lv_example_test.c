/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "bsp/esp-bsp.h"
#include "lv_example_pub.h"
#include "lv_example_image.h"

static bool test_layer_enter_cb(void *layer);
static bool test_layer_exit_cb(void *layer);
static void test_layer_timer_cb(lv_timer_t *tmr);

lv_layer_t test_layer = {
    .lv_obj_name    = "test_layer",
    .lv_obj_layer   = NULL,
    .enter_cb       = test_layer_enter_cb,
    .exit_cb        = test_layer_exit_cb,
    .timer_cb       = test_layer_timer_cb,
};

static time_out_count time_1000ms;
static lv_obj_t *btn_changed;

static void btn_exit_clock_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_SHORT_CLICKED) {
        lv_func_goto_layer(&main_Layer);
    }
}

static bool test_layer_enter_cb(void *layer)
{
    bool ret = false;
    lv_layer_t *create_layer = layer;

    if (NULL == create_layer->lv_obj_layer) {
        ret = true;
        create_layer->lv_obj_layer = lv_obj_create(lv_scr_act());
        lv_obj_remove_style_all(create_layer->lv_obj_layer);
        lv_obj_set_size(create_layer->lv_obj_layer, BSP_LCD_H_RES, BSP_LCD_V_RES);

        btn_changed = lv_btn_create(create_layer->lv_obj_layer);
        lv_obj_set_size(btn_changed, BSP_LCD_H_RES, BSP_LCD_V_RES);
        lv_obj_set_style_radius(btn_changed, 0, 0);
        lv_obj_set_style_bg_color(btn_changed, lv_color_hex(0x0000), 0);//lv_color_hex(0x1F1F1F)
        lv_obj_add_event_cb(btn_changed, btn_exit_clock_event_cb, LV_EVENT_ALL, btn_changed);
    }
    set_time_out(&time_1000ms, 1000);

    return ret;
}

static bool test_layer_exit_cb(void *layer)
{
    LV_LOG_USER("exit");
    return true;
}

static void test_layer_timer_cb(lv_timer_t *tmr)
{
    static uint8_t list = 0;

    if (is_time_out(&time_1000ms)) {
        list++;
        LV_LOG_USER("new, %d", list % 5);

        switch (list % 5) {
        case 0:
            lv_obj_set_style_bg_color(btn_changed, lv_color_hex(0xFF0000), 0);
            break;
        case 1:
            lv_obj_set_style_bg_color(btn_changed, lv_color_hex(0x00FF00), 0);
            break;
        case 2:
            lv_obj_set_style_bg_color(btn_changed, lv_color_hex(0x0000FF), 0);
            break;
        case 3:
            lv_obj_set_style_bg_color(btn_changed, lv_color_hex(0x000000), 0);
            break;
        case 4:
            lv_obj_set_style_bg_color(btn_changed, lv_color_hex(0xFFFFFF), 0);
            break;
        }
    }
}
