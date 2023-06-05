/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "ui_main.h"
#include "device.h"
#include "app_wifi.h"

/* LVGL objects defination */
static lv_obj_t *status_bar = NULL;
static lv_obj_t *btn_wifi = NULL;
static lv_obj_t *btn_setting = NULL;
static lv_obj_t *btn_battery = NULL;
static lv_obj_t *btn_little_time = NULL;
static lv_obj_t *btn_home = NULL;
static lv_obj_t *btn_switch = NULL;
static lv_obj_t *btn_music = NULL;

/* Static function forward declaration */
static void btn_cb(lv_obj_t *obj, lv_event_t event);
static void ui_status_bar_update_battery(lv_task_t *task);

void ui_status_bar_init(void)
{
    status_bar = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_hidden(status_bar, true);
    lv_obj_set_size(status_bar, 800, 50);
    lv_obj_set_style_local_bg_color(status_bar, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);
    lv_obj_set_style_local_radius(status_bar, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_border_width(status_bar, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_align(status_bar, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);

    btn_little_time = lv_btn_create(status_bar, NULL);
    lv_obj_set_style_local_outline_width(btn_little_time, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_radius(btn_little_time, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 25);
    lv_obj_set_style_local_value_str(btn_little_time, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "08:00");
    lv_obj_set_style_local_bg_color(btn_little_time, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);
    lv_obj_set_style_local_border_color(btn_little_time, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);
    lv_obj_set_style_local_value_font(btn_little_time, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, &font_clock_32);
    lv_obj_set_style_local_value_align(btn_little_time, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
    lv_obj_set_style_local_value_color(btn_little_time, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xd9e1f9));
    lv_obj_set_event_cb(btn_little_time, btn_cb);
    lv_obj_align(btn_little_time, NULL, LV_ALIGN_CENTER, 0, 0);

    btn_setting = lv_btn_create(status_bar, NULL);
    lv_obj_set_style_local_radius(btn_setting, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 25);
    lv_obj_set_width(btn_setting, 50);
    lv_obj_set_style_local_outline_width(btn_setting, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_bg_color(btn_setting, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);
    lv_obj_set_style_local_border_color(btn_setting, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);
    lv_obj_set_style_local_value_font(btn_setting, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, &font_bar_symbol);
    lv_obj_set_style_local_value_color(btn_setting, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xd9e1f9));
    lv_obj_set_style_local_value_str(btn_setting, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_SETTINGS);
    lv_obj_set_style_local_value_str(btn_setting, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_SYMBOL_EXTRA_SETTING_SOLID);
    lv_obj_align(btn_setting, status_bar, LV_ALIGN_IN_RIGHT_MID, -20, 0);
    lv_obj_set_event_cb(btn_setting, btn_cb);

    btn_battery = lv_btn_create(status_bar, NULL);
    lv_obj_set_style_local_radius(btn_battery, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 25);
    lv_obj_set_width(btn_battery, 50);
    lv_obj_set_click(btn_battery, false);
    lv_obj_set_event_cb(btn_battery, btn_cb);
    lv_obj_set_style_local_bg_color(btn_battery, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);
    lv_obj_set_style_local_border_color(btn_battery, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);
    lv_obj_set_style_local_value_font(btn_battery, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, &font_bar_symbol);
    lv_obj_set_style_local_value_color(btn_battery, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xd9e1f9));
    lv_obj_set_style_local_value_str(btn_battery, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_EXTRA_BATTERY_FULL);
    lv_obj_align(btn_battery, btn_setting, LV_ALIGN_OUT_LEFT_MID, -20, 0);

    btn_wifi = lv_btn_create(status_bar, NULL);
    lv_obj_set_width(btn_wifi, 50);
    lv_obj_set_click(btn_wifi, false);
    lv_obj_set_style_local_radius(btn_wifi, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 25);
    lv_obj_set_style_local_bg_color(btn_wifi, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);
    lv_obj_set_style_local_border_color(btn_wifi, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);
    lv_obj_set_style_local_value_font(btn_wifi, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, &font_bar_symbol);
    lv_obj_set_style_local_value_color(btn_wifi, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xd9e1f9));
    lv_obj_set_style_local_value_str(btn_wifi, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_EXTRA_WIFI_SLASH);
    lv_obj_align(btn_wifi, btn_battery, LV_ALIGN_OUT_LEFT_MID, -20, 0);

    btn_home = lv_btn_create(status_bar, NULL);
    lv_obj_set_width(btn_home, 50);
    lv_obj_set_style_local_outline_width(btn_home, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_radius(btn_home, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 25);
    lv_obj_set_style_local_bg_color(btn_home, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);
    lv_obj_set_style_local_border_color(btn_home, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);
    lv_obj_set_style_local_value_font(btn_home, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, &font_bar_symbol);
    lv_obj_set_style_local_value_color(btn_home, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xd9e1f9));
    lv_obj_set_style_local_value_str(btn_home, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_HOME);
    lv_obj_align(btn_home, NULL, LV_ALIGN_IN_LEFT_MID, 20, 0);
    lv_obj_set_event_cb(btn_home, btn_cb);

    btn_switch = lv_btn_create(status_bar, NULL);
    lv_obj_set_width(btn_switch, 50);
    lv_obj_set_style_local_outline_width(btn_switch, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_radius(btn_switch, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 25);
    lv_obj_set_style_local_bg_color(btn_switch, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);
    lv_obj_set_style_local_border_color(btn_switch, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);
    lv_obj_set_style_local_value_font(btn_switch, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, &font_bar_symbol);
    lv_obj_set_style_local_value_color(btn_switch, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xd9e1f9));
    lv_obj_set_style_local_value_str(btn_switch, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_EXTRA_APP);
    lv_obj_set_style_local_value_str(btn_switch, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_SYMBOL_EXTRA_APP_SOLID);
    lv_obj_align(btn_switch, btn_home, LV_ALIGN_OUT_RIGHT_MID, 20, 0);
    lv_obj_set_event_cb(btn_switch, btn_cb);

    btn_music = lv_btn_create(status_bar, NULL);
    lv_obj_set_width(btn_music, 50);
    lv_obj_set_style_local_outline_width(btn_music, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_radius(btn_music, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 25);
    lv_obj_set_style_local_bg_color(btn_music, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);
    lv_obj_set_style_local_border_color(btn_music, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_BAR);
    lv_obj_set_style_local_value_font(btn_music, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, &font_bar_symbol);
    lv_obj_set_style_local_value_color(btn_music, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xd9e1f9));
    lv_obj_set_style_local_value_str(btn_music, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_EXTRA_MUSIC);
    lv_obj_set_style_local_value_str(btn_music, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_SYMBOL_EXTRA_MUSIC_SOLID);

    lv_obj_align(btn_music, btn_switch, LV_ALIGN_OUT_RIGHT_MID, 20, 0);
    lv_obj_set_event_cb(btn_music, btn_cb);
    lv_task_create(ui_status_bar_update_battery, 100, 1, NULL);

    lv_obj_set_hidden(status_bar, false);
}

void ui_status_bar_time_show(bool show)
{
    if (show) {
        lv_obj_set_hidden(btn_little_time, false);
    } else {
        lv_obj_set_hidden(btn_little_time, true);
    }
}

/* ******************************** Event Handler ******************************** */
static void btn_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_CLICKED == event) {
        if (btn_setting == obj) {
            ui_show(&ui_setting_func, UI_SHOW_OVERRIDE);
            return;
        }

        if (btn_home == obj) {
            ui_show(&ui_clock_func, UI_SHOW_OVERRIDE);
            return;
        }

        if (btn_little_time == obj) {
            ui_show(&ui_clock_func, UI_SHOW_OVERRIDE);
            return;
        }

        if (btn_music == obj) {
            ui_show(&ui_music_func, UI_SHOW_OVERRIDE);
            return;
        }

        if (btn_switch == obj) {
            ui_show(&ui_switch_func, UI_SHOW_OVERRIDE);
            return;
        }
    }
}

void ui_status_bar_set_item_text(status_bar_item_t item, const char *text)
{
    if (NULL == text) {
        return;
    }

    switch (item) {
    case status_bar_item_wifi:
        if (NULL != btn_wifi) {
            lv_obj_set_style_local_value_str(btn_wifi, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, text);
        }
        break;
    case status_bar_item_battery:
        if (NULL != btn_battery) {
            lv_obj_set_style_local_value_str(btn_battery, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, text);
        }
        break;
    case status_bar_item_little_time:
        if (NULL != btn_little_time) {
            lv_obj_set_style_local_value_str(btn_little_time, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, text);
        }
        break;
    default:
        break;
    }
}

static void ui_status_bar_update_battery(lv_task_t *task)
{
    (void) task;

    WiFi_Connect_Status wifi_status = wifi_connected_already();
    if (WIFI_STATUS_CONNECTED_OK == wifi_status) {
        lv_obj_set_style_local_value_str(btn_wifi, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_EXTRA_WIFI_MAX);
    } else {
        lv_obj_set_style_local_value_str(btn_wifi, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_EXTRA_WIFI_SLASH);
    }

    static float voltage = 0.0f;
    static float voltage_avr = 0.0f;
    static uint32_t voltage_count = 0;

    adc_bat_get_voltage(&voltage);

    if (voltage_count < 10) {
        voltage_count++;
        voltage_avr += voltage;
    } else {
        voltage_avr /= 10;
        if (voltage_avr > 4.1f) {
            ui_status_bar_set_item_text(status_bar_item_battery, LV_SYMBOL_EXTRA_BATTERY_FULL);
        } else if (voltage_avr > 3.0f) {
            ui_status_bar_set_item_text(status_bar_item_battery, LV_SYMBOL_EXTRA_BATTERY_THREE_QUARTERS);
        } else if (voltage_avr > 3.9f) {
            ui_status_bar_set_item_text(status_bar_item_battery, LV_SYMBOL_EXTRA_BATTERY_HALF);
        } else if (voltage_avr > 3.8f) {
            ui_status_bar_set_item_text(status_bar_item_battery, LV_SYMBOL_EXTRA_BATTERY_QUARTER);
        } else if (voltage_avr > 3.7f) {
            ui_status_bar_set_item_text(status_bar_item_battery, LV_SYMBOL_EXTRA_BATTERY_EMPTY);
        }

        voltage_count = 0;
        voltage_avr = 0;
    }
}
