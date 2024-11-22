/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "esp_log.h"
#include "esp_check.h"

#include "app_sr_handler.h"
#include "ui_main.h"
#include "bsp_board_extra.h"
#include "settings.h"

static const char *TAG = "GUIDE";

LV_IMG_DECLARE(hand_down);
LV_IMG_DECLARE(mic_logo);

static void ui_sr_timer_cb(lv_task_t *task);

/* UI function declaration */
ui_func_desc_t ui_sr_func = {
    .name = "ui_sr",
    .init = ui_sr_init,
    .show = ui_sr_show,
    .hide = NULL,
};

sys_param_t *sys_set;

static bool g_sr_anim_active = false;
static int32_t g_sr_anim_count = 0;
static lv_obj_t *g_sr_label = NULL;
static lv_obj_t *g_sr_mask = NULL;
static lv_obj_t *g_sr_bar[8] = {NULL};

static uint8_t reboot_wait_time = 5;
static lv_obj_t *bg_factory_reset;

static int int16_sin(int32_t deg)
{
    static const int16_t sin_0_90_table[] = {
        0,     572,   1144,  1715,  2286,  2856,  3425,  3993,  4560,  5126,  5690,  6252,  6813,  7371,  7927,  8481,
        9032,  9580,  10126, 10668, 11207, 11743, 12275, 12803, 13328, 13848, 14364, 14876, 15383, 15886, 16383, 16876,
        17364, 17846, 18323, 18794, 19260, 19720, 20173, 20621, 21062, 21497, 21925, 22347, 22762, 23170, 23571, 23964,
        24351, 24730, 25101, 25465, 25821, 26169, 26509, 26841, 27165, 27481, 27788, 28087, 28377, 28659, 28932, 29196,
        29451, 29697, 29934, 30162, 30381, 30591, 30791, 30982, 31163, 31335, 31498, 31650, 31794, 31927, 32051, 32165,
        32269, 32364, 32448, 32523, 32587, 32642, 32687, 32722, 32747, 32762, 32767
    };

    if (deg < 0) {
        deg = -deg;
    }

    deg = deg % 360;

    if (deg <= 90) {
        return sin_0_90_table[deg];
    } else if (deg <= 180) {
        return sin_0_90_table[180 - deg];
    } else if (deg <= 270) {
        return -sin_0_90_table[deg - 270];
    } else {
        return -sin_0_90_table[360 - deg];
    }
}

void sr_anim_start(void)
{
    ESP_LOGI(TAG, "animation start");
    g_sr_anim_count = 0;
    g_sr_anim_active = true;
}

void sr_anim_stop(void)
{
    ESP_LOGI(TAG, "animation stop");
    g_sr_anim_active = false;
}

static void set_tips_info(const char *message)
{
    if (g_sr_label) {
        lv_label_set_text(g_sr_label, message);
        lv_label_set_align(g_sr_label, LV_LABEL_ALIGN_CENTER);
        lv_obj_align(g_sr_label, NULL, LV_ALIGN_CENTER, 0, 80);
    }
}

static void factory_reset_event_handler(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_SHORT_CLICKED) {
        ESP_LOGI(TAG, "factory reset");
        settings_factory_reset();
        lv_obj_set_hidden(bg_factory_reset, false);
    }
}

static void ui_speech_anim_cb()
{
    const int32_t step = 40;

    if (g_sr_anim_active) {
        /* Set bar value */
        int32_t sr_val[4] = {
            abs(int16_sin(g_sr_anim_count * step + step * 0)) / (32768 >> 7),
            abs(int16_sin(g_sr_anim_count * step + step * 1)) / (32768 >> 7),
            abs(int16_sin(g_sr_anim_count * step + step * 2)) / (32768 >> 7),
            abs(int16_sin(g_sr_anim_count * step + step * 3)) / (32768 >> 7),
        };

        for (size_t i = 0; i < 4; i++) {
            lv_bar_set_value(g_sr_bar[i], sr_val[i] > 20 ? sr_val[i] : 20, LV_ANIM_ON);
            lv_bar_set_value(g_sr_bar[7 - i], sr_val[i] > 20 ? sr_val[i] : 20, LV_ANIM_ON);
            lv_bar_set_start_value(g_sr_bar[i], sr_val[i] > 20 ? -sr_val[i] : -20, LV_ANIM_ON);
            lv_bar_set_start_value(g_sr_bar[7 - i], sr_val[i] > 20 ? -sr_val[i] : -20, LV_ANIM_ON);
        }
        g_sr_anim_count++;
    } else {
        /* The first timer callback will set the bars to 0 */
        if (g_sr_anim_count != 0) {
            for (size_t i = 0; i < 8; i++) {
                lv_bar_set_value(g_sr_bar[i], 0, LV_ANIM_ON);
                lv_bar_set_start_value(g_sr_bar[i], -0, LV_ANIM_ON);
            }
            g_sr_anim_count = 0;
        } else {
            /* The second timer callback will hide sr mask */
        }
    }
}

static void ui_create_reboot()
{
    bg_factory_reset = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(bg_factory_reset, LV_HOR_RES, LV_VER_RES);
    lv_obj_align(bg_factory_reset, NULL, LV_ALIGN_CENTER, 0, 0);

    reboot_wait_time = 50;
    lv_obj_t *label_reboot = lv_label_create(bg_factory_reset, NULL);
    lv_obj_set_style_local_text_font(label_reboot, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_28);
    lv_label_set_text(label_reboot, "Rebooting  6 sec");
    lv_obj_align(label_reboot, bg_factory_reset, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_hidden(bg_factory_reset, true);

    return;
}

void ui_sr_anim_start()
{
    ESP_LOGI(TAG, "sr animation initialize");
    g_sr_mask = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(g_sr_mask, LV_HOR_RES, LV_VER_RES);
    lv_obj_align(g_sr_mask, NULL, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *obj_img = NULL;
    obj_img = lv_obj_create(g_sr_mask, NULL);
    lv_obj_set_size(obj_img, 80, 80);
    lv_obj_set_style_local_border_width(obj_img, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_outline_width(obj_img, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);

    lv_obj_align(obj_img, NULL, LV_ALIGN_CENTER, 0, -30);
    lv_obj_t *img_mic_logo = lv_img_create(obj_img, NULL);
    lv_img_set_src(img_mic_logo, &mic_logo);
    lv_obj_align(img_mic_logo, NULL, LV_ALIGN_CENTER, 0, 0);

    g_sr_label = lv_label_create(g_sr_mask, NULL);
    lv_obj_set_style_local_text_font(g_sr_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_24);
    lv_label_set_text(g_sr_label, ("Listening..."));
    lv_label_set_align(g_sr_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(g_sr_label, NULL, LV_ALIGN_CENTER, 0, 80);

    for (size_t i = 0; i < sizeof(g_sr_bar) / sizeof(g_sr_bar[0]); i++) {
        g_sr_bar[i] = lv_bar_create(g_sr_mask, NULL);
        lv_obj_set_size(g_sr_bar[i], 5, 60);
        lv_obj_set_style_local_bg_color(g_sr_bar[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(237, 238, 239));
        lv_obj_set_style_local_bg_color(g_sr_bar[i], LV_BAR_PART_INDIC, LV_STATE_DEFAULT, lv_color_make(246, 175, 171));
        lv_bar_set_range(g_sr_bar[i], -100, 100);
        lv_bar_set_value(g_sr_bar[i], 20, LV_ANIM_OFF);
        lv_bar_set_start_value(g_sr_bar[i], -20, LV_ANIM_OFF);
    }

    for (size_t i = 0; i < sizeof(g_sr_bar) / sizeof(g_sr_bar[0]) / 2; i++) {
        lv_obj_align(g_sr_bar[i], obj_img, LV_ALIGN_OUT_LEFT_MID, 15 * i - 65, 0);
        lv_obj_align(g_sr_bar[i + 4], obj_img, LV_ALIGN_OUT_RIGHT_MID, 15 * i + 20, 0);
    }

    lv_obj_t *lab_hint = lv_label_create(g_sr_mask, NULL);
    lv_obj_set_style_local_text_font(lab_hint, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_24);
    lv_label_set_text_static(lab_hint, ("Touch to enter factory reset"));
    lv_label_set_align(lab_hint, LV_LABEL_ALIGN_AUTO);
    lv_obj_align(lab_hint, NULL, LV_ALIGN_CENTER, 0, 140);

    lv_obj_t *btn_reset = lv_btn_create(g_sr_mask, NULL);
    lv_obj_set_style_local_border_width(btn_reset, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_border_width(btn_reset, LV_BTN_PART_MAIN, LV_STATE_FOCUSED, 0);
    lv_obj_set_style_local_outline_width(btn_reset, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_outline_width(btn_reset, LV_BTN_PART_MAIN, LV_STATE_FOCUSED, 0);
    lv_obj_set_size(btn_reset, 100, 75);
    lv_obj_align(btn_reset, NULL, LV_ALIGN_CENTER, 0, 200);
    lv_obj_set_event_cb(btn_reset, factory_reset_event_handler);

    lv_obj_t *img = lv_img_create(btn_reset, NULL);
    lv_img_set_src(img, &hand_down);
    lv_obj_align(img, NULL, LV_ALIGN_CENTER, 0, 0);

    ui_create_reboot();
    sr_anim_stop();

    lv_task_create(ui_sr_timer_cb, 100, 1, NULL);
}

static void ui_update_reboot_step(void)
{
    if (false == bg_factory_reset->hidden) {
        if (reboot_wait_time / 10) {

            char reboot_info[30] = {0};
            lv_obj_t *label_reboot = lv_obj_get_child(bg_factory_reset, 0);

            sprintf(reboot_info, "Rebooting  %d sec", reboot_wait_time / 10);
            reboot_wait_time--;
            lv_label_set_text(label_reboot, reboot_info);
        } else {
            bsp_extra_led_set_rgb(0, 0, 0, 0);
            esp_restart();
        }
    }
    return;
}

static void ui_sr_timer_cb(lv_task_t *task)
{
    lv_event_info_t lvgl_event;

    ui_speech_anim_cb();
    ui_update_reboot_step();

    if (pdPASS == app_lvgl_get_event(&lvgl_event, 0)) {
        switch (lvgl_event.event) {

        case LV_EVENT_I_AM_LEAVING:
            sr_anim_stop();
            set_tips_info("Listening...");
            break;
        case LV_EVENT_I_AM_HERE:
            sr_anim_start();
            set_tips_info("Say command");
            break;

        case LV_EVENT_LIGHT_ON:
            bsp_extra_led_set_rgb(0, 50, 50, 50);
            set_tips_info((const char *)lvgl_event.event_data);
            break;
        case LV_EVENT_LIGHT_OFF:
            bsp_extra_led_set_rgb(0, 0, 0, 0);
            set_tips_info((const char *)lvgl_event.event_data);
            break;
        case LV_EVENT_AC_SET_ON:
        case LV_EVENT_AC_SET_OFF:
        case LV_EVENT_AC_TEMP_ADD:
        case LV_EVENT_AC_TEMP_DEC:
            set_tips_info((const char *)lvgl_event.event_data);
            break;

        default:
        case LV_EVENT_EXIT_CLOCK:
            ESP_LOGI(TAG, "invalid event: %d", lvgl_event.event);
            break;
        }
    }
}

void ui_sr_init(void *data)
{
    (void)data;
    ui_sr_anim_start();
}

void ui_sr_show(void *data)
{
    sys_set = settings_get_parameter();
    ui_sr_init(data);
}
