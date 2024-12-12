/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "ui_main.h"
#include "bsp_board_extra.h"

/* UI function declaration */
ui_func_desc_t ui_led_func = {
    .name = "Light Control",
    .init = ui_led_init,
    .show = ui_led_show,
    .hide = ui_led_hide,
};

static ui_state_t ui_led_state = ui_state_dis;

bool led_on = false;

/* Defaukt saturation and brightness value */
static uint8_t saturation_val = 100;
static uint8_t brightness_val = 50;

/* LVGL objects defination */
static lv_obj_t *obj_page_led = NULL;
static lv_obj_t *_color_sel = NULL;
static lv_obj_t *_slider_saturation = NULL;
static lv_obj_t *_slider_brightness = NULL;
static lv_obj_t *_label_on_off = NULL;
static lv_obj_t *_label_brightness = NULL;
static lv_obj_t *_label_saturation = NULL;
static lv_obj_t *_label_brightness_val = NULL;
static lv_obj_t *_label_saturation_val = NULL;
static lv_obj_t *_btn_pwr = NULL;

/* Static function forward declaration */
static void slider_brightness_cb(lv_obj_t *obj, lv_event_t event);
static void color_sel_cb(lv_obj_t *obj, lv_event_t event);
static void _btn_pwr_cb(lv_obj_t *obj, lv_event_t event);
static void slider_saturation_cb(lv_obj_t *obj, lv_event_t event);
static void led_color_update(void);

void ui_led_init(void *data)
{
    ui_page_show("Light Control");
    obj_page_led = ui_page_get_obj();

    _color_sel = lv_cpicker_create(obj_page_led, NULL);
    lv_obj_set_event_cb(_color_sel, color_sel_cb);
    // lv_obj_set_style_local_scale_width(_color_sel, LV_CPICKER_PART_MAIN, LV_STATE_DEFAULT, 85);
    // lv_obj_set_style_local_pad_inner(_color_sel, LV_CPICKER_PART_MAIN, LV_STATE_DEFAULT, 120);
    lv_cpicker_set_color_mode_fixed(_color_sel, true);
    lv_obj_align(_color_sel, NULL, LV_ALIGN_IN_LEFT_MID, 50, 25);
    lv_obj_set_event_cb(_color_sel, color_sel_cb);

    _slider_saturation = lv_slider_create(obj_page_led, NULL);
    lv_obj_set_width(_slider_saturation, 300);
    lv_slider_set_range(_slider_saturation, 0, 100);
    lv_slider_set_value(_slider_saturation, 0, LV_ANIM_ON);
    lv_obj_set_style_local_bg_color(_slider_saturation, LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, COLOR_THEME);
    lv_obj_set_style_local_border_color(_slider_saturation, LV_SLIDER_PART_BG, LV_STATE_FOCUSED, COLOR_THEME);
    lv_obj_set_style_local_outline_color(_slider_saturation, LV_SLIDER_PART_BG, LV_STATE_FOCUSED, COLOR_THEME);
    lv_obj_set_event_cb(_slider_saturation, slider_saturation_cb);

    _slider_brightness = lv_slider_create(obj_page_led, NULL);
    lv_obj_set_width(_slider_brightness, 300);
    lv_slider_set_range(_slider_brightness, 0, 100);
    lv_slider_set_value(_slider_brightness, 0, LV_ANIM_ON);
    lv_obj_set_style_local_bg_color(_slider_brightness, LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, COLOR_THEME);
    lv_obj_set_style_local_border_color(_slider_brightness, LV_SLIDER_PART_BG, LV_STATE_FOCUSED, COLOR_THEME);
    lv_obj_set_style_local_outline_color(_slider_brightness, LV_SLIDER_PART_BG, LV_STATE_FOCUSED, COLOR_THEME);
    lv_obj_set_event_cb(_slider_brightness, slider_brightness_cb);

    _label_on_off = lv_label_create(obj_page_led, NULL);
    lv_label_set_recolor(_label_on_off, true);
    lv_label_set_text(_label_on_off, "ON / #f6ae3d OFF#");

    lv_obj_align(_slider_saturation, NULL, LV_ALIGN_IN_RIGHT_MID, -50, -25);
    lv_obj_align(_slider_brightness, NULL, LV_ALIGN_IN_RIGHT_MID, -50, 50);

    lv_obj_set_style_local_text_font(_label_on_off, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_24);
    lv_obj_align(_label_on_off, _slider_brightness, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 30);

    _label_brightness_val = lv_label_create(obj_page_led, NULL);
    lv_label_set_text(_label_brightness_val, "0%");
    lv_obj_set_style_local_text_font(_label_brightness_val, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_20);
    lv_obj_align(_label_brightness_val, _slider_brightness, LV_ALIGN_OUT_TOP_RIGHT, 0, -10);

    _label_brightness = lv_label_create(obj_page_led, NULL);
    lv_label_set_text(_label_brightness, "Brightness");
    lv_obj_set_style_local_text_font(_label_brightness, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_24);
    lv_obj_align(_label_brightness, _slider_brightness, LV_ALIGN_OUT_TOP_LEFT, 0, -10);

    _label_saturation = lv_label_create(obj_page_led, NULL);
    lv_label_set_text(_label_saturation, "Saturation");
    lv_obj_set_style_local_text_font(_label_saturation, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_24);
    lv_obj_align(_label_saturation, _slider_saturation, LV_ALIGN_OUT_TOP_LEFT, 0, -10);


    _label_saturation_val = lv_label_create(obj_page_led, NULL);
    lv_label_set_text(_label_saturation_val, "0%");
    lv_obj_set_style_local_text_font(_label_saturation_val, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_20);
    lv_obj_align(_label_saturation_val, _slider_saturation, LV_ALIGN_OUT_TOP_RIGHT, 0, -10);

    _btn_pwr = lv_btn_create(obj_page_led, NULL);
    lv_obj_set_size(_btn_pwr, 50, 50);
    lv_obj_set_style_local_radius(_btn_pwr, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 25);
    lv_obj_set_style_local_value_str(_btn_pwr, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_POWER);
    lv_obj_set_style_local_value_font(_btn_pwr, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, &font_bar_symbol);

    lv_obj_set_style_local_border_width(_btn_pwr, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_outline_color(_btn_pwr, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_BG);
    lv_obj_set_style_local_value_color(_btn_pwr, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_THEME);
    lv_obj_set_style_local_bg_color(_btn_pwr, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_BG);

    /* Change align value after a while */
    lv_obj_align(_btn_pwr, _slider_brightness, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 25);
    lv_obj_set_event_cb(_btn_pwr, _btn_pwr_cb);

    ui_led_state = ui_state_show;
}

void ui_led_show(void *data)
{
    if (ui_state_dis == ui_led_state) {
        ui_led_init(data);
    } else if (ui_state_hide == ui_led_state) {

        lv_obj_set_hidden(_color_sel, false);
        lv_obj_set_hidden(_slider_saturation, false);
        lv_obj_set_hidden(_slider_brightness, false);
        lv_obj_set_hidden(_label_on_off, false);
        lv_obj_set_hidden(_btn_pwr, false);

        lv_obj_set_hidden(_label_saturation, false);
        lv_obj_set_hidden(_label_brightness, false);
        lv_obj_set_hidden(_label_saturation_val, false);
        lv_obj_set_hidden(_label_brightness_val, false);

        ui_page_show("Light Control");

        ui_led_state = ui_state_show;
    }
}

void ui_led_hide(void *data)
{
    if (ui_state_show == ui_led_state) {
        lv_obj_set_hidden(_color_sel, true);
        lv_obj_set_hidden(_slider_saturation, true);
        lv_obj_set_hidden(_slider_brightness, true);
        lv_obj_set_hidden(_label_on_off, true);
        lv_obj_set_hidden(_btn_pwr, true);
        lv_obj_set_hidden(_label_saturation, true);
        lv_obj_set_hidden(_label_brightness, true);
        lv_obj_set_hidden(_label_saturation_val, true);
        lv_obj_set_hidden(_label_brightness_val, true);

        ui_page_hide(NULL);

        ui_led_state = ui_state_hide;
    }
}

void ui_led_update(void)
{
    if (ui_state_dis == ui_led_state) {
        return;
    }

    led_color_update();

    if (led_on) {
        lv_obj_set_style_local_value_color(_btn_pwr, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
        lv_obj_set_style_local_bg_color(_btn_pwr, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_THEME);
        lv_label_set_text(_label_on_off, "#f6ae3d ON# / OFF");
        lv_slider_set_value(_slider_brightness, brightness_val, LV_ANIM_ON);
        lv_slider_set_value(_slider_saturation, saturation_val, LV_ANIM_ON);
        lv_event_send(_slider_brightness, LV_EVENT_VALUE_CHANGED, NULL);
        lv_event_send(_slider_saturation, LV_EVENT_VALUE_CHANGED, NULL);
    } else {
        lv_obj_set_style_local_value_color(_btn_pwr, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_THEME);
        lv_obj_set_style_local_bg_color(_btn_pwr, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_BG);
        lv_label_set_text(_label_on_off, "ON / #f6ae3d OFF#");
        lv_slider_set_value(_slider_brightness, 0, LV_ANIM_ON);
        lv_slider_set_value(_slider_saturation, 0, LV_ANIM_ON);
        lv_label_set_text(_label_brightness_val, "0%");
        lv_label_set_text(_label_saturation_val, "0%");
    }
}

static void slider_saturation_cb(lv_obj_t *obj, lv_event_t event)
{
    static char fmt_text[8];

    if (LV_EVENT_VALUE_CHANGED == event) {
        saturation_val = lv_slider_get_value(obj);

        sprintf(fmt_text, "%d%%", saturation_val);
        lv_label_set_text(_label_saturation_val, fmt_text);

        if (!led_on) {
            lv_event_send(_btn_pwr, LV_EVENT_CLICKED, NULL);
        } else {
        }

        led_color_update();
    }
}

static void slider_brightness_cb(lv_obj_t *obj, lv_event_t event)
{
    static char fmt_text[8];

    if (LV_EVENT_VALUE_CHANGED == event) {
        brightness_val = lv_slider_get_value(obj);

        sprintf(fmt_text, "%d%%", brightness_val);
        lv_label_set_text(_label_brightness_val, fmt_text);

        if (!led_on) {
            lv_event_send(_btn_pwr, LV_EVENT_CLICKED, NULL);
        } else {
        }

        led_color_update();
    }
}

static void color_sel_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_CLICKED == event) {
        if (!led_on) {
            lv_event_send(_btn_pwr, LV_EVENT_CLICKED, NULL);
        }

        led_color_update();
    } else if (LV_EVENT_VALUE_CHANGED == event) {
        if (!led_on) {
            lv_event_send(_btn_pwr, LV_EVENT_CLICKED, NULL);
        }

        led_color_update();
    }
}

static void _btn_pwr_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_CLICKED == event) {
        led_on = !led_on;
        // esp_qcloud_iothub_report_all_property();

        if (led_on) {
            lv_obj_set_style_local_value_color(_btn_pwr, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
            lv_obj_set_style_local_bg_color(_btn_pwr, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_THEME);
            lv_label_set_text(_label_on_off, "#f6ae3d ON# / OFF");
            lv_slider_set_value(_slider_brightness, brightness_val, LV_ANIM_ON);
            lv_slider_set_value(_slider_saturation, saturation_val, LV_ANIM_ON);
            lv_event_send(_slider_brightness, LV_EVENT_VALUE_CHANGED, NULL);
            lv_event_send(_slider_saturation, LV_EVENT_VALUE_CHANGED, NULL);
        } else {
            lv_obj_set_style_local_value_color(_btn_pwr, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_THEME);
            lv_obj_set_style_local_bg_color(_btn_pwr, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_BG);
            lv_label_set_text(_label_on_off, "ON / #f6ae3d OFF#");
            lv_slider_set_value(_slider_brightness, 0, LV_ANIM_ON);
            lv_slider_set_value(_slider_saturation, 0, LV_ANIM_ON);
            lv_label_set_text(_label_brightness_val, "0%");
            lv_label_set_text(_label_saturation_val, "0%");
        }

        led_color_update();
    }
}

static void led_color_update(void)
{
    lv_color_t color;

    if (led_on) {
        color = lv_color_hsv_to_rgb(lv_cpicker_get_hue(_color_sel), lv_slider_get_value(_slider_saturation), lv_slider_get_value(_slider_brightness));
#if LV_COLOR_16_SWAP == 0
        bsp_extra_led_set_rgb(0, color.ch.red << 3, color.ch.green << 2, color.ch.blue << 3);
#else
        bsp_extra_led_set_rgb(0, color.ch.red << 3, (color.ch.green_h << 3 | color.ch.green_l) << 2, color.ch.blue << 3);
#endif
    } else {
        bsp_extra_led_set_rgb(0, 0, 0, 0);
    }
}
