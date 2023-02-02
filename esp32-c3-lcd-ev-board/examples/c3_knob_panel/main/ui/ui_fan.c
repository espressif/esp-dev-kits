/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <time.h>
#include "lvgl.h"
#include "ui.h"
#include "ui_fan.h"

#include "lv_example_pub.h"
#include "lv_example_func.h"
#include "lv_example_image.h"

static lv_obj_t  *page;
static time_out_count time_20ms;
static const char *TAG = "Fan";

static bool fan_layer_enter_cb(struct lv_layer_t *layer);
static bool fan_layer_exit_cb(struct lv_layer_t *layer);
static void fan_layer_timer_cb(lv_timer_t *tmr);

lv_layer_t fan_Layer = {
    .lv_obj_name    = "fan_Layer",
    .lv_obj_parent  = NULL,
    .lv_obj_layer   = NULL,
    .lv_show_layer  = NULL,
    .enter_cb       = fan_layer_enter_cb,
    .exit_cb        = fan_layer_exit_cb,
    .timer_cb       = fan_layer_timer_cb,
};

static void fan_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);

    if (LV_EVENT_FOCUSED == code) {
        lv_group_set_editing(lv_group_get_default(), true);
    } else if (LV_EVENT_KEY == code) {
        uint32_t key = lv_event_get_key(e);
        printf("%s, key=%d\n", TAG, key);

    } else if (LV_EVENT_LONG_PRESSED == code) {
        lv_indev_wait_release(lv_indev_get_next(NULL));
        ui_remove_all_objs_from_encoder_group();
        lv_func_goto_layer(&main_Layer);
    }
}

void ui_fan_init(lv_obj_t *parent)
{
    page = lv_obj_create(parent);
    lv_obj_set_size(page, LV_HOR_RES, LV_VER_RES);
    //lv_obj_set_size(page, lv_obj_get_width(lv_obj_get_parent(page)), lv_obj_get_height(lv_obj_get_parent(page)));

    lv_obj_set_style_border_width(page, 0, 0);
    lv_obj_set_style_radius(page, 0, 0);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_center(page);

    lv_obj_t *arc = lv_arc_create(page);
    lv_obj_set_size(arc, lv_obj_get_width(page) - 15, lv_obj_get_height(page) - 15);
    lv_arc_set_rotation(arc, 180);
    lv_arc_set_bg_angles(arc, 0, 180);
    // lv_arc_set_angles(arc, 0, 30);
    lv_arc_set_value(arc, 30);
    // lv_arc_set_range(arc, 0, 100);
    lv_obj_set_style_arc_width(arc, 20, LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc, 20, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(arc, lv_color_make(60, 60, 60), LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc, lv_color_make(20, 70, 200), LV_PART_INDICATOR);
    lv_obj_set_style_outline_width(arc, 2, LV_STATE_FOCUSED | LV_PART_KNOB);
    lv_obj_set_style_outline_color(arc, lv_palette_main(LV_PALETTE_LIGHT_BLUE), LV_STATE_FOCUSED | LV_PART_KNOB);
    lv_obj_set_style_outline_color(arc, lv_palette_main(LV_PALETTE_YELLOW), LV_STATE_EDITED | LV_PART_KNOB);
    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);   /*Be sure the knob is not displayed*/
    // lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);  /*To not allow adjusting by click*/
    lv_obj_center(arc);

    lv_obj_t *label1 = lv_label_create(page);
    lv_obj_set_style_text_font(label1, &font_cn_32, 0);
    lv_label_set_text(label1, "Fan");
    lv_obj_set_width(label1, 150);  /*Set smaller width to make the lines wrap*/
    lv_obj_set_style_text_align(label1, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label1, LV_ALIGN_CENTER, 0, -60);

    lv_obj_t *label2 = lv_label_create(page);
    lv_label_set_text(label2, "%");
    lv_obj_set_style_text_font(label2, &lv_font_montserrat_20, 0);
    lv_obj_set_width(label2, 50);
    lv_obj_set_style_text_align(label2, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label2, LV_ALIGN_CENTER, 40, -10);

    lv_obj_t *label3 = lv_label_create(page);
    lv_label_set_text_fmt(label3, "%d", lv_arc_get_value(arc));
    lv_obj_set_style_text_font(label3, &lv_font_montserrat_40, 0);
    lv_obj_set_width(label3, 150);  /*Set smaller width to make the lines wrap*/
    lv_obj_set_style_text_align(label3, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label3, LV_ALIGN_CENTER, 0, 0);

    lv_obj_add_event_cb(page, fan_event_cb, LV_EVENT_FOCUSED, NULL);
    lv_obj_add_event_cb(page, fan_event_cb, LV_EVENT_LONG_PRESSED, NULL);
    ui_add_obj_to_encoder_group(page);
}

static bool fan_layer_enter_cb(struct lv_layer_t *layer)
{
    bool ret = false;

    if (NULL == layer->lv_obj_layer) {
        ret = true;
        layer->lv_obj_layer = lv_obj_create(lv_scr_act());
        lv_obj_remove_style_all(layer->lv_obj_layer);
        lv_obj_set_size(layer->lv_obj_layer, LV_HOR_RES, LV_VER_RES);

        ui_fan_init(layer->lv_obj_layer);
        set_time_out(&time_20ms, 20);
    }

    return ret;
}

static bool fan_layer_exit_cb(struct lv_layer_t *layer)
{
    LV_LOG_USER("");
    return true;
}

static void fan_layer_timer_cb(lv_timer_t *tmr)
{
    if (is_time_out(&time_20ms)) {
        //fan_handler();
    }
}