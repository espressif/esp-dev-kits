/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "lvgl.h"
#include <stdio.h>
#include "ui_thermostat.h"

#include "lv_example_pub.h"
#include "lv_example_image.h"

static lv_obj_t *temp_arc;
static lv_obj_t *page;
static lv_obj_t *temp_wheel;
static time_out_count time_500ms;

static bool thermostat_layer_enter_cb(void *layer);
static bool thermostat_layer_exit_cb(void *layer);
static void thermostat_layer_timer_cb(lv_timer_t *tmr);

lv_layer_t thermostat_Layer = {
    .lv_obj_name    = "thermostat_Layer",
    .lv_obj_parent  = NULL,
    .lv_obj_layer   = NULL,
    .lv_show_layer  = NULL,
    .enter_cb       = thermostat_layer_enter_cb,
    .exit_cb        = thermostat_layer_exit_cb,
    .timer_cb       = thermostat_layer_timer_cb,
};

static void thermostat_event_cb(lv_event_t *e)
{
    uint8_t current;

    lv_event_code_t code = lv_event_get_code(e);

    if (LV_EVENT_FOCUSED == code) {
        lv_group_set_editing(lv_group_get_default(), true);
    } else if (LV_EVENT_KEY == code) {

        if (is_time_out(&time_500ms)) {
            uint32_t key = lv_event_get_key(e);
            current = lv_arc_get_value(temp_arc);
            if (LV_KEY_RIGHT == key) {
                if (current < lv_arc_get_max_value(temp_arc)) {
                    current++;
                }
            } else {
                if (current > lv_arc_get_min_value(temp_arc)) {
                    current--;
                }
            }
            lv_arc_set_value(temp_arc, current);
            lv_roller_set_selected(temp_wheel, (current - 19), LV_ANIM_ON);
        }

    } else if (LV_EVENT_LONG_PRESSED == code) {
        lv_indev_wait_release(lv_indev_get_next(NULL));
        ui_remove_all_objs_from_encoder_group();
        lv_func_goto_layer(&menu_layer);
    }
}

static void mask_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);

    static int16_t mask_top_id = -1;
    static int16_t mask_bottom_id = -1;

    if (code == LV_EVENT_COVER_CHECK) {
        lv_event_set_cover_res(e, LV_COVER_RES_MASKED);
    } else if (code == LV_EVENT_VALUE_CHANGED) {
        char buf[32];
        lv_roller_get_selected_str(obj, buf, sizeof(buf));
        LV_LOG_USER("Selected value: %s", buf);
    } else if (code == LV_EVENT_DRAW_MAIN_BEGIN) {
        /* add mask */
        const lv_font_t *font = lv_obj_get_style_text_font(obj, LV_PART_MAIN);
        lv_coord_t line_space = lv_obj_get_style_text_line_space(obj, LV_PART_MAIN);
        lv_coord_t font_h = lv_font_get_line_height(font);

        lv_area_t roller_coords;
        lv_obj_get_coords(obj, &roller_coords);

        lv_area_t rect_area;
        rect_area.x1 = roller_coords.x1;
        rect_area.x2 = roller_coords.x2;
        rect_area.y1 = roller_coords.y1;
        rect_area.y2 = roller_coords.y1 + (lv_obj_get_height(obj) - font_h - line_space) / 2;

        lv_draw_mask_fade_param_t *fade_mask_top = lv_mem_buf_get(sizeof(lv_draw_mask_fade_param_t));
        lv_draw_mask_fade_init(fade_mask_top, &rect_area, LV_OPA_TRANSP, rect_area.y1, LV_OPA_COVER, rect_area.y2);
        mask_top_id = lv_draw_mask_add(fade_mask_top, NULL);

        rect_area.y1 = rect_area.y2 + font_h + line_space - 1;
        rect_area.y2 = roller_coords.y2;

        lv_draw_mask_fade_param_t *fade_mask_bottom = lv_mem_buf_get(sizeof(lv_draw_mask_fade_param_t));
        lv_draw_mask_fade_init(fade_mask_bottom, &rect_area, LV_OPA_COVER, rect_area.y1, LV_OPA_TRANSP, rect_area.y2);
        mask_bottom_id = lv_draw_mask_add(fade_mask_bottom, NULL);

    } else if (code == LV_EVENT_DRAW_POST_END) {
        lv_draw_mask_fade_param_t *fade_mask_top = lv_draw_mask_remove_id(mask_top_id);
        lv_draw_mask_fade_param_t *fade_mask_bottom = lv_draw_mask_remove_id(mask_bottom_id);
        lv_draw_mask_free_param(fade_mask_top);
        lv_draw_mask_free_param(fade_mask_bottom);
        lv_mem_buf_release(fade_mask_top);
        lv_mem_buf_release(fade_mask_bottom);
        mask_top_id = -1;
        mask_bottom_id = -1;
    }
}

/**
 * Add a fade mask to roller.
 */
void lv_create_obj_roller(lv_obj_t *parent)
{
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_bg_color(&style, lv_color_black());
    lv_style_set_bg_opa(&style, LV_OPA_0);
    lv_style_set_text_color(&style, lv_color_white());
    lv_style_set_border_width(&style, 0);
    lv_style_set_pad_all(&style, 0);
    lv_obj_add_style(lv_scr_act(), &style, 0);

    temp_wheel = lv_roller_create(parent);
    lv_obj_add_style(temp_wheel, &style, 0);
    printf("line_space:%d\r\n", lv_obj_get_style_text_line_space(temp_wheel, LV_PART_MAIN));
    lv_obj_set_style_text_line_space(temp_wheel, 40, LV_PART_MAIN);
    //lv_obj_set_style_border_width(temp_wheel, 10, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(temp_wheel, LV_OPA_TRANSP, LV_PART_SELECTED);

#if LV_FONT_MONTSERRAT_48
    lv_obj_set_style_text_font(temp_wheel, &lv_font_montserrat_48, LV_PART_SELECTED);
#endif
    lv_roller_set_options(temp_wheel,
                          "19\n"
                          "20\n"
                          "21\n"
                          "22\n"
                          "23\n"
                          "24\n"
                          "25\n"
                          "26\n"
                          "27\n"
                          "28\n"
                          "29\n"
                          "30",
                          LV_ROLLER_MODE_NORMAL);

    lv_obj_align(temp_wheel, LV_ALIGN_CENTER, 5, 10);
    lv_roller_set_visible_row_count(temp_wheel, 3);
    lv_obj_add_event_cb(temp_wheel, mask_event_cb, LV_EVENT_ALL, NULL);
}

void ui_thermostat_init(lv_obj_t *parent)
{
    page = lv_obj_create(parent);
    lv_obj_set_size(page, LV_HOR_RES, LV_VER_RES);

    lv_obj_set_style_border_width(page, 0, 0);
    lv_obj_set_style_radius(page, 0, 0);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_center(page);

    lv_obj_t *img_thermostat_bg = lv_img_create(page);
    lv_img_set_src(img_thermostat_bg, &AC_BG);
    lv_obj_align(img_thermostat_bg, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *img_thermostat_temp = lv_img_create(page);
    lv_img_set_src(img_thermostat_temp, &AC_temper);
    lv_obj_align(img_thermostat_temp, LV_ALIGN_CENTER, 0, 20);

    temp_arc = lv_arc_create(page);
    lv_obj_set_size(temp_arc, LV_HOR_RES - 40, LV_VER_RES - 40);
    lv_arc_set_rotation(temp_arc, 180 + (180 - 150) / 2);
    lv_arc_set_bg_angles(temp_arc, 0, 150);
    lv_arc_set_value(temp_arc, 22);
    lv_arc_set_range(temp_arc, 19, 30);
    lv_obj_set_style_arc_width(temp_arc, 10, LV_PART_MAIN);
    lv_obj_set_style_arc_width(temp_arc, 10, LV_PART_INDICATOR);

    lv_obj_set_style_arc_color(temp_arc, lv_color_make(230, 103, 80), LV_PART_MAIN);
    lv_obj_set_style_arc_color(temp_arc, lv_color_make(230, 103, 80), LV_PART_INDICATOR);

#if 0
    //lv_obj_set_style_arc_color(temp_arc, lv_color_make(255, 255, 255), LV_PART_KNOB);
    lv_obj_set_style_arc_color(temp_arc, lv_color_make(230, 103, 80), LV_PART_KNOB);
    lv_obj_set_style_outline_width(temp_arc, 2, LV_STATE_FOCUSED | LV_PART_KNOB);
    lv_obj_set_style_outline_color(temp_arc, lv_color_make(255, 255, 255), LV_STATE_FOCUSED | LV_PART_KNOB);
    lv_obj_set_style_outline_color(temp_arc, lv_color_make(255, 255, 255), LV_STATE_EDITED | LV_PART_KNOB);
#else
    static lv_style_t style_knob_color;
    lv_style_init(&style_knob_color);
    lv_style_set_bg_color(&style_knob_color, lv_color_make(0xFF, 0xFF, 0xFF));
    lv_obj_add_style(temp_arc, &style_knob_color, LV_PART_KNOB);
#endif

    //lv_obj_remove_style(temp_arc, NULL, LV_PART_KNOB);   /*Be sure the knob is not displayed*/
    //lv_obj_clear_flag(temp_arc, LV_OBJ_FLAG_CLICKABLE);  /*To not allow adjusting by click*/
    lv_obj_align(temp_arc, LV_ALIGN_TOP_MID, 0, 15);

    lv_obj_t *img_temp_unit = lv_img_create(page);
    lv_img_set_src(img_temp_unit, &AC_unit);
    lv_obj_align(img_temp_unit, LV_ALIGN_CENTER, 50, -10);

    lv_create_obj_roller(parent);
    lv_roller_set_selected(temp_wheel, (22 - 19), LV_ANIM_ON);

    lv_anim_t a1;
    lv_anim_init(&a1);
    lv_anim_set_var(&a1, temp_arc);
    lv_anim_set_values(&a1, lv_obj_get_y_aligned(temp_arc) - 20, lv_obj_get_y_aligned(temp_arc));
    lv_anim_set_exec_cb(&a1, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_path_cb(&a1, lv_anim_path_overshoot);
    //lv_anim_set_time(&a1, 400);
    lv_anim_set_time(&a1, 400 * 3);
    lv_anim_start(&a1);

    lv_anim_t a2;
    lv_anim_init(&a2);
    lv_anim_set_var(&a2, img_thermostat_temp);
    lv_anim_set_values(&a2, lv_obj_get_y_aligned(img_thermostat_temp) + 20, lv_obj_get_y_aligned(img_thermostat_temp));
    lv_anim_set_exec_cb(&a2, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_path_cb(&a2, lv_anim_path_overshoot);
    //lv_anim_set_time(&a2, 400);
    lv_anim_set_time(&a1, 400 * 3);
    lv_anim_start(&a2);

    ui_remove_all_objs_from_encoder_group();//roll will add event default.
    lv_obj_add_event_cb(page, thermostat_event_cb, LV_EVENT_FOCUSED, NULL);
    lv_obj_add_event_cb(page, thermostat_event_cb, LV_EVENT_KEY, NULL);
    lv_obj_add_event_cb(page, thermostat_event_cb, LV_EVENT_LONG_PRESSED, NULL);
    ui_add_obj_to_encoder_group(page);
}

static bool thermostat_layer_enter_cb(void *layer)
{
    bool ret = false;

    LV_LOG_USER("");
    lv_layer_t *create_layer = layer;
    if (NULL == create_layer->lv_obj_layer) {
        ret = true;
        create_layer->lv_obj_layer = lv_obj_create(lv_scr_act());
        lv_obj_remove_style_all(create_layer->lv_obj_layer);
        lv_obj_set_size(create_layer->lv_obj_layer, LV_HOR_RES, LV_VER_RES);

        ui_thermostat_init(create_layer->lv_obj_layer);
        set_time_out(&time_500ms, 100);
    }
    return ret;
}

static bool thermostat_layer_exit_cb(void *layer)
{
    LV_LOG_USER("");
    return true;
}

static void thermostat_layer_timer_cb(lv_timer_t *tmr)
{
    feed_clock_time();
}
