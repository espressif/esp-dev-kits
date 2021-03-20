/**
 * @file ui_prov.c
 * @brief Provision UI src.
 * @version 0.1
 * @date 2021-03-07
 * 
 * @copyright Copyright 2021 Espressif Systems (Shanghai) Co. Ltd.
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *               http://www.apache.org/licenses/LICENSE-2.0
 * 
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#include "ui_prov.h"

static const char *TAG = "ui_prov";

#define COLOR_BAR   lv_color_make(86, 94, 102)
#define COLOR_THEME lv_color_make(252, 199, 0)
#define COLOR_DEEP  lv_color_make(246, 174, 61)
#define COLOR_TEXT  lv_color_make(56, 56, 56)
#define COLOR_BG    lv_color_make(238, 241, 245)

static lv_obj_t *page_wifi = NULL;
static lv_obj_t *wifi_progress = NULL;
static lv_obj_t *wifi_list_left = NULL;
static lv_obj_t *wifi_list_right = NULL;
static lv_obj_t *keyboard = NULL;
static lv_obj_t *textarea_pswd = NULL;
static lv_obj_t *btn_connect = NULL;
static lv_obj_t *label_ssid = NULL;
static lv_obj_t *msg = NULL;

static char *ap_ssid = NULL;

/*!< Callback function defination */
static void wifi_list_cb(lv_obj_t *obj, lv_event_t event);
static void btn_connect_cb(lv_obj_t *obj, lv_event_t event);
static void keyboard_event_cb(lv_obj_t *obj, lv_event_t event);
static void msg_event_cb(lv_obj_t *obj, lv_event_t event);

/*!< Tasks defination */
static void wifi_scan_progress_task(lv_task_t *task);

/*!< Private function forward declaration */
void ui_set_list_count(size_t item);
static void btn_pwd_mode_cb(lv_obj_t *obj, lv_event_t event);

void ui_prov_init(void)
{
    lv_port_sem_take();

    page_wifi = lv_page_create(lv_scr_act(), NULL);
    lv_obj_set_click(page_wifi, false);
    lv_obj_set_size(page_wifi, 750, 400);
    // lv_page_set_edge_flash(page_wifi, true);
    lv_obj_set_style_local_outline_color(page_wifi, LV_PAGE_PART_BG, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_outline_color(page_wifi, LV_PAGE_PART_BG, LV_STATE_PRESSED, LV_COLOR_WHITE);
    lv_obj_set_style_local_border_color(page_wifi, LV_PAGE_PART_BG, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_border_color(page_wifi, LV_PAGE_PART_BG, LV_STATE_PRESSED, LV_COLOR_WHITE);
    lv_obj_set_style_local_radius(page_wifi, LV_PAGE_PART_BG, LV_STATE_DEFAULT, 15);
    lv_obj_align(page_wifi, NULL, LV_ALIGN_CENTER, 0, 20);

    wifi_list_left = lv_table_create(page_wifi, NULL);
    lv_table_set_col_cnt(wifi_list_left, 3);
    lv_table_set_row_cnt(wifi_list_left, 4);
    lv_obj_set_style_local_border_width(wifi_list_left, LV_TABLE_PART_BG, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_text_font(wifi_list_left, LV_TABLE_PART_CELL1, LV_STATE_DEFAULT, &font_en_20);

    wifi_list_right = lv_table_create(page_wifi, NULL);
    lv_table_set_col_cnt(wifi_list_right, 3);
    lv_table_set_row_cnt(wifi_list_right, 4);
    lv_obj_set_style_local_border_width(wifi_list_right, LV_TABLE_PART_BG, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_text_font(wifi_list_right, LV_TABLE_PART_CELL1, LV_STATE_DEFAULT, &font_en_20);

    lv_obj_set_drag_parent(wifi_list_left, true);
    lv_obj_set_drag_parent(wifi_list_right, true);

    lv_obj_set_drag_parent(wifi_list_left, true);
    lv_obj_set_drag_parent(wifi_list_right, true);

    /* Set Wi-Fi list's column width */
    lv_table_set_col_width(wifi_list_left, 0, 200);
    lv_table_set_col_width(wifi_list_left, 1, 50);
    lv_table_set_col_width(wifi_list_left, 2, 50);

    lv_table_set_col_width(wifi_list_right, 0, 200);
    lv_table_set_col_width(wifi_list_right, 1, 50);
    lv_table_set_col_width(wifi_list_right, 2, 50);

    lv_obj_align(wifi_list_left, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
    lv_obj_align(wifi_list_right, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);

    /* Set Wi-Fi list's click event */
    lv_obj_set_event_cb(wifi_list_left, wifi_list_cb);
    lv_obj_set_event_cb(wifi_list_right, wifi_list_cb);

    /*!< Create a progress bar to show scanning progress */
    wifi_progress = lv_bar_create(lv_scr_act(), NULL);
    lv_obj_set_hidden(wifi_progress, true);
    lv_obj_set_size(wifi_progress, 750, 6);
    lv_bar_set_range(wifi_progress, 0, 21);
    lv_bar_set_value(wifi_progress, 0, LV_ANIM_ON);
    lv_obj_align(wifi_progress, NULL, LV_ALIGN_IN_TOP_MID, 0, 25);
    lv_obj_set_style_local_bg_color(wifi_progress, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_bg_color(wifi_progress, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, COLOR_THEME);

    /* Initiaize done, show Wi-Fi list */
    lv_obj_set_hidden(page_wifi, false);

    lv_port_sem_give();
}

static void ui_show_pswd_textarea(void)
{
    /*!< Hide Wi-Fi lists */
    lv_obj_set_hidden(wifi_list_left, true);
    lv_obj_set_hidden(wifi_list_right, true);

    /*!< Create textarea object if not created yet */
    if (NULL == textarea_pswd) {
        textarea_pswd = lv_textarea_create(lv_scr_act(), NULL);
        lv_textarea_set_max_length(textarea_pswd, 32);
        lv_textarea_set_one_line(textarea_pswd, true);
        lv_textarea_set_text(textarea_pswd, "");
        lv_textarea_set_pwd_mode(textarea_pswd, true);
        lv_textarea_set_placeholder_text(textarea_pswd, " Password");
        lv_obj_set_size(textarea_pswd, 520, 46);
        lv_obj_align(textarea_pswd, NULL, LV_ALIGN_IN_TOP_LEFT, 70, 140);
        lv_obj_set_style_local_text_font(textarea_pswd, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, &font_en_24);
        lv_obj_set_style_local_border_width(textarea_pswd, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, 0);
        lv_obj_set_style_local_radius(textarea_pswd, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, 6);
        lv_obj_set_style_local_bg_color(textarea_pswd, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, COLOR_BG);

        /*!< Create a 'password mode' button */
        lv_obj_t *btn_pwd_mode = lv_btn_create(textarea_pswd, NULL);
        lv_obj_set_size(btn_pwd_mode, 60, 36);
        lv_obj_set_style_local_value_font(btn_pwd_mode, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, &font_kb_24);
        lv_obj_set_style_local_value_str(btn_pwd_mode, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_EYE_CLOSE);
        lv_obj_set_style_local_border_width(btn_pwd_mode, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
        lv_obj_set_style_local_border_width(btn_pwd_mode, LV_BTN_PART_MAIN, LV_STATE_PRESSED, 0);
        lv_obj_set_style_local_border_width(btn_pwd_mode, LV_BTN_PART_MAIN, LV_STATE_FOCUSED, 0);
        lv_obj_set_style_local_border_color(btn_pwd_mode, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_BG);
        lv_obj_set_style_local_border_color(btn_pwd_mode, LV_BTN_PART_MAIN, LV_STATE_PRESSED, COLOR_BG);
        lv_obj_set_style_local_border_color(btn_pwd_mode, LV_BTN_PART_MAIN, LV_STATE_FOCUSED, COLOR_BG);
        lv_obj_set_style_local_bg_color(btn_pwd_mode, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_BG);
        lv_obj_set_style_local_bg_color(btn_pwd_mode, LV_BTN_PART_MAIN, LV_STATE_PRESSED, COLOR_BG);
        lv_obj_set_style_local_bg_color(btn_pwd_mode, LV_BTN_PART_MAIN, LV_STATE_PRESSED, COLOR_BG);
        lv_obj_set_style_local_outline_color(btn_pwd_mode, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_BG);
        lv_obj_set_style_local_outline_color(btn_pwd_mode, LV_BTN_PART_MAIN, LV_STATE_PRESSED, COLOR_BG);
        lv_obj_set_style_local_outline_color(btn_pwd_mode, LV_BTN_PART_MAIN, LV_STATE_FOCUSED, COLOR_BG);

        // lv_obj_set_style_local_radius(btn_pwd_mode, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 5);
        lv_obj_align(btn_pwd_mode, NULL, LV_ALIGN_IN_RIGHT_MID, 0, 0);
        lv_obj_set_event_cb(btn_pwd_mode, btn_pwd_mode_cb);

        btn_connect = lv_btn_create(lv_scr_act(), NULL);
        lv_obj_set_height(btn_connect, 50);
        lv_obj_set_style_local_value_str(btn_connect, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "Connect");
        lv_obj_set_style_local_value_font(btn_connect, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, &font_en_bold_24);
        lv_obj_set_style_local_value_color(btn_connect, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
        lv_obj_set_style_local_bg_color(btn_connect, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_THEME);
        lv_obj_set_style_local_border_color(btn_connect, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
        lv_obj_align(btn_connect, textarea_pswd, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
        lv_obj_set_event_cb(btn_connect, btn_connect_cb);
    } else {
        lv_obj_set_hidden(textarea_pswd, false);
        lv_obj_set_hidden(btn_connect, false);

    }
}

void ui_set_list_count(size_t item)
{
    if (0 != item % 2) {
        lv_table_set_row_cnt(wifi_list_left, item / 2 + 1);
        lv_table_set_row_cnt(wifi_list_right, item / 2 + 1);    
    } else {
        lv_table_set_row_cnt(wifi_list_left, item / 2);
        lv_table_set_row_cnt(wifi_list_right, item / 2);
    }
}

void ui_set_list_text(size_t index, const char *ap_name, int rssi, bool secure)
{
    static const uint16_t ssid_col_num = 0;
    static const uint16_t rssi_col_num = 1;
    static const uint16_t lock_col_num = 2;
    
    static const int rssi_min = -80;
    static const int rssi_max = -50;

    static lv_obj_t *obj_list = NULL;

    if (0 == index % 2) {
        obj_list = wifi_list_left;
    } else {
        obj_list = wifi_list_right;
    }

    /*!< Set AP name */
    lv_table_set_cell_crop(obj_list, index / 2, ssid_col_num, true);
    lv_table_set_cell_value(obj_list, index / 2, ssid_col_num, ap_name);

    /*!< Set Wi-Fi logo according to RSSI */
    if (rssi < rssi_min) {
        lv_table_set_cell_value(obj_list, index / 2, rssi_col_num, LV_SYMBOL_EXTRA_WIFI_MIN);
    } else if (rssi > rssi_max) {
        lv_table_set_cell_value(obj_list, index / 2, rssi_col_num, LV_SYMBOL_EXTRA_WIFI_MAX);
    } else {
        lv_table_set_cell_value(obj_list, index / 2, rssi_col_num, LV_SYMBOL_EXTRA_WIFI_MID);
    }

    /*!< Show Wi-Fi secure info */
    if (secure) {
        lv_table_set_cell_value(obj_list, index / 2, lock_col_num, LV_SYMBOL_EXTRA_LOCK);
    }
}

static void wifi_list_cb(lv_obj_t *obj, lv_event_t event)
{
    static uint16_t _row = 0, _col = 0;

    if (LV_EVENT_CLICKED == event) {
        lv_table_get_pressed_cell(obj, &_row, &_col);

        /* DO NOT show password enter page if SSID is NULL */
        if ('\0' == strcmp(lv_table_get_cell_value(obj, _row, 0), "")) {
            ESP_LOGE(TAG, "Illigal SSID");
            return;
        }

        ap_ssid = (char *)lv_table_get_cell_value(obj, _row, 0);

        if (NULL != keyboard) {
            lv_obj_set_hidden(keyboard, false);
        } else {
            /*!< Create keyboard and set style and location */
            keyboard = lv_keyboard_create(lv_scr_act(), NULL);
            lv_obj_set_style_local_text_font(keyboard, LV_KEYBOARD_PART_BTN, LV_STATE_DEFAULT, &font_kb_24);
            lv_obj_align(keyboard, NULL,LV_ALIGN_IN_BOTTOM_MID, 0, 0);

            /*!< Set keyboard event callback for hiding itself */
            lv_obj_set_event_cb(keyboard, keyboard_event_cb);

            /*!< Show password textarea */
            ui_show_pswd_textarea();

            /*!< Show SSID name */
            if (NULL == label_ssid) {
                label_ssid = lv_label_create(lv_scr_act(), NULL);
                lv_label_set_text(label_ssid, "");
                lv_obj_set_style_local_text_font(label_ssid, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_bold_28);
                lv_obj_align(label_ssid, NULL, LV_ALIGN_IN_TOP_LEFT, 70, 90);
            }
            lv_obj_set_hidden(label_ssid, false);
            lv_label_set_text_fmt(label_ssid, "Enter password for \"%s\"", lv_table_get_cell_value(obj, _row, 0));

            /*!< Register textarea for keyboard */
            lv_keyboard_set_textarea(keyboard, textarea_pswd);
        }
    }
}

static void keyboard_event_cb(lv_obj_t *obj, lv_event_t event)
{
    lv_keyboard_def_event_cb(keyboard, event);

    if(LV_EVENT_CANCEL == event) {
        if(keyboard) {
            lv_obj_del(keyboard);
            keyboard = NULL;

            lv_obj_set_hidden(textarea_pswd, true);
            lv_obj_set_hidden(btn_connect, true);
            lv_obj_set_hidden(label_ssid, true);
            lv_obj_set_hidden(wifi_list_left, false);
            lv_obj_set_hidden(wifi_list_right, false);
        }
    }
}

static void btn_pwd_mode_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_CLICKED == event) {
        lv_textarea_set_pwd_mode(textarea_pswd, !lv_textarea_get_pwd_mode(textarea_pswd));
        lv_obj_set_style_local_value_str(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_textarea_get_pwd_mode(textarea_pswd) ? LV_SYMBOL_EYE_CLOSE: LV_SYMBOL_EYE_OPEN);
    }
}

void ui_connected(bool connected)
{
    if (connected) {
        lv_obj_del(keyboard);
        lv_obj_del(textarea_pswd);
        lv_obj_del(btn_connect);
        lv_obj_del(label_ssid);
        lv_obj_del(wifi_list_left);
        lv_obj_del(wifi_list_right);
    } else {
        msg = lv_msgbox_create(lv_scr_act(), NULL);
        lv_msgbox_set_text(msg, "Failed connect to AP");
        static const char * btns[] ={"OK", ""};
        lv_msgbox_add_btns(msg, btns);
        lv_obj_set_event_cb(msg, msg_event_cb);
    }
}

void ui_show_ap_info(wifi_ap_record_t *ap_info)
{
    static const char *column_str[] = {
        "SSID", "BSSID", "RSSI", "Channel", "Auth Mode",
    };

    static const char *wifi_second_chan_str[] = {
        "HT20", "HT40+", "HT40-",
    };

    static const char *wifi_auth_mode_str[] = {
        "OPEN", "WEP", "WPA_PSK", "WPA2_PSK",
        "WPA_WPA2_PSK", "WPA2_ENTERPRISE",
        "WPA3_PSK", "WPA2_WPA3_PSK", "WAPI_PSK",
    };

    lv_obj_t *table = lv_table_create(page_wifi, NULL);
    lv_obj_set_style_local_border_width(table, LV_TABLE_PART_BG, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_text_font(table, LV_TABLE_PART_CELL1, LV_STATE_DEFAULT, &font_en_20);
    lv_table_set_row_cnt(table, sizeof column_str / sizeof column_str[0]);
    lv_table_set_col_cnt(table, 2);
    lv_table_set_col_width(table, 0, 200);
    lv_table_set_col_width(table, 1, 400);

    /*!< Add items text to col 1 */
    for (size_t i = 0; i < sizeof column_str / sizeof column_str[0]; i++) {
        lv_table_set_cell_value(table, i, 0, column_str[i]);
    }

    lv_table_set_cell_value_fmt(table, 0, 1,    /*!< SSID */
        "%s",
        (const char *)ap_info->ssid);
    lv_table_set_cell_value_fmt(table, 1, 1,    /*!< BSSID */
        "%02X:%02X:%02X:%02X:%02X:%02X",
        ap_info->bssid[0], ap_info->bssid[1], ap_info->bssid[2],
        ap_info->bssid[3], ap_info->bssid[4], ap_info->bssid[5]);
    
    lv_table_set_cell_value_fmt(table, 2, 1,    /*!< RSSI */
        "%d", ap_info->rssi);

    lv_table_set_cell_value_fmt(table, 3, 1,    /*!< AP Channel */
        "%u - %s", ap_info->primary, wifi_second_chan_str[(int)ap_info->second]);

    lv_table_set_cell_value_fmt(table, 4, 1,    /*!< Auth mode */
        "%s", wifi_auth_mode_str[(int)ap_info->authmode]);

    lv_obj_align(table, page_wifi, LV_ALIGN_CENTER, 0, 0);
}

static void wifi_scan_progress_task(lv_task_t *task)
{
    if (lv_bar_get_value(wifi_progress) < lv_bar_get_max_value(wifi_progress)) {
        lv_bar_set_value(wifi_progress, lv_bar_get_value(wifi_progress) + 1, LV_ANIM_ON);
    } else {
        lv_task_del(task);
        lv_obj_set_hidden(wifi_progress, true);
    }
}

void ui_scan_start(void)
{
    lv_bar_set_value(wifi_progress, 0, LV_ANIM_ON);
    lv_obj_set_hidden(wifi_progress, false);
    lv_task_create(wifi_scan_progress_task, 100, 1, NULL);
}

static void btn_connect_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_CLICKED == event) {
        lv_btn_set_state(obj, LV_BTN_STATE_DISABLED);
        wifi_sta_set_ssid(ap_ssid);
        wifi_sta_set_pswd(lv_textarea_get_text(textarea_pswd));
        wifi_sta_connect();
    }
}

static void msg_event_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_VALUE_CHANGED == event) {
        lv_obj_del(obj);
        lv_obj_set_hidden(label_ssid, true);
        lv_obj_set_hidden(btn_connect, true);
        lv_obj_set_hidden(textarea_pswd, true);
        lv_btn_set_state(btn_connect, LV_BTN_STATE_RELEASED);
        // lv_textarea_set_text(textarea_pswd, "");

        lv_obj_set_hidden(wifi_list_left, false);
        lv_obj_set_hidden(wifi_list_right, false);

        lv_obj_del(keyboard);
        keyboard = NULL;
        
    }
}
