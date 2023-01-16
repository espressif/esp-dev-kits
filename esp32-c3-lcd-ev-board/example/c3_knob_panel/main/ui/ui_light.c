/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "lvgl.h"
#include <stdio.h>
#include "ui.h"
#include "ui_light.h"

#include "lv_example_pub.h"
#include "lv_example_func.h"
#include "lv_example_image.h"

static const char *TAG = "ui light";

LV_IMG_DECLARE(light_brightness);

static lv_obj_t *arc;
static lv_obj_t *img, *label_brightness;
static lv_obj_t *page;
static time_out_count time_20ms;

static bool light_layer_enter_cb(struct lv_layer_t *layer);
static bool light_layer_exit_cb(struct lv_layer_t *layer);
static void light_layer_timer_cb(lv_timer_t *tmr);

lv_layer_t light_Layer = {
    .lv_obj_name    = "light_Layer",
    .lv_obj_parent  = NULL,
    .lv_obj_layer   = NULL,
    .lv_show_layer  = NULL,
    .enter_cb       = light_layer_enter_cb,
    .exit_cb        = light_layer_exit_cb,
    .timer_cb       = light_layer_timer_cb,
};

static void brightness_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    lv_obj_t *label = lv_event_get_user_data(e);

    if (code == LV_EVENT_VALUE_CHANGED) {
        lv_label_set_text_fmt(label, "%d", lv_arc_get_value(obj));
        lv_obj_set_style_img_opa(img, lv_arc_get_value(obj) * 255 / 100, 0);
        printf("%s: lv_arc_get_value:%d\n", TAG, lv_arc_get_value(obj));
    }
}

static void light_event_cb(lv_event_t *e)
{
    uint8_t current;

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);

    if (LV_EVENT_FOCUSED == code) {
        lv_group_set_editing(lv_group_get_default(), true);
    } else if (LV_EVENT_KEY == code) {
        uint32_t key = lv_event_get_key(e);
        printf("evt=%s\n", (LV_KEY_RIGHT == key) ? "LV_KEY_RIGHT" : "LV_KEY_LEFT");

        if (LV_KEY_RIGHT == key) {
            current = lv_arc_get_value(arc);
            if (current < 100) {
                current++;
            }
        } else {
            current = lv_arc_get_value(arc);
            if (current) {
                current--;
            }
        }
        lv_arc_set_value(arc, current);
        lv_label_set_text_fmt(label_brightness, "%d", current);
        lv_obj_set_style_img_opa(img, current * 255 / 100, 0);
        printf("arc value:[%d]\r\n", lv_arc_get_value(arc));

    } else if (LV_EVENT_LONG_PRESSED == code) {
        printf("evt=%s\n", "LV_EVENT_LONG_PRESSED");
        lv_indev_wait_release(lv_indev_get_next(NULL));
        ui_remove_all_objs_from_encoder_group();
        lv_func_goto_layer(&main_Layer);
    }
}

#if 0
void ui_light_init(lv_obj_t *parent)
{
    page = lv_obj_create(parent);
    lv_obj_set_size(page, LV_HOR_RES, LV_VER_RES);
    //lv_obj_set_size(page, lv_obj_get_width(lv_obj_get_parent(page)), lv_obj_get_height(lv_obj_get_parent(page)));

    lv_obj_set_style_border_width(page, 0, 0);
    lv_obj_set_style_radius(page, 0, 0);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_center(page);
    LV_LOG_USER("[%d,%d]", lv_obj_get_width(lv_obj_get_parent(page)), lv_obj_get_height(lv_obj_get_parent(page)));

    /*Create a Tab view object*/
    lv_obj_t *tabview;
    tabview = lv_tabview_create(page, LV_DIR_TOP, 0);
    //lv_obj_set_size(tabview, lv_obj_get_width(lv_obj_get_parent(page)), lv_obj_get_height(lv_obj_get_parent(page)));
    lv_obj_set_size(page, LV_HOR_RES, LV_VER_RES);
    lv_obj_center(tabview);

    // lv_obj_add_event_cb(lv_tabview_get_content(tabview), scroll_begin_event, LV_EVENT_SCROLL_BEGIN, NULL);

    // lv_obj_set_style_bg_color(tabview, lv_palette_darken(LV_PALETTE_RED, 1), 0);

    // lv_obj_t *tab_btns = lv_tabview_get_tab_btns(tabview);
    // lv_obj_set_style_bg_color(tab_btns, lv_palette_darken(LV_PALETTE_GREY, 3), 0);
    // lv_obj_set_style_text_color(tab_btns, lv_palette_lighten(LV_PALETTE_GREY, 5), 0);
    // lv_obj_set_style_border_side(tab_btns, LV_BORDER_SIDE_RIGHT, LV_PART_ITEMS | LV_STATE_CHECKED);

    /*Add 3 tabs (the tabs are page (lv_page) and can be scrolled*/
    lv_obj_t *tab1 = lv_tabview_add_tab(tabview, "Tab 1");
    lv_obj_t *tab2 = lv_tabview_add_tab(tabview, "Tab 2");
    lv_obj_t *tab3 = lv_tabview_add_tab(tabview, "Tab 3");
    lv_obj_clear_flag(tab1, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(tab2, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(tab3, LV_OBJ_FLAG_SCROLLABLE);

    /**
     * Tab 1 for brightness
     */
    arc = lv_arc_create(tab1);
    lv_obj_set_size(arc, lv_obj_get_width(tab1) - 15, lv_obj_get_height(tab1) - 15);
    lv_arc_set_rotation(arc, 180);
    lv_arc_set_bg_angles(arc, 0, 180);
    // lv_arc_set_angles(arc, 0, 30);
    lv_arc_set_value(arc, 30);
    // lv_arc_set_range(arc, 0, 100);
    lv_obj_set_style_arc_width(arc, 20, LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc, 20, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(arc, lv_color_make(60, 60, 60), LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc, lv_color_make(200, 150, 20), LV_PART_INDICATOR);
    lv_obj_set_style_outline_width(arc, 2, LV_STATE_FOCUSED | LV_PART_KNOB);
    lv_obj_set_style_outline_color(arc, lv_palette_main(LV_PALETTE_LIGHT_BLUE), LV_STATE_FOCUSED | LV_PART_KNOB);
    lv_obj_set_style_outline_color(arc, lv_palette_main(LV_PALETTE_YELLOW), LV_STATE_EDITED | LV_PART_KNOB);

    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);   /*Be sure the knob is not displayed*/
    // lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);  /*To not allow adjusting by click*/
    lv_obj_center(arc);

    lv_obj_t *label1 = lv_label_create(tab1);
    lv_obj_set_style_text_font(label1, &font_cn_32, 0);
    lv_label_set_text(label1, "调光");
    lv_obj_set_width(label1, 150);  /*Set smaller width to make the lines wrap*/
    lv_obj_set_style_text_align(label1, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label1, LV_ALIGN_CENTER, 0, -60);

    lv_obj_t *label2 = lv_label_create(tab1);
    lv_label_set_text(label2, "%");
    lv_obj_set_style_text_font(label2, &lv_font_montserrat_20, 0);
    lv_obj_set_width(label2, 50);
    lv_obj_set_style_text_align(label2, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label2, LV_ALIGN_CENTER, 40, -10);

    lv_obj_t *label_brightness = lv_label_create(tab1);
    lv_label_set_text_fmt(label_brightness, "%d", lv_arc_get_value(arc));
    LV_LOG_USER("lv_arc_get_value:[%d]", lv_arc_get_value(arc));

    lv_obj_set_style_text_font(label_brightness, &lv_font_montserrat_40, 0);
    lv_obj_set_width(label_brightness, 150);  /*Set smaller width to make the lines wrap*/
    lv_obj_set_style_text_align(label_brightness, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label_brightness, LV_ALIGN_CENTER, 0, 0);

    img = lv_img_create(tab1);
    lv_img_set_src(img, &light_brightness);
    lv_obj_align_to(img, label_brightness, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_set_style_bg_color(img, lv_color_make(200, 0, 0), 0);
    lv_obj_set_style_img_opa(img, lv_arc_get_value(arc) * 255 / 100, 0);

    lv_obj_add_event_cb(arc, brightness_event_cb, LV_EVENT_VALUE_CHANGED, label_brightness);

    lv_anim_t a1;
    lv_anim_init(&a1);
    lv_anim_set_var(&a1, arc);
    lv_anim_set_values(&a1, lv_obj_get_y_aligned(arc) - 20, lv_obj_get_y_aligned(arc));
    lv_anim_set_exec_cb(&a1, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_path_cb(&a1, lv_anim_path_overshoot);
    //lv_anim_set_time(&a1, 400);
    lv_anim_set_time(&a1, 400 * 3);
    lv_anim_start(&a1);
    lv_anim_t a2;
    lv_anim_init(&a2);
    lv_anim_set_var(&a2, img);
    lv_anim_set_values(&a2, lv_obj_get_y_aligned(img) + 20, lv_obj_get_y_aligned(img));
    lv_anim_set_exec_cb(&a2, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_path_cb(&a2, lv_anim_path_overshoot);
    //lv_anim_set_time(&a2, 400);
    lv_anim_set_time(&a1, 400 * 3);
    lv_anim_start(&a2);

    /**
     * Tab 2 for colorwheel
     */
    lv_obj_t *cw;
    cw = lv_colorwheel_create(tab2, true);
    lv_obj_set_size(cw, 180, 180);
    lv_obj_center(cw);

    /**
     * Tab 3 for colorwheel
     */
    lv_obj_t *label = lv_label_create(tab3);
    lv_label_set_text(label, "First tab");


    //ui_add_obj_to_encoder_group(arc);
    //ui_add_obj_to_encoder_group(cw);
    ui_add_obj_to_encoder_group(page);
    lv_group_focus_obj(page);
    // ui_add_obj_to_encoder_group(label);
    //lv_obj_add_event_cb(page, light_event_cb, LV_EVENT_FOCUSED, NULL);
    lv_obj_add_event_cb(page, light_event_cb, LV_EVENT_KEY, NULL);
    lv_obj_add_event_cb(page, light_event_cb, LV_EVENT_LONG_PRESSED, NULL);
    // lv_obj_add_event_cb(tabview, light_event_cb, LV_EVENT_CLICKED, NULL);
    //lv_obj_add_event_cb(arc, light_event_cb, LV_EVENT_LONG_PRESSED, NULL);
    //lv_obj_add_event_cb(cw, light_event_cb, LV_EVENT_LONG_PRESSED, NULL);
    printf("arc:%x, cw:%x, tabview:=%x, page:%x", arc, cw, tabview, page);
}
#else
void ui_light_init(lv_obj_t *parent)
{
    page = lv_obj_create(parent);
    lv_obj_set_size(page, LV_HOR_RES, LV_VER_RES);
    //lv_obj_set_size(page, lv_obj_get_width(lv_obj_get_parent(page)), lv_obj_get_height(lv_obj_get_parent(page)));

    lv_obj_set_style_border_width(page, 0, 0);
    lv_obj_set_style_radius(page, 0, 0);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_center(page);
    LV_LOG_USER("[%d,%d]", lv_obj_get_width(lv_obj_get_parent(page)), lv_obj_get_height(lv_obj_get_parent(page)));

    arc = lv_arc_create(page);
    //lv_obj_set_size(arc, lv_obj_get_width(page) - 15, lv_obj_get_height(page) - 15);
    lv_obj_set_size(arc, LV_HOR_RES - 15, LV_VER_RES - 15);
    lv_arc_set_rotation(arc, 180);
    lv_arc_set_bg_angles(arc, 0, 180);
    // lv_arc_set_angles(arc, 0, 30);
    lv_arc_set_value(arc, 30);
    // lv_arc_set_range(arc, 0, 100);
    lv_obj_set_style_arc_width(arc, 20, LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc, 20, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(arc, lv_color_make(60, 60, 60), LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc, lv_color_make(200, 150, 20), LV_PART_INDICATOR);
    lv_obj_set_style_outline_width(arc, 2, LV_STATE_FOCUSED | LV_PART_KNOB);
    lv_obj_set_style_outline_color(arc, lv_palette_main(LV_PALETTE_LIGHT_BLUE), LV_STATE_FOCUSED | LV_PART_KNOB);
    lv_obj_set_style_outline_color(arc, lv_palette_main(LV_PALETTE_YELLOW), LV_STATE_EDITED | LV_PART_KNOB);

    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);   /*Be sure the knob is not displayed*/
    //lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);  /*To not allow adjusting by click*/
    lv_obj_center(arc);

    lv_obj_t *label1 = lv_label_create(page);
    lv_obj_set_style_text_font(label1, &font_cn_32, 0);
    lv_label_set_text(label1, "调光");
    lv_obj_set_width(label1, 150);  /*Set smaller width to make the lines wrap*/
    lv_obj_set_style_text_align(label1, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label1, LV_ALIGN_CENTER, 0, -60);

    lv_obj_t *label2 = lv_label_create(page);
    lv_label_set_text(label2, "%");
    lv_obj_set_style_text_font(label2, &lv_font_montserrat_20, 0);
    lv_obj_set_width(label2, 50);
    lv_obj_set_style_text_align(label2, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label2, LV_ALIGN_CENTER, 40, -10);

    label_brightness = lv_label_create(page);
    lv_label_set_text_fmt(label_brightness, "%d", lv_arc_get_value(arc));
    LV_LOG_USER("arc value:[%d]", lv_arc_get_value(arc));

    lv_obj_set_style_text_font(label_brightness, &lv_font_montserrat_40, 0);
    lv_obj_set_width(label_brightness, 150);  /*Set smaller width to make the lines wrap*/
    lv_obj_set_style_text_align(label_brightness, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label_brightness, LV_ALIGN_CENTER, 0, 0);

    img = lv_img_create(page);
    lv_img_set_src(img, &light_brightness);
    lv_obj_align_to(img, label_brightness, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_set_style_bg_color(img, lv_color_make(200, 0, 0), 0);
    lv_obj_set_style_img_opa(img, lv_arc_get_value(arc) * 255 / 100, 0);

    //lv_obj_add_event_cb(arc, brightness_event_cb, LV_EVENT_VALUE_CHANGED, label_brightness);

    lv_anim_t a1;
    lv_anim_init(&a1);
    lv_anim_set_var(&a1, arc);
    lv_anim_set_values(&a1, lv_obj_get_y_aligned(arc) - 20, lv_obj_get_y_aligned(arc));
    lv_anim_set_exec_cb(&a1, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_path_cb(&a1, lv_anim_path_overshoot);
    //lv_anim_set_time(&a1, 400);
    lv_anim_set_time(&a1, 400 * 3);
    lv_anim_start(&a1);
    lv_anim_t a2;
    lv_anim_init(&a2);
    lv_anim_set_var(&a2, img);
    lv_anim_set_values(&a2, lv_obj_get_y_aligned(img) + 20, lv_obj_get_y_aligned(img));
    lv_anim_set_exec_cb(&a2, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_path_cb(&a2, lv_anim_path_overshoot);
    //lv_anim_set_time(&a2, 400);
    lv_anim_set_time(&a1, 400 * 3);
    lv_anim_start(&a2);

    lv_obj_add_event_cb(page, light_event_cb, LV_EVENT_FOCUSED, NULL);
    lv_obj_add_event_cb(page, light_event_cb, LV_EVENT_KEY, NULL);
    lv_obj_add_event_cb(page, light_event_cb, LV_EVENT_LONG_PRESSED, NULL);
    ui_add_obj_to_encoder_group(page);
}
#endif

void ui_light_set_brightness(uint8_t value)
{
    lv_arc_set_value(arc, value);
}

static bool light_layer_enter_cb(struct lv_layer_t *layer)
{
    bool ret = false;

    LV_LOG_USER("%x", layer->lv_obj_layer);
    if (NULL == layer->lv_obj_layer) {
        ret = true;
        layer->lv_obj_layer = lv_obj_create(lv_scr_act());
        lv_obj_remove_style_all(layer->lv_obj_layer);
        lv_obj_set_size(layer->lv_obj_layer, LV_HOR_RES, LV_VER_RES);

        ui_light_init(layer->lv_obj_layer);
        set_time_out(&time_20ms, 20);
    }

    return ret;
}

static bool light_layer_exit_cb(struct lv_layer_t *layer)
{
    LV_LOG_USER("");
    return true;
}

static void light_layer_timer_cb(lv_timer_t *tmr)
{
    if (is_time_out(&time_20ms)) {
        //light_handler();
    }
}
