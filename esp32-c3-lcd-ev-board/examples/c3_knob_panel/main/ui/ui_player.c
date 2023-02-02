/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "lvgl.h"
#include <stdio.h>
#include "ui.h"
#include "ui_player.h"

#include "lv_example_pub.h"
#include "lv_example_func.h"
#include "lv_example_image.h"

static lv_obj_t *page;

static time_out_count time_20ms;

static bool player_Layer_enter_cb(struct lv_layer_t *layer);
static bool player_Layer_exit_cb(struct lv_layer_t *layer);
static void player_Layer_timer_cb(lv_timer_t *tmr);

lv_layer_t player_Layer = {
    .lv_obj_name    = "player_Layer",
    .lv_obj_parent  = NULL,
    .lv_obj_layer   = NULL,
    .lv_show_layer  = NULL,
    .enter_cb       = player_Layer_enter_cb,
    .exit_cb        = player_Layer_exit_cb,
    .timer_cb       = player_Layer_timer_cb,
};

static void player_event_cb(lv_event_t *e)
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

void ui_player_init(lv_obj_t *parent)
{
    page = lv_obj_create(parent);
    lv_obj_set_size(page, LV_HOR_RES, LV_VER_RES);
    //lv_obj_set_size(page, lv_obj_get_width(lv_obj_get_parent(page)), lv_obj_get_height(lv_obj_get_parent(page)));

    lv_obj_set_style_border_width(page, 0, 0);
    lv_obj_set_style_radius(page, 0, 0);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_center(page);

    static lv_style_t style_btn;
    lv_style_init(&style_btn);
    lv_style_set_bg_color(&style_btn, lv_color_white());
    lv_style_set_bg_opa(&style_btn, LV_OPA_20);
    lv_style_set_border_width(&style_btn, 0);

    LV_IMG_DECLARE(img_player);
    lv_obj_t *img = lv_img_create(page);
    lv_img_set_src(img, &img_player);
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_img_opa(img, LV_OPA_50, 0);
    lv_obj_refr_size(page);

    lv_obj_t *arc_time = lv_arc_create(img);
    lv_obj_set_size(arc_time, lv_obj_get_width(page) - 12, lv_obj_get_height(page) - 12);
    lv_arc_set_rotation(arc_time, 160);
    lv_arc_set_bg_angles(arc_time, 0, 150);
    lv_arc_set_value(arc_time, 30);
    lv_obj_set_style_arc_width(arc_time, 4, LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc_time, 4, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(arc_time, lv_color_make(100, 100, 100), LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc_time, lv_color_make(230, 230, 230), LV_PART_INDICATOR);
    lv_obj_remove_style(arc_time, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(arc_time, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_arc_opa(arc_time, LV_OPA_40, LV_PART_MAIN);
    lv_obj_set_style_arc_opa(arc_time, LV_OPA_60, LV_PART_INDICATOR);
    lv_obj_center(arc_time);

    lv_obj_t *arc_volume = lv_arc_create(img);
    lv_obj_set_size(arc_volume, lv_obj_get_width(page) - 12, lv_obj_get_height(page) - 12);
    lv_arc_set_rotation(arc_volume, 55);
    lv_arc_set_bg_angles(arc_volume, 0, 90);
    lv_arc_set_range(arc_volume, 0, 30);
    lv_arc_set_value(arc_volume, 18);
    lv_obj_set_style_arc_width(arc_volume, 6, LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc_volume, 6, LV_PART_INDICATOR);
    lv_obj_set_style_outline_width(arc_volume, 2, LV_STATE_FOCUSED | LV_PART_KNOB);
    lv_obj_set_style_outline_color(arc_volume, lv_palette_main(LV_PALETTE_LIGHT_BLUE), LV_STATE_FOCUSED | LV_PART_KNOB);
    lv_obj_set_style_outline_color(arc_volume, lv_palette_main(LV_PALETTE_YELLOW), LV_STATE_EDITED | LV_PART_KNOB);
    lv_obj_set_style_arc_color(arc_volume, lv_color_make(100, 100, 100), LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc_volume, lv_color_make(0, 200, 0), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(arc_volume, lv_color_make(0, 200, 0), LV_PART_KNOB);
    lv_obj_set_style_pad_all(arc_volume, 3, LV_PART_KNOB);
    lv_obj_set_style_arc_opa(arc_volume, LV_OPA_40, LV_PART_MAIN);
    lv_obj_set_style_arc_opa(arc_volume, LV_OPA_60, LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(arc_volume, LV_OPA_60, LV_PART_KNOB);
    lv_obj_center(arc_volume);

    lv_obj_t *label_speaker = lv_label_create(img);
    lv_label_set_text(label_speaker, LV_SYMBOL_VOLUME_MAX);
    lv_obj_set_style_text_font(label_speaker, &lv_font_montserrat_12, 0);
    lv_obj_set_width(label_speaker, 30);
    lv_obj_set_style_text_align(label_speaker, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_opa(label_speaker, LV_OPA_COVER, 0);
    lv_obj_set_style_transform_pivot_x(label_speaker, lv_obj_get_x(label_speaker), 0);
    lv_obj_set_style_transform_pivot_y(label_speaker, lv_obj_get_y(label_speaker), 0);
    lv_obj_set_style_transform_angle(label_speaker, /*-350*/0, 0);
    lv_obj_align(label_speaker, LV_ALIGN_CENTER, 78, 82);

    lv_obj_t *label_name = lv_label_create(img);
    lv_label_set_text(label_name, "Erica Smith");
    lv_obj_set_style_text_font(label_name, &lv_font_montserrat_24, 0);
    lv_obj_set_width(label_name, 150);
    lv_obj_set_style_text_align(label_name, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label_name, LV_ALIGN_CENTER, 0, -70);

    lv_obj_t *label_album = lv_label_create(img);
    lv_label_set_text(label_album, "Dance on the moon");
    lv_obj_set_style_text_font(label_album, &lv_font_montserrat_16, 0);
    lv_obj_set_width(label_album, 200);
    lv_obj_set_style_text_align(label_album, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align_to(label_album, label_name, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

    lv_obj_t *label_time = lv_label_create(img);
    lv_label_set_text_fmt(label_time, "%02d:%02d | %02d:%02d", 1, 24, 2, 15);
    lv_obj_set_style_text_font(label_time, &lv_font_montserrat_12, 0);
    lv_obj_set_width(label_time, 150);
    lv_obj_set_style_text_align(label_time, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label_time, LV_ALIGN_CENTER, 0, 40);

    lv_obj_t *btn_mode = lv_btn_create(img);
    lv_obj_set_size(btn_mode, 40, 40);
    lv_obj_set_style_radius(btn_mode, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(btn_mode, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_align_to(btn_mode, label_time, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    lv_obj_t *label_mode = lv_label_create(btn_mode);
    lv_label_set_text(label_mode, LV_SYMBOL_LOOP);
    lv_obj_set_style_text_font(label_mode, &lv_font_montserrat_24, 0);
    lv_obj_set_size(label_mode, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_border_width(label_mode, 2, LV_STATE_FOCUSED);
    lv_obj_set_style_border_color(label_mode, lv_palette_main(LV_PALETTE_LIGHT_BLUE), LV_STATE_FOCUSED);
    lv_obj_set_style_text_align(label_mode, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(label_mode);

    lv_obj_t *btn_play = lv_btn_create(img);
    lv_obj_set_size(btn_play, 60, 60);
    lv_obj_set_style_radius(btn_play, LV_RADIUS_CIRCLE, 0);
    lv_obj_add_style(btn_play, &style_btn, LV_PART_MAIN);
    lv_obj_align(btn_play, LV_ALIGN_CENTER, 0, 0);
    lv_obj_t *label_play = lv_label_create(btn_play);
    lv_label_set_text(label_play, LV_SYMBOL_PLAY);
    lv_obj_set_style_text_font(label_play, &lv_font_montserrat_30, 0);
    lv_obj_set_size(label_play, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_text_align(label_play, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_border_width(label_play, 2, LV_STATE_FOCUSED);
    lv_obj_set_style_border_color(label_play, lv_palette_main(LV_PALETTE_LIGHT_BLUE), LV_STATE_FOCUSED);
    lv_obj_center(label_play);

    lv_obj_t *btn_prev = lv_btn_create(img);
    lv_obj_set_size(btn_prev, 45, 45);
    lv_obj_set_style_radius(btn_prev, LV_RADIUS_CIRCLE, 0);
    lv_obj_add_style(btn_prev, &style_btn, LV_PART_MAIN);
    lv_obj_align_to(btn_prev, label_play, LV_ALIGN_OUT_LEFT_MID, -40, 0);
    lv_obj_t *label_prev = lv_label_create(btn_prev);
    lv_label_set_text(label_prev, LV_SYMBOL_PREV);
    lv_obj_set_style_text_font(label_prev, &lv_font_montserrat_24, 0);
    lv_obj_set_size(label_prev, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_text_align(label_prev, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_border_width(label_prev, 2, LV_STATE_FOCUSED);
    lv_obj_set_style_border_color(label_prev, lv_palette_main(LV_PALETTE_LIGHT_BLUE), LV_STATE_FOCUSED);
    lv_obj_center(label_prev);

    lv_obj_t *btn_next = lv_btn_create(img);
    lv_obj_set_size(btn_next, 45, 45);
    lv_obj_set_style_radius(btn_next, LV_RADIUS_CIRCLE, 0);
    lv_obj_add_style(btn_next, &style_btn, LV_PART_MAIN);
    lv_obj_align_to(btn_next, label_play, LV_ALIGN_OUT_RIGHT_MID, 40, 0);
    lv_obj_t *label_next = lv_label_create(btn_next);
    lv_label_set_text(label_next, LV_SYMBOL_NEXT);
    lv_obj_set_style_text_font(label_next, &lv_font_montserrat_24, 0);
    lv_obj_set_size(label_next, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_text_align(label_next, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_border_width(label_next, 2, LV_STATE_FOCUSED);
    lv_obj_set_style_border_color(label_next, lv_palette_main(LV_PALETTE_LIGHT_BLUE), LV_STATE_FOCUSED);
    lv_obj_center(label_next);

    ui_add_obj_to_encoder_group(btn_play);
    ui_add_obj_to_encoder_group(btn_next);
    ui_add_obj_to_encoder_group(btn_prev);
    ui_add_obj_to_encoder_group(btn_mode);
    ui_add_obj_to_encoder_group(arc_volume);
    lv_group_focus_obj(btn_play);

    lv_obj_add_event_cb(btn_play, player_event_cb, LV_EVENT_LONG_PRESSED, NULL);
    lv_obj_add_event_cb(btn_next, player_event_cb, LV_EVENT_LONG_PRESSED, NULL);
    lv_obj_add_event_cb(btn_prev, player_event_cb, LV_EVENT_LONG_PRESSED, NULL);
    lv_obj_add_event_cb(btn_mode, player_event_cb, LV_EVENT_LONG_PRESSED, NULL);
    lv_obj_add_event_cb(arc_volume, player_event_cb, LV_EVENT_LONG_PRESSED, NULL);

    lv_anim_t a1;
    lv_anim_init(&a1);
    lv_anim_set_var(&a1, arc_time);
    lv_anim_set_delay(&a1, 0);
    lv_anim_set_values(&a1, lv_obj_get_y_aligned(arc_time) - 10, lv_obj_get_y_aligned(arc_time));
    lv_anim_set_exec_cb(&a1, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_path_cb(&a1, lv_anim_path_overshoot);
    lv_anim_set_time(&a1, 500);
    lv_anim_start(&a1);

    lv_anim_t a2;
    lv_anim_init(&a2);
    lv_anim_set_var(&a2, label_name);
    lv_anim_set_values(&a2, lv_obj_get_y_aligned(label_name) - 30, lv_obj_get_y_aligned(label_name));
    lv_anim_set_exec_cb(&a2, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_path_cb(&a2, lv_anim_path_overshoot);
    lv_anim_set_time(&a2, 500);
    lv_anim_start(&a2);

    lv_anim_t a3;
    lv_anim_init(&a3);
    lv_anim_set_var(&a3, label_album);
    lv_anim_set_values(&a3, lv_obj_get_y_aligned(label_album) - 30, lv_obj_get_y_aligned(label_album));
    lv_anim_set_exec_cb(&a3, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_path_cb(&a3, lv_anim_path_overshoot);
    lv_anim_set_time(&a3, 500);
    lv_anim_start(&a3);

    lv_anim_t a4;
    lv_anim_init(&a4);
    lv_anim_set_var(&a4, label_time);
    lv_anim_set_delay(&a4, 70);
    lv_anim_set_values(&a4, lv_obj_get_y_aligned(label_time) + 30, lv_obj_get_y_aligned(label_time));
    lv_anim_set_exec_cb(&a4, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_path_cb(&a4, lv_anim_path_overshoot);
    lv_anim_set_time(&a4, 500);
    lv_anim_start(&a4);

    lv_anim_t a5;
    lv_anim_init(&a5);
    lv_anim_set_var(&a5, label_mode);
    lv_anim_set_delay(&a5, 50);
    lv_anim_set_values(&a5, lv_obj_get_y_aligned(label_mode) + 20, lv_obj_get_y_aligned(label_mode));
    lv_anim_set_exec_cb(&a5, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_path_cb(&a5, lv_anim_path_overshoot);
    lv_anim_set_time(&a5, 500);
    lv_anim_start(&a5);

    lv_anim_t a6;
    lv_anim_init(&a6);
    lv_anim_set_var(&a6, arc_volume);
    lv_anim_set_delay(&a6, 80);
    lv_anim_set_values(&a6, lv_obj_get_y_aligned(arc_volume) + 10, lv_obj_get_y_aligned(arc_volume));
    lv_anim_set_exec_cb(&a6, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_path_cb(&a6, lv_anim_path_overshoot);
    lv_anim_set_time(&a6, 500);
    lv_anim_start(&a6);
}

static bool player_Layer_enter_cb(struct lv_layer_t *layer)
{
    bool ret = false;

    if (NULL == layer->lv_obj_layer) {
        ret = true;
        layer->lv_obj_layer = lv_obj_create(lv_scr_act());
        lv_obj_remove_style_all(layer->lv_obj_layer);
        lv_obj_set_size(layer->lv_obj_layer, LV_HOR_RES, LV_VER_RES);

        ui_player_init(layer->lv_obj_layer);
        set_time_out(&time_20ms, 20);
    }

    return ret;
}

static bool player_Layer_exit_cb(struct lv_layer_t *layer)
{
    LV_LOG_USER("");
    return true;
}

static void player_Layer_timer_cb(lv_timer_t *tmr)
{
    if (is_time_out(&time_20ms)) {
        //player_Lhandler();
    }
}

