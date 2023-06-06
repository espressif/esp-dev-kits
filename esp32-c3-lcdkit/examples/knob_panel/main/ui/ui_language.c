/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "lvgl.h"
#include <stdio.h>
#include "ui_language.h"

#include "settings.h"
#include "lv_example_pub.h"
#include "lv_example_image.h"

static lv_obj_t *page;
static lv_obj_t *label_EN, *label_CN;
static lv_obj_t *imgbtn_lang_CN, *imgbtn_lang_EN;

static time_out_count time_500ms;

static bool language_Layer_enter_cb(void *layer);
static bool language_Layer_exit_cb(void *layer);
static void language_Layer_timer_cb(lv_timer_t *tmr);

lv_layer_t language_Layer = {
    .lv_obj_name    = "language_Layer",
    .lv_obj_parent  = NULL,
    .lv_obj_layer   = NULL,
    .lv_show_layer  = NULL,
    .enter_cb       = language_Layer_enter_cb,
    .exit_cb        = language_Layer_exit_cb,
    .timer_cb       = language_Layer_timer_cb,
};

static void language_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    sys_param_t *param = settings_get_parameter();

    if (LV_EVENT_FOCUSED == code) {
        lv_group_set_editing(lv_group_get_default(), true);
    } else if (LV_EVENT_KEY == code) {

        if (is_time_out(&time_500ms)) {
            if (lv_obj_has_state(imgbtn_lang_EN, LV_STATE_CHECKED) == false) {
                lv_img_set_src(imgbtn_lang_EN, &language_select);
                lv_obj_add_state(imgbtn_lang_EN, LV_STATE_CHECKED);
                lv_obj_set_style_text_opa(label_EN, LV_OPA_COVER, 0);

                lv_img_set_src(imgbtn_lang_CN, &language_unselect);
                lv_obj_clear_state(imgbtn_lang_CN, LV_STATE_CHECKED);
                lv_obj_set_style_text_opa(label_CN, LV_OPA_40, 0);
                param->language = LANGUAGE_EN;
            } else {
                lv_img_set_src(imgbtn_lang_CN, &language_select);
                lv_obj_add_state(imgbtn_lang_CN, LV_STATE_CHECKED);
                lv_obj_set_style_text_opa(label_CN, LV_OPA_COVER, 0);

                lv_img_set_src(imgbtn_lang_EN, &language_unselect);
                lv_obj_clear_state(imgbtn_lang_EN, LV_STATE_CHECKED);
                lv_obj_set_style_text_opa(label_EN, LV_OPA_40, 0);
                param->language = LANGUAGE_CN;
            }
        }
    } else if ((LV_EVENT_LONG_PRESSED == code) || (LV_EVENT_CLICKED == code)) {
        lv_indev_wait_release(lv_indev_get_next(NULL));
        ui_remove_all_objs_from_encoder_group();
        lv_func_goto_layer(&menu_layer);
        settings_write_parameter_to_nvs();
    }
}

void ui_language_init(lv_obj_t *parent)
{
    page = lv_obj_create(parent);
    lv_obj_set_size(page, LV_HOR_RES, LV_VER_RES);

    lv_obj_set_style_border_width(page, 0, 0);
    lv_obj_set_style_radius(page, 0, 0);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_center(page);

    lv_obj_t *img_language_bg = lv_img_create(page);
    lv_img_set_src(img_language_bg, &language_bg_dither);
    lv_obj_align(img_language_bg, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *labelinfo_top = lv_label_create(page);
    lv_obj_set_style_text_font(labelinfo_top, &HelveticaNeue_Regular_24, 0);
    lv_label_set_text(labelinfo_top, "Please select a");
    lv_obj_align(labelinfo_top, LV_ALIGN_TOP_MID, 0, 40);

    lv_obj_t *labelinfo_bottom = lv_label_create(page);
    lv_obj_set_style_text_font(labelinfo_bottom, &HelveticaNeue_Regular_24, 0);
    lv_label_set_text(labelinfo_bottom, "language");
    lv_obj_align(labelinfo_bottom, LV_ALIGN_TOP_MID, 0, 70);

    imgbtn_lang_EN = lv_img_create(page);
    lv_obj_align(imgbtn_lang_EN, LV_ALIGN_CENTER, -40, 50);
    lv_img_set_src(imgbtn_lang_EN, &language_select);
    lv_obj_add_state(imgbtn_lang_EN, LV_STATE_CHECKED);

    label_EN = lv_label_create(imgbtn_lang_EN);
    lv_obj_set_style_text_font(label_EN, &HelveticaNeue_Regular_24, 0);
    lv_label_set_text(label_EN, "EN");
    lv_obj_set_style_text_color(label_EN, lv_color_hex(COLOUR_BLACK), 0);
    lv_obj_align(label_EN, LV_ALIGN_CENTER, 0, 0);

    imgbtn_lang_CN = lv_img_create(page);
    lv_obj_align(imgbtn_lang_CN, LV_ALIGN_CENTER, 40, 50);
    lv_img_set_src(imgbtn_lang_CN, &language_unselect);
    lv_obj_clear_state(imgbtn_lang_CN, LV_STATE_CHECKED);

    label_CN = lv_label_create(imgbtn_lang_CN);
    lv_obj_set_style_text_font(label_CN, &font_SourceHanSansCN_Medium_22, 0);
    lv_label_set_text(label_CN, "中文");
    lv_obj_set_style_text_opa(label_CN, LV_OPA_40, 0);
    lv_obj_set_style_text_color(label_CN, lv_color_hex(COLOUR_BLACK), 0);
    lv_obj_align(label_CN, LV_ALIGN_CENTER, 0, 0);

    ui_remove_all_objs_from_encoder_group();//roll will add event default.
    lv_obj_add_event_cb(page, language_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(page, language_event_cb, LV_EVENT_FOCUSED, NULL);
    lv_obj_add_event_cb(page, language_event_cb, LV_EVENT_KEY, NULL);
    lv_obj_add_event_cb(page, language_event_cb, LV_EVENT_LONG_PRESSED, NULL);
    ui_add_obj_to_encoder_group(page);
}

static bool language_Layer_enter_cb(void *layer)
{
    bool ret = false;

    LV_LOG_USER("");
    lv_layer_t *create_layer = layer;
    if (NULL == create_layer->lv_obj_layer) {
        ret = true;
        create_layer->lv_obj_layer = lv_obj_create(lv_scr_act());
        lv_obj_remove_style_all(create_layer->lv_obj_layer);
        lv_obj_set_size(create_layer->lv_obj_layer, LV_HOR_RES, LV_VER_RES);

        ui_language_init(create_layer->lv_obj_layer);
        set_time_out(&time_500ms, 100);
    }
    sys_param_t *param = settings_get_parameter();
    param->language = LANGUAGE_EN;

    return ret;
}

static bool language_Layer_exit_cb(void *layer)
{
    LV_LOG_USER("");
    return true;
}

static void language_Layer_timer_cb(lv_timer_t *tmr)
{
    feed_clock_time();
}
