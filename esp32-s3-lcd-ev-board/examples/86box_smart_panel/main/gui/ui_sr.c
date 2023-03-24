/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include "esp_log.h"
#include "bsp/esp-bsp.h"

#include "lv_example_pub.h"
#include "lv_example_image.h"
#include "ui_sr.h"
#include "settings.h"

static const char *TAG = "ui_sr";

#define LV_SR_USE_GIF              0

static bool g_voice_wakeuped = false;
static int32_t g_sr_anim_count = 0;
static lv_obj_t *g_sr_label = NULL;
static lv_obj_t *g_sr_bar[8] = {NULL};

static bool wakeup_changed = false;
static char wakeup_text[30] = {0};

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

static void sr_label_event_handler(lv_event_t *event)
{
    char *text = (char *) event->param;
    ESP_LOGW(TAG, "sr_label_event_handler,%d,%s", strlen(text), text);
    // if (NULL != text) {
    //     lv_label_set_text(g_sr_label, text);
    // }
    // else{
    //     ESP_LOGW(TAG, "text NULL");
    // }
}

static void sr_mask_event_handler(lv_event_t *event)
{
    bool active = (bool) event->param;

    if (active) {
        lv_indev_t *indev = lv_indev_get_next(NULL);
        lv_indev_enable(indev, false);
        g_sr_anim_count = 0;
    } else {
        lv_indev_t *indev = lv_indev_get_next(NULL);
        lv_indev_enable(indev, true);
    }
}

void sr_speech_anim_handle(void)
{
    const int32_t step = 40;

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
}

lv_obj_t *ui_sr_anim_init(lv_obj_t *parent)
{
    const sys_param_t *param = settings_get_parameter();

    lv_obj_t *sr_mask = lv_obj_create(parent);
    lv_obj_set_size(sr_mask, lv_obj_get_width(lv_obj_get_parent(sr_mask)), lv_obj_get_height(lv_obj_get_parent(sr_mask)));
    lv_obj_clear_flag(sr_mask, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_radius(sr_mask, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(sr_mask, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(sr_mask, lv_obj_get_style_bg_color(lv_obj_get_parent(sr_mask), LV_PART_MAIN), LV_STATE_DEFAULT);
    lv_obj_align(sr_mask, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(sr_mask, sr_mask_event_handler, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_t *obj_img = NULL;
    obj_img = lv_obj_create(sr_mask);
    lv_obj_set_size(obj_img, 80, 80);
    lv_obj_clear_flag(obj_img, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(obj_img, 40, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(obj_img, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(obj_img, 40, LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(obj_img, LV_OPA_30, LV_STATE_DEFAULT);
    lv_obj_align(obj_img, LV_ALIGN_CENTER, 0, -30);
    lv_obj_t *img_mic_logo = lv_img_create(obj_img);
    LV_IMG_DECLARE(mic_logo)
    lv_img_set_src(img_mic_logo, &mic_logo);
    lv_obj_center(img_mic_logo);

    g_sr_label = lv_label_create(sr_mask);
    if (SR_LANG_EN == param->sr_lang) {
        lv_label_set_text_static(g_sr_label, "Say command");
    } else {
        lv_label_set_text_static(g_sr_label, "请说");
    }
    lv_obj_set_style_text_font(g_sr_label, font36.font, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(g_sr_label, lv_color_black(), LV_STATE_DEFAULT);
    lv_obj_align(g_sr_label, LV_ALIGN_CENTER, 0, 80);
    lv_obj_add_event_cb(g_sr_label, sr_label_event_handler, LV_EVENT_VALUE_CHANGED, NULL);

    for (size_t i = 0; i < sizeof(g_sr_bar) / sizeof(g_sr_bar[0]); i++) {
        g_sr_bar[i] = lv_bar_create(sr_mask);
        lv_obj_set_size(g_sr_bar[i], 5, 60);
        lv_obj_set_style_anim_time(g_sr_bar[i], 400, LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(g_sr_bar[i], lv_color_make(237, 238, 239), LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(g_sr_bar[i], lv_color_make(246, 175, 171), LV_PART_INDICATOR);
        lv_bar_set_range(g_sr_bar[i], -100, 100);
        lv_bar_set_value(g_sr_bar[i], 20, LV_ANIM_OFF);
        lv_bar_set_start_value(g_sr_bar[i], -20, LV_ANIM_OFF);
        lv_obj_set_style_anim_time(g_sr_bar[i], 400, LV_STATE_DEFAULT);

    }

    for (size_t i = 0; i < sizeof(g_sr_bar) / sizeof(g_sr_bar[0]) / 2; i++) {
        lv_obj_align_to(g_sr_bar[i], obj_img, LV_ALIGN_OUT_LEFT_MID, 15 * i - 65, 0);
        lv_obj_align_to(g_sr_bar[i + 4], obj_img, LV_ALIGN_OUT_RIGHT_MID, 15 * i + 20, 0);
    }

    g_sr_anim_count = 0;

    return sr_mask;
}

void sr_anim_start(void)
{
    g_voice_wakeuped = true;
}

void sr_anim_stop(void)
{
    g_voice_wakeuped = false;
}

void sr_anim_set_text(char *text)
{
    wakeup_changed = true;
    strcpy(wakeup_text, text);
}

anim_handle_cb sr_anim_task(lv_layer_t *layer)
{
    static bool local_voice_wakeuped = true;

#if LV_SR_USE_GIF
    static lv_obj_t *gif_listening;
#else
    static lv_obj_t *box_listening;
#endif

    if (wakeup_changed) {
        wakeup_changed = false;
        if (g_sr_label) {
            lv_label_set_text(g_sr_label, wakeup_text);
            ESP_LOGW(TAG, "g_sr_label set");
        } else {
            ESP_LOGW(TAG, "text NULL");
        }
    }

    if (g_voice_wakeuped ^ local_voice_wakeuped) {

        local_voice_wakeuped = g_voice_wakeuped;
        ESP_LOGI(TAG, "## voice_wakeuped: %d", g_voice_wakeuped);
        if (g_voice_wakeuped) {
#if LV_SR_USE_GIF
            gif_listening = lv_gif_create(layer->lv_obj_layer);
            //lv_gif_set_src(gif_listening, "S:/spiffs/test_gif.gif");
            lv_gif_set_src(gif_listening, &test_gif);
            lv_obj_align(gif_listening, LV_ALIGN_CENTER, 0, 0);
#else
            box_listening = ui_sr_anim_init(layer->lv_obj_layer);
#endif
            lv_obj_clear_flag(layer->lv_obj_layer, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(layer->lv_obj_parent, LV_OBJ_FLAG_HIDDEN);
        } else {
#if LV_SR_USE_GIF
            if (gif_listening) {
                lv_obj_del_async(gif_listening);
                gif_listening = NULL;
            }
#else
            if (box_listening) {
                lv_obj_del_async(box_listening);
                box_listening = NULL;
            }
#endif
            lv_obj_add_flag(layer->lv_obj_layer, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(layer->lv_obj_parent, LV_OBJ_FLAG_HIDDEN);
            g_sr_label = NULL;
        }
    }

#if LV_SR_USE_GIF
    return NULL;
#else
    if (local_voice_wakeuped) {
        return sr_speech_anim_handle;
    } else {
        return NULL;
    }
#endif
}
