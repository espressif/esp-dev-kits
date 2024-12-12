/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "esp_log.h"
#include "esp_check.h"

#include "ui_main.h"
#include "settings.h"

static const char *TAG = "GUIDE";

/* UI function declaration */
ui_func_desc_t ui_guide_func = {
    .name = "ui_guide",
    .init = ui_guide_init,
    .show = ui_guide_show,
    .hide = NULL,
};

/* LVGL objects defination */
static lv_obj_t *hint_tabview;

typedef enum {
    HINT_PAGE_DEMO,
    HINT_PAGE_SR_GUIDE,
    HINT_PAGE_SR_COMMAND,
} HINT_PAGE_STEP;

LV_IMG_DECLARE(icon_voice);
LV_IMG_DECLARE(icon_display);

extern esp_err_t ui_call_stack_push(ui_func_desc_t *func);

static void dst_select_event_handler()
{
    sys_param_t *sys_set = settings_get_parameter();
    sys_set->need_hint = false;
    settings_write_parameter_to_nvs();

    lv_obj_del(hint_tabview);
    ui_clock_show(NULL);
    ui_call_stack_push(&ui_clock_func);
}

static void demo_select_event_handler(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_SHORT_CLICKED) {
        sys_param_t *sys_set = settings_get_parameter();
        sys_set->demo_gui = true;
        sys_set->sr_enable = false;
        ESP_LOGI(TAG, "demo select:%s, SR Enable:%d", sys_set->demo_gui ? "UI" : "SR", sys_set->sr_enable);
        dst_select_event_handler();
    }
}

static void SR_select_event_handler(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_SHORT_CLICKED) {
        sys_param_t *sys_set = settings_get_parameter();
        sys_set->sr_enable = true;
        lv_tabview_set_tab_act(hint_tabview, HINT_PAGE_SR_GUIDE, LV_ANIM_ON);
        ESP_LOGI(TAG, "demo select:%s, SR Enable:%d", sys_set->demo_gui ? "UI" : "SR", sys_set->sr_enable);
    }
}

static void voice_cmd_event_handler(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_SHORT_CLICKED) {
        ESP_LOGI(TAG, "SR cmd next.");

        sys_param_t *sys_set = settings_get_parameter();
        sys_set->need_hint = false;
        sys_set->need_hint = false;
        settings_write_parameter_to_nvs();

        lv_obj_del(hint_tabview);
        ui_sr_show(NULL);
    }
}

static void voice_guide_event_handler(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_SHORT_CLICKED) {
        ESP_LOGI(TAG, "SR guide next.");
        lv_tabview_set_tab_act(hint_tabview, HINT_PAGE_SR_COMMAND, LV_ANIM_ON);
    }
}

void ui_hint_page_demo_select(lv_obj_t *parent)
{
    lv_obj_t *hint_page = lv_tabview_add_tab(parent, "demo");
    lv_page_set_scroll_propagation(hint_page, false);

    lv_obj_t *label_tips = lv_label_create(hint_page, NULL);
    lv_obj_align(label_tips, NULL, LV_ALIGN_IN_TOP_MID, -200, 80);
    lv_obj_set_style_local_text_font(label_tips, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_24);
    lv_label_set_text(label_tips, "The firmware offers two demos: \nfancy UI and voice control. \nPlease choose the one that suits your preferences.");

    lv_obj_t *btn_SR = lv_btn_create(hint_page, NULL);
    lv_obj_set_style_local_bg_color(btn_SR, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);
    lv_obj_set_style_local_bg_color(btn_SR, LV_BTN_PART_MAIN, LV_STATE_PRESSED, COLOR_BAR);
    lv_obj_set_size(btn_SR, 100, 75);
    lv_obj_align(btn_SR, NULL, LV_ALIGN_CENTER, -130, 60);
    lv_obj_set_event_cb(btn_SR, SR_select_event_handler);

    lv_obj_t *img_sr = lv_img_create(btn_SR, NULL);
    lv_img_set_src(img_sr, &icon_voice);
    lv_obj_align(img_sr, NULL, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *btn_UI = lv_btn_create(hint_page, NULL);
    lv_obj_set_style_local_bg_color(btn_UI, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);
    lv_obj_set_style_local_bg_color(btn_UI, LV_BTN_PART_MAIN, LV_STATE_PRESSED, COLOR_BAR);
    lv_obj_set_size(btn_UI, 100, 75);
    lv_obj_align(btn_UI, NULL, LV_ALIGN_CENTER, 130, 60);
    lv_obj_set_event_cb(btn_UI, demo_select_event_handler);

    lv_obj_t *img_UI = lv_img_create(btn_UI, NULL);
    lv_img_set_src(img_UI, &icon_display);
    lv_obj_align(img_UI, NULL, LV_ALIGN_CENTER, 0, 0);
}

void ui_hint_page_voice_guide(lv_obj_t *parent)
{
    lv_obj_t *btn_next, *label;

    lv_obj_t *hint_page = lv_tabview_add_tab(parent, "STEP");
    lv_page_set_scroll_propagation(hint_page, false);

    lv_obj_t *label_title = lv_label_create(hint_page, NULL);
    lv_obj_set_style_local_text_font(label_title, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_28);
    lv_label_set_recolor(label_title, true);
    lv_label_set_text_static(label_title, "Steps for Voice Assistant");
    lv_obj_align(label_title, NULL, LV_ALIGN_IN_TOP_MID, 0, 50);

    lv_obj_t *label_hint = lv_label_create(hint_page, NULL);
    lv_obj_set_style_local_text_font(label_hint, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_24);
    static const char msg[] = "1: Say \"Hi E. S. P.\" to wake-up the device.\n"
                              "2: Wait for the \"Hi ESP\" shows on screen.\n"
                              "3: Say command, like \"turn on the light\".";
    lv_label_set_text_static(label_hint, msg);
    lv_obj_align(label_hint, NULL, LV_ALIGN_IN_TOP_MID, 0, 100);

    btn_next = lv_btn_create(hint_page, NULL);
    lv_obj_align(btn_next, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, -80, -100);
    label = lv_label_create(btn_next, NULL);
    lv_label_set_text(label, "Next");
    lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_24);
    lv_obj_set_event_cb(btn_next, voice_guide_event_handler);
}

void ui_hint_page_voice_cmd(lv_obj_t *parent)
{
    lv_obj_t *btn_next, *label;

    lv_obj_t *hint_page = lv_tabview_add_tab(parent, "Cmd");
    lv_page_set_scroll_propagation(hint_page, false);

    lv_obj_t *label_title = lv_label_create(hint_page, NULL);
    lv_obj_set_style_local_text_font(label_title, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_28);
    lv_label_set_recolor(label_title, true);
    lv_label_set_text_static(label_title, "Default Command Words:");
    lv_obj_align(label_title, NULL, LV_ALIGN_IN_TOP_MID, 0, 50);

    lv_obj_t *label_cmd_hint = lv_label_create(hint_page, NULL);
    lv_label_set_recolor(label_cmd_hint, true);
    lv_obj_set_style_local_text_font(label_cmd_hint, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_24);
    static const char cmd_msg[] = "\"Switch On / Open the air conditioner\"\n"
                                  "\"Switch Off / Close the air conditioner\"\n"
                                  "\"Raise / Reduce the temperature\"\n"
                                  "\"Switch On / Turn On the light\"\n"
                                  "\"Switch Off / Turn Off the light\"";
    lv_label_set_text_static(label_cmd_hint, cmd_msg);
    lv_obj_align(label_cmd_hint, NULL, LV_ALIGN_IN_TOP_MID, 0, 100);

    btn_next = lv_btn_create(hint_page, NULL);
    lv_obj_align(btn_next, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, -80, -100);
    label = lv_label_create(btn_next, NULL);
    lv_label_set_text(label, "Next");
    lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_24);
    lv_obj_set_event_cb(btn_next, voice_cmd_event_handler);
}

void ui_guide_init(void *data)
{
    (void)data;

    /*Create a Tab view object*/
    hint_tabview = lv_tabview_create(lv_scr_act(), NULL);
    lv_obj_set_size(hint_tabview, LV_HOR_RES, LV_VER_RES);
    lv_tabview_set_btns_pos(hint_tabview, LV_TABVIEW_TAB_POS_NONE);

    ui_hint_page_demo_select(hint_tabview);
    ui_hint_page_voice_guide(hint_tabview);
    ui_hint_page_voice_cmd(hint_tabview);
}

void ui_guide_show(void *data)
{
    if (NULL == hint_tabview) {
        ui_guide_init(data);
        ui_status_bar_time_show(false);
    } else {
        lv_obj_set_hidden(hint_tabview, false);
        ui_status_bar_time_show(true);
    }
}