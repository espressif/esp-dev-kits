/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include "bsp/esp-bsp.h"
#include <time.h>
#include <sys/time.h>
#include "settings.h"

#include "esp_wifi.h"
#include "app_wifi.h"
#include "settings.h"
#include "main.h"

#include "lv_example_pub.h"
#include "lv_example_image.h"

static const char *TAG = "SET";

static bool set_layer_enter_cb(void *create_layer);
static bool set_layer_exit_cb(void *create_layer);
static void set_layer_timer_cb(lv_timer_t *tmr);

static void lv_create_set_home(lv_obj_t *set_background);

static void lv_create_set_wifi(lv_obj_t *set_background);
static void lv_create_set_voice(lv_obj_t *set_background);
static void lv_create_set_bright(lv_obj_t *set_background);
static void lv_create_set_factory_reset(lv_obj_t *set_background);
static void lv_create_set_aboutus(lv_obj_t *set_background);

static void lv_create_wifi_keyboard(lv_obj_t *set_background);
static void lv_create_display_standby(lv_obj_t *set_background);
static void lv_create_display_language(lv_obj_t *set_background);
static void lv_create_display_reboot(lv_obj_t *set_background);

lv_layer_t set_layer = {
    .lv_obj_name = "set_layer",
    .lv_obj_parent = NULL,
    .lv_obj_layer = NULL,
    .lv_show_layer = NULL,
    .enter_cb = set_layer_enter_cb,
    .exit_cb = set_layer_exit_cb,
    .timer_cb = set_layer_timer_cb,
};

static SPRITE_SET_PAGE_LIST sprite_focus_page = SPRITE_SET_PAGE_MAX;

static time_out_count time_1000ms;

static lv_obj_t *list_wifi = NULL;
static lv_obj_t *label_reboot = NULL;
static uint8_t reboot_wait_time = 0;

static uint8_t ssid_list_select = 0xFF;

static sprite_create_func_t sprite_create_list[] = {
    {{"Setting",        "设置"}, lv_create_set_home, NULL},

    {{"WiFi",           "WiFi"}, lv_create_set_wifi, NULL},
    {{"Sound",          "声音"}, lv_create_set_voice, NULL},
    {{"Display",        "亮度与显示"}, lv_create_set_bright, NULL},
    {{"Factory Reset",  "恢复出厂"}, lv_create_set_factory_reset, NULL},
    {{"About",          "关于"}, lv_create_set_aboutus, NULL},

    {{"SSID",           "SSID"}, lv_create_wifi_keyboard, NULL}, // 8
    {{"Stanby Time",    "待机时间"}, lv_create_display_standby, NULL},
    {{"Language",       "语言"}, lv_create_display_language, NULL},
    {{"Reboot",         "Reboot"}, lv_create_display_reboot, NULL},
};

const char *standby_time_list_name[][4] = {
    {
        "15 sec",
        "5 min",
        "1 hour",
        "forever"
    },
    {
        "15秒",
        "5分钟",
        "1小时",
        "永久"
    },
};

const char *standby_home_list_name[][3] = {
    {
        "Brightness",
        "Language",
        "Standby Time"
    },
    {
        "亮度调节",
        "语言",
        "待机时间"
    },
};

const char *aboutus_list_name[][6] = {
    {
        "Manufacturer",
        "Board model",
        "Sub board model",
        "Device ID",
        "Version",
        "SR Version"
    },
    {
        "生产厂家",
        "主板名称",
        "子板名称",
        "设备ID",
        "版本号",
        "语音版本"
    },
};

static uint8_t sprite_focus_page_goto(uint8_t sprite)
{
    lv_obj_t *parent;
    if (sprite ^ sprite_focus_page) {
        sprite_focus_page = sprite;
        for (int i = 0; i < SPRITE_SET_PAGE_MAX; i++) {
            if (sprite_create_list[i].sprite_parent) {
                list_wifi = NULL;
                label_reboot = NULL;
                lv_obj_del(sprite_create_list[i].sprite_parent);
                sprite_create_list[i].sprite_parent = NULL;
            }
        }
        parent = lv_obj_create(set_layer.lv_obj_layer);
        lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
        // lv_obj_add_flag(parent, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
        lv_obj_set_style_border_width(parent, 0, 0);
        lv_obj_set_style_border_side(parent, LV_BORDER_SIDE_BOTTOM, 0);

        sprite_create_list[sprite].sprite_parent = parent;
        sprite_create_list[sprite].sprite_create_func(parent);
    }

    return sprite_focus_page;
}

static void home_list_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    int item_select = (int)lv_event_get_user_data(e);

    if (code == LV_EVENT_SHORT_CLICKED) {
        ESP_LOGI(TAG, "item_select:%d->%d", sprite_focus_page, item_select);
        sprite_focus_page_goto(item_select);
    }
}

static void ta_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *ta = lv_event_get_target(e);
    lv_obj_t *kb = (lv_obj_t *)lv_event_get_user_data(e);
    if (code == LV_EVENT_FOCUSED) {
        lv_keyboard_set_textarea(kb, ta);
        lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
        ESP_LOGI(TAG, "FOCUSED:%d", code);
    }

    if (code == LV_EVENT_DEFOCUSED) {
        lv_keyboard_set_textarea(kb, NULL);
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
        ESP_LOGI(TAG, "DEFOCUSED:%d", code);
    }

    // lv_keyboard_def_event_cb();
    if (code == LV_EVENT_READY) {
        ESP_LOGI(TAG, "LV_EVENT_READY:%s,%s",
                 scan_info_result.ap_info[ssid_list_select].ssid, lv_textarea_get_text(ta));

        sys_set->ssid_len = strlen((char *)scan_info_result.ap_info[ssid_list_select].ssid);
        memset(sys_set->ssid, 0, sizeof(sys_set->ssid));
        memcpy(sys_set->ssid, scan_info_result.ap_info[ssid_list_select].ssid, sys_set->ssid_len);

        sys_set->password_len = strlen(lv_textarea_get_text(ta));
        memset(sys_set->password, 0, sizeof(sys_set->password));
        memcpy(sys_set->password, lv_textarea_get_text(ta), sys_set->password_len);

        settings_write_parameter_to_nvs();
        send_network_event(NET_EVENT_RECONNECT);
        sprite_focus_page_goto(SPRITE_SET_PAGE_WiFi);
        feed_clock_time();
    }

    if (code == LV_EVENT_CANCEL) {
        ESP_LOGI(TAG, "LV_EVENT_CANCEL");
    }
}

static void wifi_list_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    int item_select = (int)lv_event_get_user_data(e);

    if (code == LV_EVENT_SHORT_CLICKED) {
        feed_clock_time();
        ssid_list_select = item_select;
        ESP_LOGI(TAG, "LV_EVENT_SHORT_CLICKED:%d,%s", item_select, scan_info_result.ap_info[ssid_list_select].ssid);
        sprite_focus_page_goto(SPRITE_SET_WIFI_KEYBOARD);
    }
}

static void wifi_refresh_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_SHORT_CLICKED) {
        ESP_LOGI(TAG, "refresh");
        feed_clock_time();
        send_network_event(NET_EVENT_SCAN);
    }
}

static void display_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    int item_select = (int)lv_event_get_user_data(e);

    if (code == LV_EVENT_SHORT_CLICKED) {
        feed_clock_time();
        ESP_LOGI(TAG, "LV_EVENT_SHORT_CLICKED:%d", item_select);
        if (1 == item_select) {
            sprite_focus_page_goto(SPRITE_SET_DISPLAY_STANDBY);
        } else if (0 == item_select) {
            sprite_focus_page_goto(SPRITE_SET_DISPLAY_LANGUAGE);
        }
    }
}

static void factory_reset_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    int item_select = (int)lv_event_get_user_data(e);

    if (code == LV_EVENT_SHORT_CLICKED) {
        feed_clock_time();
        ESP_LOGI(TAG, "factory_reset_event_handler:%d", item_select);
        if (item_select) {
            settings_factory_reset();
            sprite_focus_page_goto(SPRITE_SET_DISPLAY_REBOOT);
        } else {
            sprite_focus_page_goto(SPRITE_SET_PAGE_HOME);
        }
    }
}

static void btn_back_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    int sprite = (int)lv_event_get_user_data(e);

    if (code == LV_EVENT_SHORT_CLICKED) {

        switch (sprite) {
        case SPRITE_SET_PAGE_HOME:
            lv_func_goto_layer(&main_Layer);
            break;
        case SPRITE_SET_WIFI_KEYBOARD:
            sprite_focus_page_goto(SPRITE_SET_PAGE_WiFi);
            break;
        case SPRITE_SET_DISPLAY_STANDBY:
        case SPRITE_SET_DISPLAY_LANGUAGE:
            sprite_focus_page_goto(SPRITE_SET_PAGE_BRIGHT);
            break;
        default:
            sprite_focus_page_goto(SPRITE_SET_PAGE_HOME);
            break;
        }
        feed_clock_time();
    }
}

static void slider_show_event_cb(lv_event_t *e)
{
    feed_clock_time();
    int volume_set = 0;
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *slider = lv_event_get_target(e);
        volume_set = (int)lv_slider_get_value(slider);
        sys_set->brightness = volume_set;
    }
}

static void volume_event_handler(lv_event_t *e)
{
    int *active_id = (int *)lv_event_get_user_data(e);
    lv_obj_t *cont = lv_event_get_current_target(e);
    lv_obj_t *act_cb = lv_event_get_target(e);
    lv_obj_t *old_cb = lv_obj_get_child(cont, *active_id);

    /*Do nothing if the container was clicked*/
    if (act_cb == cont) {
        ESP_LOGI(TAG, "act_cb == cont");
        return;
    }

    lv_obj_clear_state(old_cb, LV_STATE_CHECKED); /*Uncheck the previous radio button*/
    lv_obj_add_state(act_cb, LV_STATE_CHECKED);   /*Uncheck the current radio button*/

    *active_id = lv_obj_get_index(act_cb);
    feed_clock_time();
    ESP_LOGI(TAG, "Selected radio buttons: %d", *active_id);

    if (sys_set->volume ^ (*active_id)) {
        sys_set->volume = *active_id;
        // settings_write_parameter_to_nvs();
        ESP_LOGI(TAG, "save volume:%d", sys_set->volume);
    }
    // es8311_codec_set_voice_volume(60 + (*active_id)*4);
}

static void standby_event_handler(lv_event_t *e)
{
    bool valid_btn = false;
    lv_obj_t **checkboxList = (lv_obj_t **)lv_event_get_user_data(e);
    lv_obj_t *act_cb = lv_event_get_target(e); // checkbox

    for (int i = 0; i < 4; i++) {
        if (*(checkboxList + i) == act_cb) {
            valid_btn = true;
        }
    }

    if (valid_btn) {
        for (int i = 0; i < 4; i++) {
            if (*(checkboxList + i) != act_cb) {
                lv_obj_clear_state(*(checkboxList + i), LV_STATE_CHECKED);
            } else {
                lv_obj_add_state(*(checkboxList + i), LV_STATE_CHECKED);
                sys_set->standby_time = i;
                reset_clock_time(i);
            }
        }
    }
}

const uint32_t clock_time[] = {15 * 1000, 5 * 60 * 1000, 1 * 60 * 60 * 1000, 0xFFFFFFFF};

static void language_event_handler(lv_event_t *e)
{
    bool valid_btn = false;
    lv_obj_t **checkboxList = (lv_obj_t **)lv_event_get_user_data(e);
    lv_obj_t *act_cb = lv_event_get_target(e); // checkbox

    feed_clock_time();

    for (int i = 0; i < 2; i++) {
        if (*(checkboxList + i) == act_cb) {
            valid_btn = true;
        }
    }

    if (valid_btn) {
        for (int i = 0; i < 2; i++) {
            if (*(checkboxList + i) != act_cb) {
                lv_obj_clear_state(*(checkboxList + i), LV_STATE_CHECKED);
            } else {
                lv_obj_add_state(*(checkboxList + i), LV_STATE_CHECKED);
                sys_set->sr_lang = i;
                settings_write_parameter_to_nvs();

                if (sys_set->sr_enable) {
                    ESP_LOGI(TAG, "language: %s", sys_set->sr_lang ? "中文" : "EN");
                    app_sr_set_language(sys_set->sr_lang);
                }
            }
        }
    }
}

static void mute_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_VALUE_CHANGED) {
        feed_clock_time();
        ESP_LOGI(TAG, "State: %s", lv_obj_has_state(obj, LV_STATE_CHECKED) ? "On" : "Off");
        if (lv_obj_has_state(obj, LV_STATE_CHECKED)) {
            // es8311_codec_set_voice_volume(60 + (sys_set->volume)*4);
            ESP_LOGI(TAG, "recover volume:%d", (60 + (sys_set->volume) * 4));
            // es8311_set_voice_mute(false);
        } else {
            // es8311_set_voice_mute(true);
        }
    }
}

void set_button_list_style(lv_obj_t *parent)
{
    lv_obj_set_size(parent, LV_PCT(95), 70);
    lv_obj_set_style_bg_color(parent, lv_color_hex(COLOUR_GREY_2F), LV_PART_MAIN);
    lv_obj_set_style_text_color(parent, lv_color_hex(COLOUR_WHITE), 0);
    lv_obj_set_style_border_color(parent, lv_color_hex(COLOUR_GREY_4F), 0);
    // lv_obj_set_style_transform_width(btn, LV_PCT(90), 0);
    lv_obj_set_style_border_width(parent, 1, 0);
    lv_obj_set_style_border_side(parent, LV_BORDER_SIDE_BOTTOM, 0);
}

lv_obj_t *create_button_list_page(lv_obj_t *parent)
{
    lv_obj_t *list_select = lv_list_create(parent);
    lv_obj_set_size(list_select, LV_PCT(95), LV_PCT(90));
    lv_obj_set_style_border_width(list_select, 0, 0);
    lv_obj_set_style_radius(list_select, 0, 0);
    lv_obj_set_style_text_font(list_select, &SourceHanSansCN_Normal_20, 0);
    lv_obj_align(list_select, LV_ALIGN_TOP_MID, 0, 70);
    lv_obj_set_style_bg_color(list_select, lv_color_hex(COLOUR_GREY_2F), LV_PART_MAIN);

    lv_obj_set_style_bg_opa(list_select, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(list_select, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);

    return list_select;
}

void lv_create_set_title(lv_obj_t *parent, const char *title_name, SPRITE_SET_PAGE_LIST page)
{
    lv_obj_set_size(parent, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_radius(parent, 0, 0);
    lv_obj_set_style_bg_color(parent, lv_color_hex(COLOUR_GREY_2F), LV_PART_MAIN);

    lv_obj_t *btn_set_tittle = lv_btn_create(parent);
    lv_obj_remove_style_all(btn_set_tittle);
    lv_obj_align(btn_set_tittle, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_size(btn_set_tittle, 350, 60);

    lv_obj_t *label_title = lv_label_create(btn_set_tittle);
    lv_obj_set_style_text_color(label_title, lv_color_white(), 0);
    lv_obj_set_style_text_font(label_title, &SourceHanSansCN_Normal_20, 0);
    lv_label_set_text(label_title, title_name);
    lv_obj_center(label_title);

    lv_obj_t *btn_set_back = lv_btn_create(parent);
    lv_obj_align(btn_set_back, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_size(btn_set_back, 100, 60);
    lv_obj_set_style_bg_color(btn_set_back, lv_color_hex(COLOUR_GREY_2F), LV_PART_MAIN);
    lv_obj_add_event_cb(btn_set_back, btn_back_event_cb, LV_EVENT_SHORT_CLICKED, (void *)page);

    lv_obj_t *labelReturn = lv_label_create(btn_set_back);
    lv_obj_remove_style_all(labelReturn);
    lv_obj_set_style_text_color(labelReturn, lv_color_white(), 0);
    lv_obj_set_style_text_font(labelReturn, &lv_font_montserrat_16, 0);
    lv_label_set_text(labelReturn, LV_SYMBOL_LEFT);
    lv_obj_center(labelReturn);
}

static void lv_create_set_home(lv_obj_t *set_background)
{
    uint8_t focused = SPRITE_SET_PAGE_HOME;
    lv_create_set_title(set_background, sprite_create_list[focused].set_list_name[sys_set->sr_lang], focused); /*0-60*/

    /*Add buttons to the list*/
    lv_obj_t *btn, *label_enter, *label_item;

    lv_obj_t *list_select = create_button_list_page(set_background); /*LV_PCT(90) 70 Top*/

    for (int i = SPRITE_SET_PAGE_WiFi; i <= SPRITE_SET_PAGE_ABOUTUS + 1; i++) {

        btn = lv_list_add_btn(list_select, NULL, NULL);
        lv_obj_remove_style_all(btn);
        set_button_list_style(btn);
        if (i > SPRITE_SET_PAGE_ABOUTUS) {
            lv_obj_set_style_border_side(btn, LV_BORDER_SIDE_NONE, 0);
            continue;
        }
        lv_obj_add_event_cb(btn, home_list_event_handler, LV_EVENT_SHORT_CLICKED, (void *)i);

        label_item = lv_label_create(btn);
        lv_obj_set_style_text_font(label_item, &SourceHanSansCN_Normal_20, 0);
        lv_label_set_text(label_item, sprite_create_list[i].set_list_name[sys_set->sr_lang]);
        lv_obj_align(label_item, LV_ALIGN_LEFT_MID, 0, 0);

        label_enter = lv_label_create(btn);
        lv_obj_set_style_text_font(label_enter, &lv_font_montserrat_16, 0);
        lv_label_set_text(label_enter, LV_SYMBOL_RIGHT);
        lv_obj_align(label_enter, LV_ALIGN_RIGHT_MID, 0, 0);
    }
    return;
}

static void lv_create_wifi_keyboard(lv_obj_t *set_background)
{
    lv_create_set_title(set_background, (char *)scan_info_result.ap_info[ssid_list_select].ssid, SPRITE_SET_WIFI_KEYBOARD);

    /*Create a keyboard to use it with an of the text areas*/
    lv_obj_t *kb = lv_keyboard_create(set_background);
    lv_obj_set_width(kb, LV_PCT(95));

    /*Create a text area. The keyboard will write here*/
    lv_obj_t *ta;
    ta = lv_textarea_create(set_background);
    lv_obj_align(ta, LV_ALIGN_TOP_MID, 0, 120);
    lv_obj_set_size(ta, LV_PCT(95), LV_PCT(15));
    lv_obj_set_style_text_font(ta, &SourceHanSansCN_Normal_20, 0);
    lv_obj_add_event_cb(ta, ta_event_cb, LV_EVENT_ALL, kb);

    if (0 == strcmp((char *)scan_info_result.ap_info[ssid_list_select].ssid, (char *)sys_set->ssid)) {
        lv_textarea_set_placeholder_text(ta, (const char *)sys_set->password);
    } else {
        if (SR_LANG_CN == sys_set->sr_lang) {
            lv_textarea_set_placeholder_text(ta, "请输入密码");
        } else {
            lv_textarea_set_placeholder_text(ta, "Please input passwd");
        }
    }
    lv_keyboard_set_textarea(kb, ta);
}

static void lv_create_display_language(lv_obj_t *set_background)
{
    uint8_t focused = SPRITE_SET_DISPLAY_LANGUAGE;
    lv_create_set_title(set_background, sprite_create_list[focused].set_list_name[sys_set->sr_lang], focused); /*0-60*/

    lv_obj_t *btn, *label_item;
    lv_obj_t *list_select = create_button_list_page(set_background); /*LV_PCT(90) 70 Top*/

    // static uint32_t active_index_1;
    static lv_obj_t *language_checkbox[4];
    lv_obj_add_event_cb(list_select, language_event_handler, LV_EVENT_CLICKED, &language_checkbox[0]);

    for (int i = 0; i < 2; i++) {

        btn = lv_list_add_btn(list_select, NULL, NULL);
        lv_obj_remove_style_all(btn);
        set_button_list_style(btn);
        lv_obj_add_flag(btn, LV_OBJ_FLAG_EVENT_BUBBLE);

        label_item = lv_label_create(btn);
        lv_obj_set_style_text_color(label_item, lv_color_hex(COLOUR_WHITE), 0);
        lv_obj_set_style_text_font(label_item, &SourceHanSansCN_Normal_20, 0);
        if (0 == i) {
            lv_label_set_text(label_item, "EN");
        } else {
            lv_label_set_text(label_item, "中文");
        }
        lv_obj_align(label_item, LV_ALIGN_LEFT_MID, 0, 0);

        lv_obj_t *obj = lv_checkbox_create(btn);
        lv_checkbox_set_text(obj, " ");
        lv_obj_add_flag(obj, LV_OBJ_FLAG_EVENT_BUBBLE);
        lv_obj_align(obj, LV_ALIGN_RIGHT_MID, 0, 0);

        language_checkbox[i] = obj;
    }
    lv_obj_add_state(language_checkbox[sys_set->sr_lang], LV_STATE_CHECKED);

    return;
}

static void lv_create_display_reboot(lv_obj_t *set_background)
{
    lv_obj_set_size(set_background, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_radius(set_background, 0, 0);
    lv_obj_set_style_bg_color(set_background, lv_color_hex(COLOUR_GREY_2F), LV_PART_MAIN);

    reboot_wait_time = 5;
    label_reboot = lv_label_create(set_background);
    lv_obj_align(label_reboot, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(label_reboot, lv_color_hex(COLOUR_WHITE), 0);
    lv_obj_set_style_text_font(label_reboot, &SourceHanSansCN_Normal_20, 0);
    lv_label_set_text(label_reboot, "Rebooting");

    return;
}

static void lv_create_display_standby(lv_obj_t *set_background)
{
    uint8_t focused = SPRITE_SET_DISPLAY_STANDBY;
    lv_create_set_title(set_background, sprite_create_list[focused].set_list_name[sys_set->sr_lang], focused); /*0-60*/

    lv_obj_t *btn, *label_item;
    lv_obj_t *list_select = create_button_list_page(set_background); /*LV_PCT(90) 70 Top*/

    // static uint32_t active_index_1;
    static lv_obj_t *btn_checkbox[4];

    lv_obj_add_event_cb(list_select, standby_event_handler, LV_EVENT_CLICKED, &btn_checkbox[0]);

    for (int i = 0; i < sizeof(standby_time_list_name[0]) / sizeof(standby_time_list_name[0][0]); i++) {

        btn = lv_list_add_btn(list_select, NULL, NULL);
        lv_obj_remove_style_all(btn);
        set_button_list_style(btn);
        lv_obj_add_flag(btn, LV_OBJ_FLAG_EVENT_BUBBLE);

        label_item = lv_label_create(btn);
        lv_obj_set_style_text_color(label_item, lv_color_hex(COLOUR_WHITE), 0);
        lv_obj_set_style_text_font(label_item, &SourceHanSansCN_Normal_20, 0);
        lv_label_set_text(label_item, standby_time_list_name[sys_set->sr_lang][i]);
        lv_obj_align(label_item, LV_ALIGN_LEFT_MID, 0, 0);

        lv_obj_t *obj = lv_checkbox_create(btn);
        lv_checkbox_set_text(obj, " ");
        lv_obj_add_flag(obj, LV_OBJ_FLAG_EVENT_BUBBLE);
        // lv_obj_add_style(obj, &style_radio, LV_PART_INDICATOR);
        // lv_obj_add_style(obj, &style_radio_chk, LV_PART_INDICATOR | LV_STATE_CHECKED);
        lv_obj_align(obj, LV_ALIGN_RIGHT_MID, 0, 0);

        btn_checkbox[i] = obj;
    }
    lv_obj_add_state(btn_checkbox[sys_set->standby_time], LV_STATE_CHECKED);

    return;
}

static void update_reboot_step(lv_obj_t *parent)
{
    if (NULL != parent) {
        feed_clock_time();

        if (reboot_wait_time) {
            char reboot_info[30] = {0};
            sprintf(reboot_info, "Rebooting  %d sec", reboot_wait_time);
            reboot_wait_time--;
            lv_label_set_text(parent, reboot_info);
        } else {
            esp_restart();
        }
    }

    return;
}

static void update_wifi_list(lv_obj_t *parent)
{
    lv_obj_t *btn, *label_item;

    if (NULL == parent) {
        return;
    }

    app_wifi_lock(0);

    if (WIFI_SCAN_BUSY == scan_info_result.scan_done) {
        if (false == lv_obj_has_flag(parent, LV_OBJ_FLAG_HIDDEN)) {
            lv_obj_add_flag(parent, LV_OBJ_FLAG_HIDDEN);
        }

        if (lv_obj_get_child_cnt(parent)) {
            lv_obj_clean(parent);
        }
    } else if ((WIFI_SCAN_RENEW == scan_info_result.scan_done) || (WIFI_SCAN_UPDATE == scan_info_result.scan_done)) {
        if (true == lv_obj_has_flag(parent, LV_OBJ_FLAG_HIDDEN)) {
            lv_obj_clear_flag(parent, LV_OBJ_FLAG_HIDDEN);
        }

        if (WIFI_SCAN_UPDATE == scan_info_result.scan_done) {
            if (lv_obj_get_child_cnt(parent)) {
                lv_obj_clean(parent);
            }
        }

        ESP_LOGI(TAG, "update wifi list");

        for (int i = 0; i < scan_info_result.ap_count; i++) {
            if ((0 == strcmp((char *)sys_set->ssid, (char *)scan_info_result.ap_info[i].ssid)) && (i > 0)) {
                uint8_t replace_ssid[32] = {0};
                memcpy(replace_ssid, scan_info_result.ap_info[0].ssid, sizeof(replace_ssid));
                memcpy(scan_info_result.ap_info[0].ssid,
                       scan_info_result.ap_info[i].ssid, sizeof(replace_ssid));
                memcpy(scan_info_result.ap_info[i].ssid, replace_ssid, sizeof(replace_ssid));
                ESP_LOGI(TAG, "replace[%d->0:%s]", i, scan_info_result.ap_info[i].ssid);
            }
        }

        for (int i = 0; i < scan_info_result.ap_count + 1; i++) {

            btn = lv_list_add_btn(parent, NULL, NULL);
            lv_obj_remove_style_all(btn);
            set_button_list_style(btn);
            lv_obj_set_size(btn, LV_PCT(95), 60);

            if (i >= scan_info_result.ap_count) {
                lv_obj_set_style_border_side(btn, LV_BORDER_SIDE_NONE, 0);
                continue;
            }
            lv_obj_add_event_cb(btn, wifi_list_event_handler, LV_EVENT_SHORT_CLICKED, (void *)i);

            label_item = lv_label_create(btn);
            lv_obj_set_style_text_font(label_item, &SourceHanSansCN_Normal_20, 0);

            if (0 == strcmp((char *)scan_info_result.ap_info[i].ssid, (char *)sys_set->ssid)) {

                char ssid_conected[32 + 10];
                memset(ssid_conected, 0, sizeof(ssid_conected));

                WiFi_Connect_Status status = wifi_connected_already();
                if (WIFI_STATUS_CONNECTING == status) {
                    sprintf(ssid_conected, "%s %s", scan_info_result.ap_info[i].ssid,
                            (SR_LANG_CN == sys_set->sr_lang) ? "连接中" : "Connecting");
                } else if (WIFI_STATUS_CONNECTED_FAILED == status) {
                    sprintf(ssid_conected, "%s %s", scan_info_result.ap_info[i].ssid,
                            (SR_LANG_CN == sys_set->sr_lang) ? "未连接" : "Disconnected");
                } else {
                    sprintf(ssid_conected, "%s %s", scan_info_result.ap_info[i].ssid,
                            (SR_LANG_CN == sys_set->sr_lang) ? "已连接" : "Connected");
                }

                lv_label_set_text(label_item, ssid_conected);
                lv_obj_set_style_text_color(label_item, lv_color_hex(COLOUR_YELLOW), 0);
            } else {
                lv_label_set_text(label_item, (char *)scan_info_result.ap_info[i].ssid);
            }
            lv_obj_align(label_item, LV_ALIGN_LEFT_MID, 0, 0);

            lv_obj_t *label_enter = lv_label_create(btn);
            lv_obj_set_style_text_font(label_enter, &lv_font_montserrat_16, 0);
            lv_label_set_text(label_enter, LV_SYMBOL_RIGHT);
            lv_obj_align(label_enter, LV_ALIGN_RIGHT_MID, 0, 0);
        }
    }

    app_wifi_unlock();

    if ((WIFI_SCAN_RENEW == scan_info_result.scan_done) || (WIFI_SCAN_UPDATE == scan_info_result.scan_done)) {
        app_wifi_state_set(WIFI_SCAN_IDLE);
    }
}

static void lv_create_set_wifi(lv_obj_t *set_background)
{
    lv_obj_t *label_refresh;

    uint8_t focused = SPRITE_SET_PAGE_WiFi;
    lv_create_set_title(set_background, sprite_create_list[focused].set_list_name[sys_set->sr_lang], focused); /*0-60*/

    lv_obj_t *btn_refresh = lv_btn_create(set_background);
    lv_obj_align(btn_refresh, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_obj_set_size(btn_refresh, 100, 60);
    lv_obj_set_style_border_width(btn_refresh, 0, 0);
    lv_obj_set_style_bg_color(btn_refresh, lv_color_hex(COLOUR_GREY_2F), LV_PART_MAIN);
    lv_obj_add_event_cb(btn_refresh, wifi_refresh_event_handler, LV_EVENT_SHORT_CLICKED, NULL);

    label_refresh = lv_label_create(btn_refresh);
    lv_obj_remove_style_all(label_refresh);
    lv_obj_set_style_text_color(label_refresh, lv_color_white(), 0);
    lv_obj_set_style_text_font(label_refresh, &lv_font_montserrat_16, 0);
    lv_label_set_text(label_refresh, LV_SYMBOL_REFRESH);
    lv_obj_center(label_refresh);

    app_wifi_state_set(WIFI_SCAN_RENEW);
    list_wifi = create_button_list_page(set_background);
    update_wifi_list(list_wifi);
}

void create_sound_volume(lv_obj_t *parent)
{
    static uint32_t active_index_1 = 0;

    /* The idea is to enable `LV_OBJ_FLAG_EVENT_BUBBLE` on checkboxes and process the
     * `LV_EVENT_CLICKED` on the container.
     * A variable is passed as event user data where the index of the active
     * radiobutton is saved */

    uint32_t i;

    lv_obj_t *cont_volume = lv_obj_create(parent);
    lv_obj_set_flex_flow(cont_volume, LV_FLEX_FLOW_ROW);
    lv_obj_set_size(cont_volume, lv_pct(75), lv_pct(100));
    lv_obj_add_event_cb(cont_volume, volume_event_handler, LV_EVENT_CLICKED, &active_index_1);

    lv_obj_center(cont_volume);
    lv_obj_set_style_bg_color(cont_volume, lv_color_hex(COLOUR_GREY_2F), LV_PART_MAIN);
    lv_obj_set_style_border_width(cont_volume, 0, 0);
    lv_obj_set_style_bg_opa(cont_volume, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cont_volume, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
    lv_obj_clear_flag(cont_volume, LV_OBJ_FLAG_SCROLLABLE);

    for (i = 0; i < 5; i++) {
        lv_obj_t *obj = lv_checkbox_create(cont_volume);
        lv_checkbox_set_text(obj, " ");
        lv_obj_add_flag(obj, LV_OBJ_FLAG_EVENT_BUBBLE);
        // lv_obj_add_style(obj, &style_radio, LV_PART_INDICATOR);
        // lv_obj_add_style(obj, &style_radio_chk, LV_PART_INDICATOR | LV_STATE_CHECKED);
    }
    /*Make the first checkbox checked*/
    if ((sys_set->volume < 6) && (sys_set->volume > 0)) {
        lv_obj_add_state(lv_obj_get_child(cont_volume, sys_set->volume - 1), LV_STATE_CHECKED);
        active_index_1 = sys_set->volume - 1;
    } else {
        lv_obj_add_state(lv_obj_get_child(cont_volume, 5 - 1), LV_STATE_CHECKED);
        active_index_1 = 4;
    }
}

void create_bright_slider(lv_obj_t *parent)
{
    /*Create a slider and add the style*/
    lv_obj_t *slider = lv_slider_create(parent);
    lv_obj_remove_style_all(slider); /*Remove the styles coming from the theme*/
    lv_obj_set_size(slider, lv_pct(80), 10);
    lv_obj_center(slider);

    lv_obj_add_style(slider, &style_set_main, LV_PART_MAIN);
    lv_obj_add_style(slider, &style_set_indicator, LV_PART_INDICATOR); // click
    lv_obj_add_style(slider, &style_set_pressed_color, LV_PART_INDICATOR | LV_STATE_PRESSED);
    lv_obj_add_style(slider, &style_set_pressed_color, LV_PART_KNOB | LV_STATE_PRESSED);

    lv_obj_add_event_cb(slider, slider_show_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_slider_set_value(slider, sys_set->brightness, LV_ANIM_OFF);
    // lv_slider_set_range(slider,5,100);
}

static void lv_create_set_voice(lv_obj_t *set_background)
{
    uint8_t focused = SPRITE_SET_PAGE_VOICE;
    lv_create_set_title(set_background, sprite_create_list[focused].set_list_name[sys_set->sr_lang], focused); /*0-60*/

    /*Add buttons to the list*/
    lv_obj_t *btn, *label_item;

    lv_obj_t *list_select = create_button_list_page(set_background);
    lv_obj_set_size(list_select, LV_PCT(95), LV_PCT(90));
    /*
    Title
    */
    btn = lv_list_add_btn(list_select, NULL, NULL);
    lv_obj_remove_style_all(btn);
    set_button_list_style(btn);

    label_item = lv_label_create(btn);
    lv_obj_set_style_text_font(label_item, &SourceHanSansCN_Normal_20, 0);
    lv_label_set_text(label_item, (SR_LANG_CN == sys_set->sr_lang) ? "静音" : "Mute");
    lv_obj_align(label_item, LV_ALIGN_LEFT_MID, 0, 0);

    lv_obj_t *sw = lv_switch_create(btn);
    lv_obj_add_state(sw, LV_STATE_CHECKED);
    lv_obj_align(sw, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_add_event_cb(sw, mute_event_handler, LV_EVENT_ALL, NULL);
    /*
    slider
    */
    btn = lv_list_add_btn(list_select, NULL, NULL);
    lv_obj_remove_style_all(btn);
    set_button_list_style(btn);

    lv_obj_t *label_left = lv_label_create(btn);
    lv_obj_set_style_text_font(label_left, &lv_font_montserrat_16, 0);
    lv_label_set_text(label_left, LV_SYMBOL_VOLUME_MID);
    lv_obj_align(label_left, LV_ALIGN_LEFT_MID, 0, 0);

    lv_obj_t *label_right = lv_label_create(btn);
    lv_obj_set_style_text_font(label_right, &lv_font_montserrat_16, 0);
    lv_label_set_text(label_right, LV_SYMBOL_VOLUME_MAX);
    lv_obj_align(label_right, LV_ALIGN_RIGHT_MID, 0, 0);

    create_sound_volume(btn);
}

static void lv_create_set_bright(lv_obj_t *set_background)
{
    uint8_t focused = SPRITE_SET_PAGE_BRIGHT;
    lv_create_set_title(set_background, sprite_create_list[focused].set_list_name[sys_set->sr_lang], focused); /*0-60*/

    /*Add buttons to the list*/
    lv_obj_t *btn, *label_enter, *label_item;

    lv_obj_t *list_select = create_button_list_page(set_background);
    lv_obj_set_size(list_select, LV_PCT(95), LV_PCT(90));
    /*
    Title
    */
    btn = lv_list_add_btn(list_select, NULL, NULL);
    lv_obj_remove_style_all(btn);
    set_button_list_style(btn);

    label_item = lv_label_create(btn);
    lv_obj_set_style_text_font(label_item, &SourceHanSansCN_Normal_20, 0);
    lv_label_set_text(label_item, standby_home_list_name[sys_set->sr_lang][0]);
    lv_obj_align(label_item, LV_ALIGN_LEFT_MID, 0, 0);

    /*
    slider
    */
    btn = lv_list_add_btn(list_select, NULL, NULL);
    lv_obj_remove_style_all(btn);
    set_button_list_style(btn);

    lv_obj_t *label_left = lv_label_create(btn);
    lv_obj_set_style_text_font(label_left, &lv_font_montserrat_16, 0);
    lv_label_set_text(label_left, LV_SYMBOL_MINUS);
    lv_obj_align(label_left, LV_ALIGN_LEFT_MID, 0, 0);

    lv_obj_t *label_right = lv_label_create(btn);
    lv_obj_set_style_text_font(label_right, &lv_font_montserrat_16, 0);
    lv_label_set_text(label_right, LV_SYMBOL_PLUS);
    lv_obj_align(label_right, LV_ALIGN_RIGHT_MID, 0, 0);

    create_bright_slider(btn);
    /*
    Empty
    */
    btn = lv_list_add_btn(list_select, NULL, NULL);
    lv_obj_remove_style_all(btn);
    // set_button_list_style(btn);
    lv_obj_set_size(btn, LV_PCT(100), 40);
    lv_obj_set_style_border_width(btn, 0, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(COLOUR_GREY_BF), LV_PART_MAIN);

    /*
    待机首页
    */
    btn = lv_list_add_btn(list_select, NULL, NULL);
    lv_obj_remove_style_all(btn);
    set_button_list_style(btn);
    lv_obj_add_event_cb(btn, display_event_handler, LV_EVENT_SHORT_CLICKED, 0);

    label_item = lv_label_create(btn);
    lv_obj_set_style_text_font(label_item, &SourceHanSansCN_Normal_20, 0);
    lv_label_set_text(label_item, standby_home_list_name[sys_set->sr_lang][1]);
    lv_obj_align(label_item, LV_ALIGN_LEFT_MID, 0, 0);

    label_enter = lv_label_create(btn);
    lv_obj_set_style_text_font(label_enter, &SourceHanSansCN_Normal_20, 0);
    lv_obj_set_style_text_color(label_enter, lv_color_hex(COLOUR_GREY_8F), LV_PART_MAIN);
    lv_label_set_text(label_enter, (SR_LANG_EN == sys_set->sr_lang) ? "EN" : "中文");
    lv_obj_align(label_enter, LV_ALIGN_LEFT_MID, 300, 0);

    label_enter = lv_label_create(btn);
    lv_obj_set_style_text_font(label_enter, &lv_font_montserrat_16, 0);
    lv_label_set_text(label_enter, LV_SYMBOL_RIGHT);
    lv_obj_align(label_enter, LV_ALIGN_RIGHT_MID, 0, 0);

    /*
    待机时间
    */
    btn = lv_list_add_btn(list_select, NULL, NULL);
    lv_obj_remove_style_all(btn);
    set_button_list_style(btn);
    lv_obj_add_event_cb(btn, display_event_handler, LV_EVENT_SHORT_CLICKED, (void *)1);

    label_item = lv_label_create(btn);
    lv_obj_set_style_text_font(label_item, &SourceHanSansCN_Normal_20, 0);
    lv_label_set_text(label_item, standby_home_list_name[sys_set->sr_lang][2]);
    lv_obj_align(label_item, LV_ALIGN_LEFT_MID, 0, 0);

    label_enter = lv_label_create(btn);
    lv_obj_set_style_text_font(label_enter, &SourceHanSansCN_Normal_20, 0);
    lv_obj_set_style_text_color(label_enter, lv_color_hex(COLOUR_GREY_8F), LV_PART_MAIN);
    lv_label_set_text(label_enter, standby_time_list_name[sys_set->sr_lang][sys_set->standby_time]);
    lv_obj_align(label_enter, LV_ALIGN_LEFT_MID, 300, 0);

    label_enter = lv_label_create(btn);
    lv_obj_set_style_text_font(label_enter, &lv_font_montserrat_16, 0);
    lv_label_set_text(label_enter, LV_SYMBOL_RIGHT);
    lv_obj_align(label_enter, LV_ALIGN_RIGHT_MID, 0, 0);
}

static void lv_create_set_factory_reset(lv_obj_t *set_background)
{
    uint8_t focused = SPRITE_SET_PAGE_Factory_RST;
    lv_create_set_title(set_background, sprite_create_list[focused].set_list_name[sys_set->sr_lang], focused); /*0-60*/

    lv_obj_t *label_tips;

    lv_obj_t *list_select = create_button_list_page(set_background);
    lv_obj_set_size(list_select, LV_PCT(95), LV_PCT(90));

    label_tips = lv_label_create(set_background);
    lv_obj_align(label_tips, LV_ALIGN_CENTER, 0, -80);
    lv_obj_set_style_text_color(label_tips, lv_color_hex(COLOUR_GREY_8F), LV_PART_MAIN);
    lv_obj_set_style_text_font(label_tips, &SourceHanSansCN_Normal_20, 0);
    if (SR_LANG_CN == sys_set->sr_lang) {
        lv_label_set_text(label_tips, "是否确认恢复出厂设置 ?");
    } else {
        lv_label_set_text(label_tips, "Are you sure to restore factory settings?");
    }

    lv_obj_t *btn_cancel = lv_btn_create(set_background);
    lv_obj_set_size(btn_cancel, 100, 50);
    lv_obj_align(btn_cancel, LV_ALIGN_CENTER, -100, 60);
    lv_obj_add_event_cb(btn_cancel, factory_reset_event_handler, LV_EVENT_SHORT_CLICKED, (void *)0);

    label_tips = lv_label_create(btn_cancel);
    lv_obj_align(label_tips, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(label_tips, lv_color_hex(COLOUR_WHITE), LV_PART_MAIN);
    lv_obj_set_style_text_font(label_tips, &SourceHanSansCN_Normal_20, 0);
    lv_label_set_text(label_tips, (SR_LANG_CN == sys_set->sr_lang) ? "否" : "No");

    lv_obj_t *btn_confirm = lv_btn_create(set_background);
    lv_obj_set_size(btn_confirm, 100, 50);
    lv_obj_align(btn_confirm, LV_ALIGN_CENTER, 100, 60);
    lv_obj_add_event_cb(btn_confirm, factory_reset_event_handler, LV_EVENT_SHORT_CLICKED, (void *)1);

    label_tips = lv_label_create(btn_confirm);
    lv_obj_align(label_tips, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(label_tips, lv_color_hex(COLOUR_WHITE), LV_PART_MAIN);
    lv_obj_set_style_text_font(label_tips, &SourceHanSansCN_Normal_20, 0);
    lv_label_set_text(label_tips, (SR_LANG_CN == sys_set->sr_lang) ? "是" : "Yes");
}

static void lv_create_set_aboutus(lv_obj_t *set_background)
{
    uint8_t focused = SPRITE_SET_PAGE_ABOUTUS;
    lv_create_set_title(set_background, sprite_create_list[focused].set_list_name[sys_set->sr_lang], focused); /*0-60*/
    /*Add buttons to the list*/
    lv_obj_t *btn, *label_enter, *label_item;

    lv_obj_t *list_select = create_button_list_page(set_background); /*LV_PCT(90) 70 Top*/

    uint8_t eth_mac[6];
    char text_data[20];
    esp_wifi_get_mac(WIFI_IF_STA, eth_mac);

    for (int i = 0; i < sizeof(aboutus_list_name[0]) / sizeof(aboutus_list_name[0][0]);) {

        btn = lv_list_add_btn(list_select, NULL, NULL);
        lv_obj_remove_style_all(btn);
        set_button_list_style(btn);

        label_item = lv_label_create(btn);
        lv_obj_set_style_text_color(label_item, lv_color_hex(COLOUR_WHITE), 0);
        lv_obj_set_style_text_font(label_item, &SourceHanSansCN_Normal_20, 0);
        lv_label_set_text(label_item, aboutus_list_name[sys_set->sr_lang][i + 0]);
        lv_obj_align(label_item, LV_ALIGN_LEFT_MID, 0, 0);

        label_enter = lv_label_create(btn);
        lv_obj_set_style_text_color(label_enter, lv_color_hex(0xAFAFAF), 0);
        lv_obj_set_style_text_font(label_enter, &SourceHanSansCN_Normal_20, 0);

        memset(text_data, 0, sizeof(text_data));
        switch (i) {
        case 0:
            lv_label_set_text(label_enter, "ESPRESSIF");
            break;
        case 1:
            lv_label_set_text(label_enter, "ESP32-S3-LCD-Ev-Board");
            break;
        case 2:
            lv_label_set_text(label_enter, "SUB2");
            break;
        case 3:
            snprintf(text_data, sizeof(text_data), "%02x%02x%02x%02x%02x%02x",
                     eth_mac[0], eth_mac[1], eth_mac[2],
                     eth_mac[3], eth_mac[4], eth_mac[5]);
            lv_label_set_text(label_enter, text_data);
            break;
        case 4:
            sprintf(text_data, "v%d.%d.%d_%s", \
                    EXAMPLE_VERSION_MAJOR, \
                    EXAMPLE_VERSION_MINOR, \
                    EXAMPLE_VERSION_PATCH, \
                    __DATE__);
            lv_label_set_text(label_enter, text_data);
            break;
        case 5:
#if CONFIG_SR_MN_CN_MULTINET5_RECOGNITION_QUANT8 || CONFIG_SR_MN_EN_MULTINET5_SINGLE_RECOGNITION_QUANT8
            snprintf(text_data, sizeof(text_data), "%s", \
                     (SR_LANG_CN == sys_set->sr_lang) ? "mn5q8_cn" : "mn5q8_en");
#elif CONFIG_SR_MN_CN_MULTINET6_QUANT || CONFIG_SR_MN_EN_MULTINET6_QUANT
            snprintf(text_data, sizeof(text_data), "%s", \
                     (SR_LANG_CN == sys_set->sr_lang) ? "mn6_cn" : "mn6_en");
#endif
            lv_label_set_text(label_enter, text_data);
            break;
        }
        lv_obj_align(label_enter, LV_ALIGN_RIGHT_MID, 0, 0);
        i++;
    }
    return;
}

static bool set_layer_enter_cb(void *create_layer)
{
    bool ret = false;

    lv_layer_t *layer = create_layer;

    if (NULL == layer->lv_obj_layer) {
        ret = true;
        layer->lv_obj_layer = lv_obj_create(lv_scr_act());
        lv_obj_remove_style_all(layer->lv_obj_layer);
        lv_obj_set_size(layer->lv_obj_layer, LV_HOR_RES, LV_VER_RES);
        sprite_focus_page_goto(layer->user_data);
    }
    set_time_out(&time_1000ms, 1000);

    return ret;
}

static bool set_layer_exit_cb(void *create_layer)
{
    sprite_focus_page = SPRITE_SET_PAGE_MAX;
    for (int i = 0; i < SPRITE_SET_PAGE_MAX; i++) {
        if (sprite_create_list[i].sprite_parent) {
            sprite_create_list[i].sprite_parent = NULL;
        }
    }
    return true;
}

static void set_layer_timer_cb(lv_timer_t *tmr)
{
    update_wifi_list(list_wifi);

    if (is_time_out(&time_1000ms)) {
        update_reboot_step(label_reboot);
    }
}
