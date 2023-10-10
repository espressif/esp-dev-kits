/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "ui_main.h"
#include "esp_log.h"
#include "esp_heap_caps.h"

#include "esp_wifi.h"
#include "app_wifi.h"
#include "settings.h"

static const char *TAG = "ui_app";
#define LOG_TRACE(...)  ESP_LOGI(TAG, ##__VA_ARGS__)
#define CHECK(a, str, ret_val) do { \
        if (!(a)) { \
            ESP_LOGE(TAG,"%s(%d): %s", __FUNCTION__, __LINE__, str); \
            return (ret_val); \
        } \
    } while(0)

#define COLOUR_YELLOW           0xE9BD85

/* UI function declaration */
ui_func_desc_t ui_app_func = {
    .name = "Wi-Fi Connect",
    .init = ui_app_init,
    .show = ui_app_show,
    .hide = ui_app_hide,
};

/* LVGL objects defination */
static lv_obj_t *wifi_refresh_btn, *obj_page_WiFi = NULL;
static lv_obj_t *wifi_keypad_bg, *wifi_list_list = NULL;

static lv_task_t *wifi_update_task;
static lv_obj_t *passwd_kb, *pwd_ta;

static ui_state_t ui_app_state = ui_state_dis;

static uint8_t ssid_select = 0x00;

void show_list_hiden_keypad(bool enable)
{
    if (enable) {
        lv_obj_set_hidden(wifi_list_list, false);
        lv_obj_set_hidden(wifi_refresh_btn, false);
        lv_obj_set_hidden(wifi_keypad_bg, true);
        ui_page_show("Wi-Fi Connect");
    } else {
        lv_obj_set_hidden(wifi_list_list, true);
        lv_obj_set_hidden(wifi_refresh_btn, true);
        lv_obj_set_hidden(wifi_keypad_bg, false);
        ui_page_show(scan_result.ap_info[ssid_select].ssid);
    }
}

static void wifi_list_event_handler(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_SHORT_CLICKED == event) {
        ssid_select = lv_list_get_btn_index(wifi_list_list, obj);
        ESP_LOGW(TAG, "Select:%d, %s", ssid_select, lv_list_get_btn_text(obj));

        show_list_hiden_keypad(false);
        lv_textarea_set_text(pwd_ta, "");
        sys_param_t *sys_set = settings_get_parameter();
        if (0 == strcmp((char *)scan_result.ap_info[ssid_select].ssid, (char *)sys_set->ssid)) {
            lv_textarea_set_placeholder_text(pwd_ta, (const char *)sys_set->password);
        } else {
            lv_textarea_set_placeholder_text(pwd_ta, "Please input passwd");
        }
    }
}

static void wifi_refresh_event_handler(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_SHORT_CLICKED == event) {
        ESP_LOGI(TAG, "Refresh WiFi List");
        send_network_event(NET_EVENT_SCAN);
    }
}

static void kb_event_cb(lv_obj_t *keyboard, lv_event_t event)
{
    lv_keyboard_def_event_cb(passwd_kb, event);
    if (event == LV_EVENT_CANCEL) {
    } else if (event == LV_EVENT_APPLY) {
        sys_param_t *sys_set = settings_get_parameter();

        ESP_LOGW(TAG, "KB READY:%s,%s",
                 scan_result.ap_info[ssid_select].ssid, lv_textarea_get_text(pwd_ta));

        sys_set->ssid_len = strlen((char *)scan_result.ap_info[ssid_select].ssid);
        memset(sys_set->ssid, 0, sizeof(sys_set->ssid));
        memcpy(sys_set->ssid, scan_result.ap_info[ssid_select].ssid, sys_set->ssid_len);

        sys_set->password_len = strlen(lv_textarea_get_text(pwd_ta));
        memset(sys_set->password, 0, sizeof(sys_set->password));
        memcpy(sys_set->password, lv_textarea_get_text(pwd_ta), sys_set->password_len);

        settings_write_parameter_to_nvs();
        send_network_event(NET_EVENT_RECONNECT);
        show_list_hiden_keypad(true);
    }
}

static void ta_event_cb(lv_obj_t *ta, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED) {
        /* Focus on the clicked text area */
        if (passwd_kb != NULL) {
            lv_keyboard_set_textarea(passwd_kb, ta);
        }
    }
}

static void create_passwd_textarea(lv_obj_t *parent)
{
    sys_param_t *sys_set = settings_get_parameter();

    /* Create the password box */
    pwd_ta = lv_textarea_create(parent, NULL);
    lv_obj_set_style_local_border_width(pwd_ta, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_border_width(pwd_ta, LV_BTN_PART_MAIN, LV_STATE_FOCUSED, 0);
    lv_obj_set_style_local_outline_width(pwd_ta, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_outline_width(pwd_ta, LV_BTN_PART_MAIN, LV_STATE_FOCUSED, 0);
    lv_obj_set_style_local_text_font(pwd_ta, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_24);
    lv_textarea_set_one_line(pwd_ta, true);
    lv_textarea_set_cursor_hidden(pwd_ta, true);
    lv_obj_set_size(pwd_ta, 650, 80);
    lv_obj_set_pos(pwd_ta, 5, 20);
    lv_obj_set_event_cb(pwd_ta, ta_event_cb);

    lv_textarea_set_text(pwd_ta, "");
    if (0 == strcmp((char *)scan_result.ap_info[ssid_select].ssid, (char *)sys_set->ssid)) {
        lv_textarea_set_placeholder_text(pwd_ta, (const char *)sys_set->password);
    } else {
        lv_textarea_set_placeholder_text(pwd_ta, "Please input passwd");
    }

    /* Create a keyboard */
    passwd_kb = lv_keyboard_create(parent, NULL);
    lv_obj_set_size(passwd_kb, 650, 200);
    lv_obj_set_pos(passwd_kb, 5, 120);
    lv_keyboard_set_textarea(passwd_kb, pwd_ta);
    lv_keyboard_set_cursor_manage(passwd_kb, true);
    lv_obj_set_event_cb(passwd_kb, kb_event_cb);
}

static void update_wifi_list(lv_obj_t *parent)
{
    uint8_t list_cn;
    lv_obj_t *list_btn, *label_item;
    sys_param_t *sys_set = settings_get_parameter();

    if (NULL == parent) {
        return;
    }

    app_wifi_lock(0);

    if (WIFI_SCAN_BUSY == scan_result.scan_done) {
        if (false == parent->hidden) {
            lv_obj_set_hidden(parent, true);
        }

        do {
            list_cn = lv_list_get_size(parent);
            for (int i = 0; i < list_cn; i++) {
                lv_list_remove(parent, i);
            }
        } while (lv_list_get_size(parent));
    } else if ((WIFI_SCAN_RENEW == scan_result.scan_done) || (WIFI_SCAN_UPDATE == scan_result.scan_done)) {
        if (true == parent->hidden) {
            lv_obj_set_hidden(parent, false);
        }

        list_cn = lv_list_get_size(parent);

        if (WIFI_SCAN_UPDATE == scan_result.scan_done) {
            do {
                list_cn = lv_list_get_size(parent);
                for (int i = 0; i < list_cn; i++) {
                    lv_list_remove(parent, i);
                }
            } while (lv_list_get_size(parent));
        } else {
            ESP_LOGI(TAG, "list_cn:%d, [%s]", list_cn, "renew");
        }

        for (int i = 0; i < scan_result.ap_count; i++) {
            if ((0 == strcmp((char *)sys_set->ssid, (char *)scan_result.ap_info[i].ssid)) && (i > 0)) {
                uint8_t replace_ssid[32] = {0};
                memcpy(replace_ssid, scan_result.ap_info[0].ssid, sizeof(replace_ssid));
                memcpy(scan_result.ap_info[0].ssid,
                       scan_result.ap_info[i].ssid, sizeof(replace_ssid));
                memcpy(scan_result.ap_info[i].ssid, replace_ssid, sizeof(replace_ssid));
            }
        }

        for (int i = 0; i < scan_result.ap_count; i++) {

            list_btn = lv_list_add_btn(parent, NULL, NULL);
            lv_obj_set_size(list_btn, 400, 30);
            // lv_obj_set_style_local_border_width(list_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
            // lv_obj_set_style_local_border_width(list_btn, LV_BTN_PART_MAIN, LV_STATE_FOCUSED, 0);
            lv_obj_set_style_local_outline_width(list_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
            lv_obj_set_style_local_outline_width(list_btn, LV_BTN_PART_MAIN, LV_STATE_FOCUSED, 0);

            lv_obj_set_event_cb(list_btn, wifi_list_event_handler);

            label_item = lv_label_create(list_btn, NULL);
            lv_obj_set_style_local_text_font(label_item, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_24);

            if (0 == strcmp((char *)scan_result.ap_info[i].ssid, (char *)sys_set->ssid)) {

                char ssid_conected[32 + 10];
                memset(ssid_conected, 0, sizeof(ssid_conected));

                WiFi_Connect_Status status = wifi_connected_already();
                if (WIFI_STATUS_CONNECTING == status) {
                    sprintf(ssid_conected, "%s %s", scan_result.ap_info[i].ssid,
                            (SR_LANG_CN == sys_set->sr_lang) ? "连接中" : "Connecting");
                } else if (WIFI_STATUS_CONNECTED_FAILED == status) {
                    sprintf(ssid_conected, "%s %s", scan_result.ap_info[i].ssid,
                            (SR_LANG_CN == sys_set->sr_lang) ? "未连接" : "Disconnected");
                } else {
                    sprintf(ssid_conected, "%s %s", scan_result.ap_info[i].ssid,
                            (SR_LANG_CN == sys_set->sr_lang) ? "已连接" : "Connected");
                }

                lv_label_set_text(label_item, ssid_conected);
                lv_obj_set_style_local_text_color(label_item, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(COLOUR_YELLOW));
            } else {
                lv_obj_set_style_local_text_color(label_item, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x00));
                lv_label_set_text(label_item, (char *)scan_result.ap_info[i].ssid);
            }
            lv_obj_align(label_item, NULL, LV_ALIGN_IN_LEFT_MID, 0, 0);

            lv_obj_t *label_enter = lv_label_create(list_btn, NULL);
            lv_obj_set_style_local_text_font(label_enter, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_16);
            lv_label_set_text(label_enter, LV_SYMBOL_RIGHT);
            lv_obj_align(label_enter, NULL, LV_ALIGN_IN_RIGHT_MID, 0, 0);
        }

        list_cn = lv_list_get_size(parent);
        ESP_LOGI(TAG, "list_cn:%d, [%s]", list_cn, "end");
    }

    app_wifi_unlock();

    if ((WIFI_SCAN_RENEW == scan_result.scan_done) || (WIFI_SCAN_UPDATE == scan_result.scan_done)) {
        app_wifi_state_set(WIFI_SCAN_IDLE);
    }
}

void ui_app_init(void *data)
{
    ui_page_show("Wi-Fi Connect");
    obj_page_WiFi = ui_page_get_obj();

    wifi_refresh_btn = lv_btn_create(obj_page_WiFi, NULL);
    lv_obj_align(wifi_refresh_btn, NULL, LV_ALIGN_IN_TOP_RIGHT, 10, 5);
    lv_obj_set_size(wifi_refresh_btn, 50, 50);
    lv_obj_set_event_cb(wifi_refresh_btn, wifi_refresh_event_handler);
    lv_obj_set_style_local_bg_color(wifi_refresh_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_bg_color(wifi_refresh_btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_WHITE);
    lv_obj_set_style_local_value_color(wifi_refresh_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_THEME);
    lv_obj_set_style_local_value_color(wifi_refresh_btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, COLOR_THEME);
    lv_obj_set_style_local_outline_color(wifi_refresh_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_outline_color(wifi_refresh_btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_WHITE);
    lv_obj_set_style_local_border_color(wifi_refresh_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_border_color(wifi_refresh_btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_WHITE);
    lv_obj_set_style_local_value_str(wifi_refresh_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_REFRESH);
    lv_obj_set_style_local_value_font(wifi_refresh_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_40);

    wifi_list_list = lv_list_create(obj_page_WiFi, NULL);
    lv_obj_set_size(wifi_list_list, 700, 310);
    lv_obj_align(wifi_list_list, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

    app_wifi_state_set(WIFI_SCAN_RENEW);
    update_wifi_list(wifi_list_list);

    wifi_keypad_bg = lv_list_create(obj_page_WiFi, NULL);
    lv_obj_set_size(wifi_keypad_bg, 700, 310);
    lv_obj_set_style_local_border_width(wifi_keypad_bg, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_border_width(wifi_keypad_bg, LV_BTN_PART_MAIN, LV_STATE_FOCUSED, 0);
    lv_obj_align(wifi_keypad_bg, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

    create_passwd_textarea(wifi_keypad_bg);

    ui_app_state = ui_state_show;
}

static void ui_wifi_list_timer_cb(lv_task_t *task)
{
    update_wifi_list(wifi_list_list);
}

void ui_app_show(void *data)
{
    if (ui_state_dis == ui_app_state) {
        ui_app_init(data);
        wifi_update_task = lv_task_create(ui_wifi_list_timer_cb, 100, 1, NULL);
    } else if (ui_state_hide == ui_app_state) {
        ui_page_show("Wi-Fi Connect");
        ui_app_state = ui_state_show;
        lv_task_set_cb(wifi_update_task, ui_wifi_list_timer_cb);
    }
    show_list_hiden_keypad(true);
}

void ui_app_hide(void *data)
{
    if (ui_state_show == ui_app_state) {
        lv_obj_set_hidden(wifi_list_list, true);
        lv_obj_set_hidden(wifi_keypad_bg, true);
        lv_obj_set_hidden(obj_page_WiFi, true);
        lv_obj_set_hidden(wifi_refresh_btn, true);

        ui_app_state = ui_state_hide;
        lv_task_set_cb(wifi_update_task, NULL);
    }
}
