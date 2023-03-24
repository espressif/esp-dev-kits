/**
 * @file ui_music.c
 * @brief Music example UI
 * @version 0.1
 * @date 2021-01-01
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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "ui_main.h"
#include "bsp_codec.h"
#include "app_player.h"
#include "file_manager.h"

static const char *TAG = "ui_music";

ui_func_desc_t ui_music_func = {
    .name = "Music",
    .init = ui_music_init,
    .show = ui_music_show,
    .hide = ui_music_hide,
};
extern bool sdcard_ok;
static ui_state_t ui_music_state = ui_state_dis;

static lv_obj_t *img_album = NULL;
static lv_obj_t *label_music_name = NULL;
static lv_obj_t *btn_prev = NULL;
static lv_obj_t *btn_play_pause = NULL;
static lv_obj_t *btn_next = NULL;
static lv_obj_t *label_music_time = NULL;
static lv_obj_t *label_music_length = NULL;
static lv_obj_t *btn_play_mode = NULL;
static lv_obj_t *btn_list = NULL;
static lv_obj_t *roller_lyric = NULL;
static lv_obj_t *slider_volume = NULL;
static lv_obj_t *obj_lyric = NULL;
static lv_obj_t *obj_control = NULL;
static lv_obj_t *slider_progress = NULL;
static lv_obj_t *list_music_file = NULL;

const char *lrc = NULL;
const uint32_t *lrc_ts = NULL;
uint32_t lrc_line_num = 0;

static uint32_t music_total_time = 0;

extern void *data_music_album[];

LV_FONT_DECLARE(font_20_cn_gb);

static const char *music_lyric[] = {
    "Music Lyric - Examples\n"
    "Lyrics by : Espressif\n"
    "Composed by : Espressif\n"
    "Music player example with lyric display\n"
    "You can place pcm files in SD card\n"
    "Example lyrics - 1\n"
    "Example lyrics - 2\n"
    "Example lyrics - 3\n"
    "Example lyrics - 4\n"
    "Example lyrics - 5\n"
    "Example lyrics - 6\n"
    "Example lyrics - 7",
};

static void lyric_task(lv_task_t *task);
static void slider_volume_cb(lv_obj_t *obj, lv_event_t event);
static void btn_play_pause_cb(lv_obj_t *obj, lv_event_t event);
static void btn_prev_next_cb(lv_obj_t *obj, lv_event_t event);
static void btn_list_cb(lv_obj_t *obj, lv_event_t event);
static void btn_file_list_cb(lv_obj_t *obj, lv_event_t event);

static void audio_cb(player_cb_ctx_t *ctx)
{
    if (AUDIO_EVENT_CHANGE == ctx->audio_event) {

    } else if (AUDIO_EVENT_PLAY_BRFORE == ctx->audio_event) {
        lv_port_sem_take();
        uint8_t name[256];
        StringGBK2UTF8(app_player_get_name_from_index(app_player_get_index()), name);
        lv_label_set_text(label_music_name, (char *)name);
        music_total_time = app_player_get_total_time() / 1000;
        lv_label_set_text_fmt(label_music_length, "%02d:%02d", music_total_time / 60, music_total_time % 60);

        app_player_get_current_lyric(&lrc, &lrc_ts, &lrc_line_num);
        if (lrc) {
            lv_roller_set_options(roller_lyric, lrc, LV_ROLLER_MODE_NORMAL);
        } else {
            lv_roller_set_options(roller_lyric, music_lyric[0], LV_ROLLER_MODE_NORMAL);
        }
        lv_port_sem_give();
    } else if (AUDIO_EVENT_PLAY == ctx->audio_event) {
        lv_port_sem_take();
        lv_obj_set_style_local_value_str(btn_play_pause, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_PAUSE);
        lv_port_sem_give();
    } else if (AUDIO_EVENT_FILE_SCAN_DONE == ctx->audio_event) {
        size_t list_len = 0;
        const char **p_audio_file_info = app_player_get_file_list(&list_len);
        lv_port_sem_take();
        for (size_t i = 0; i < list_len; i++) {
            uint8_t name[256];
            StringGBK2UTF8(p_audio_file_info[i], name);
            lv_obj_t *list_btn = lv_list_add_btn(list_music_file, LV_SYMBOL_AUDIO, (char *)name);
            lv_obj_set_event_cb(list_btn, btn_file_list_cb);
        }
        lv_port_sem_give();
    }
}

void ui_music_init(void *data)
{
    ESP_LOGW(TAG, "ui_music_init");
    img_album = lv_img_create(lv_scr_act(), NULL);
    lv_img_set_src(img_album, data_music_album[0]);
    lv_obj_set_style_local_radius(img_album, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, 10);
    lv_obj_set_pos(img_album, 25, 75);

    obj_lyric = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(obj_lyric, 465, 280);
    lv_obj_set_style_local_radius(obj_lyric, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 15);
    lv_obj_set_style_local_border_width(obj_lyric, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_align(obj_lyric, img_album, LV_ALIGN_OUT_RIGHT_TOP, 25, 0);

    slider_volume = lv_slider_create(lv_scr_act(), NULL);
    lv_obj_set_size(slider_volume, 200, 4);
    lv_slider_set_range(slider_volume, 0, 100);
    lv_slider_set_value(slider_volume, 70, LV_ANIM_ON);
    lv_obj_set_style_local_radius(slider_volume, LV_BAR_PART_BG, LV_STATE_DEFAULT, 15);
    lv_obj_align(slider_volume, img_album, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 20);
    lv_obj_set_style_local_value_str(slider_volume, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, LV_SYMBOL_VOLUME_MID);
    lv_obj_set_style_local_value_align(slider_volume, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, LV_ALIGN_OUT_LEFT_MID);
    lv_obj_set_style_local_value_font(slider_volume, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, &lv_font_montserrat_32);
    lv_obj_set_style_local_value_ofs_x(slider_volume, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, -20);
    lv_obj_set_style_local_value_color(slider_volume, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, COLOR_THEME);
    lv_obj_set_style_local_bg_color(slider_volume, LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, COLOR_THEME);
    lv_obj_set_event_cb(slider_volume, slider_volume_cb);
    lv_event_send(slider_volume, LV_EVENT_VALUE_CHANGED, NULL);

    obj_control = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_click(obj_control, false);
    lv_obj_set_style_local_border_width(obj_control, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_size(obj_control, 750, 80);
    lv_obj_align(obj_control, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -25);

    btn_prev = lv_btn_create(obj_control, NULL);
    lv_obj_set_size(btn_prev, 60, 60);
    lv_obj_set_style_local_radius(btn_prev, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 30);
    lv_obj_set_style_local_value_str(btn_prev, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_PREV);
    lv_obj_set_style_local_border_width(btn_prev, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_border_width(btn_prev, LV_BTN_PART_MAIN, LV_STATE_FOCUSED, 0);
    lv_obj_set_style_local_outline_width(btn_prev, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_outline_width(btn_prev, LV_BTN_PART_MAIN, LV_STATE_FOCUSED, 0);
    lv_obj_set_style_local_bg_color(btn_prev, LV_BTN_PART_MAIN, LV_STATE_PRESSED, COLOR_THEME);
    lv_obj_set_style_local_value_font(btn_prev, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_32);
    lv_obj_align(btn_prev, NULL, LV_ALIGN_IN_LEFT_MID, 25, 0);
    lv_obj_set_event_cb(btn_prev, btn_prev_next_cb);

    btn_play_pause = lv_btn_create(obj_control, btn_prev);
    if (PLAYER_STATE_PLAYING == app_player_get_state()) {
        lv_obj_set_style_local_value_str(btn_play_pause, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_PAUSE);
    } else {
        lv_obj_set_style_local_value_str(btn_play_pause, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_PLAY);
    }
    lv_obj_set_style_local_value_ofs_x(btn_play_pause, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 3);
    lv_obj_set_style_local_border_width(btn_play_pause, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 2);
    lv_obj_set_style_local_border_width(btn_play_pause, LV_BTN_PART_MAIN, LV_STATE_FOCUSED, 2);
    lv_obj_set_style_local_border_color(btn_play_pause, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_THEME);
    lv_obj_set_style_local_border_color(btn_play_pause, LV_BTN_PART_MAIN, LV_STATE_FOCUSED, COLOR_THEME);
    lv_obj_set_style_local_border_color(btn_play_pause, LV_BTN_PART_MAIN, LV_STATE_PRESSED, COLOR_THEME);
    lv_obj_set_style_local_value_align(btn_play_pause, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
    lv_obj_align(btn_play_pause, btn_prev, LV_ALIGN_OUT_RIGHT_MID, 25, 0);
    lv_obj_set_event_cb(btn_play_pause, btn_play_pause_cb);

    btn_next = lv_btn_create(obj_control, btn_prev);
    lv_obj_set_style_local_value_str(btn_next, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_NEXT);
    lv_obj_align(btn_next, btn_play_pause, LV_ALIGN_OUT_RIGHT_MID, 25, 0);
    lv_obj_set_event_cb(btn_next, btn_prev_next_cb);

    slider_progress = lv_slider_create(obj_control, NULL);
    lv_obj_set_size(slider_progress, 250, 4);
    lv_slider_set_range(slider_progress, 0, 100);
    lv_slider_set_value(slider_progress, 0, LV_ANIM_ON);
    lv_obj_set_style_local_bg_color(slider_progress, LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, COLOR_THEME);
    lv_obj_align(slider_progress, btn_next, LV_ALIGN_OUT_RIGHT_MID, 35, 0);

    label_music_time = lv_label_create(obj_control, NULL);
    lv_label_set_text(label_music_time, "00:00");
    lv_obj_set_style_local_text_font(label_music_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_20);
    lv_obj_align(label_music_time, slider_progress, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 6);

    label_music_length = lv_label_create(obj_control, NULL);
    lv_label_set_text(label_music_length, "00:00");
    lv_obj_set_style_local_text_font(label_music_length, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_20);
    lv_obj_align(label_music_length, slider_progress, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 6);

    btn_play_mode = lv_btn_create(obj_control, btn_prev);
    lv_obj_set_style_local_value_str(btn_play_mode, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_REFRESH);
    lv_obj_set_style_local_value_color(btn_play_mode, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_THEME);
    lv_obj_set_style_local_value_color(btn_play_mode, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_WHITE);
    lv_obj_align(btn_play_mode, slider_progress, LV_ALIGN_OUT_RIGHT_MID, 35, 0);

    btn_list = lv_btn_create(obj_control, btn_prev);
    lv_obj_set_style_local_value_str(btn_list, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_LIST);
    lv_obj_set_style_local_value_color(btn_list, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_THEME);
    lv_obj_set_style_local_value_color(btn_list, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_WHITE);
    lv_obj_align(btn_list, btn_play_mode, LV_ALIGN_OUT_RIGHT_MID, 25, 0);
    lv_obj_set_event_cb(btn_list, btn_list_cb);

    label_music_name = lv_label_create(obj_lyric, NULL);
    lv_label_set_text(label_music_name, "Music Player");
    lv_label_set_align(label_music_name, LV_LABEL_ALIGN_CENTER);
    lv_obj_set_style_local_text_font(label_music_name, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_20_cn_gb);
    lv_obj_set_style_local_border_width(label_music_name, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 1);
    lv_obj_set_style_local_border_color(label_music_name, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, COLOR_THEME);
    lv_obj_set_style_local_bg_opa(label_music_name, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_80);
    lv_obj_set_style_local_pad_all(label_music_name, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 10);
    lv_obj_set_style_local_radius(label_music_name, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_label_set_long_mode(label_music_name, LV_LABEL_LONG_SROLL_CIRC);
    lv_obj_set_size(label_music_name, 400, 48);
    lv_obj_align(label_music_name, NULL, LV_ALIGN_IN_TOP_MID, 0, 15);

    roller_lyric = lv_roller_create(obj_lyric, NULL);
    lv_roller_set_auto_fit(roller_lyric, false);
    lv_roller_set_visible_row_count(roller_lyric, 3);
    lv_obj_set_width(roller_lyric, 450);
    lv_roller_set_options(roller_lyric, music_lyric[0], LV_ROLLER_MODE_NORMAL);
    lv_obj_set_style_local_text_font(roller_lyric, LV_ROLLER_PART_BG, LV_STATE_DEFAULT, &font_20_cn_gb);
    lv_obj_set_style_local_text_font(roller_lyric, LV_ROLLER_PART_SELECTED, LV_STATE_DEFAULT, &font_20_cn_gb);
    lv_obj_set_style_local_bg_color(roller_lyric, LV_ROLLER_PART_BG, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_bg_color(roller_lyric, LV_ROLLER_PART_SELECTED, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_text_color(roller_lyric, LV_ROLLER_PART_BG, LV_STATE_DEFAULT, COLOR_BAR);
    lv_obj_set_style_local_text_color(roller_lyric, LV_ROLLER_PART_SELECTED, LV_STATE_DEFAULT, COLOR_THEME);
    lv_obj_set_style_local_bg_opa(roller_lyric, LV_ROLLER_PART_BG, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_border_width(roller_lyric, LV_ROLLER_PART_BG, LV_STATE_DEFAULT, 0);
    lv_obj_align(roller_lyric, label_music_name, LV_ALIGN_OUT_BOTTOM_MID, 0, 40);

    list_music_file = lv_list_create(lv_scr_act(), NULL);
    lv_obj_set_hidden(list_music_file, true);
    lv_obj_set_size(list_music_file, 300, 250);
    lv_obj_set_style_local_text_font(list_music_file, LV_LIST_PART_BG, LV_STATE_DEFAULT, &font_20_cn_gb);
    lv_obj_set_style_local_border_width(list_music_file, LV_LIST_PART_BG, LV_STATE_DEFAULT, 1);
    lv_obj_set_style_local_border_color(list_music_file, LV_LIST_PART_BG, LV_STATE_DEFAULT, COLOR_THEME);
    lv_obj_align(list_music_file, obj_control, LV_ALIGN_OUT_TOP_RIGHT, 0, -10);
    /* Update music file to list */

    app_player_callback_register(audio_cb, NULL);
    /* Start music task */
    if (sdcard_ok) {
        ESP_ERROR_CHECK(app_player_start("/sdcard"));
    } else {
        ESP_ERROR_CHECK(app_player_start("/spiffs"));
    }

    lv_task_create(lyric_task, 100, 1, NULL);
    ui_music_state = ui_state_show;
}

void ui_music_show(void *data)
{
    if (ui_state_dis == ui_music_state) {
        ui_music_init(data);
        ui_status_bar_time_show(true);
    } else {
        lv_obj_set_hidden(slider_volume, false);
        lv_obj_set_hidden(obj_control, false);
        lv_obj_set_hidden(obj_lyric, false);
        lv_obj_set_hidden(img_album, false);

        ui_status_bar_time_show(true);

        ui_music_state = ui_state_show;
    }
}

void ui_music_hide(void *data)
{
    (void) data;

    if (ui_state_show == ui_music_state) {
        lv_obj_set_hidden(slider_volume, true);
        lv_obj_set_hidden(obj_control, true);
        lv_obj_set_hidden(obj_lyric, true);
        lv_obj_set_hidden(img_album, true);
        lv_obj_set_hidden(list_music_file, true);

        ui_music_state = ui_state_hide;
    }
}

static void lyric_task(lv_task_t *task)
{
    if (PLAYER_STATE_PLAYING != app_player_get_state()) {
        return;
    }

    uint32_t ct = app_player_get_current_time();
    static uint32_t counter_1s = 0;
    if (++counter_1s >= 10) {
        counter_1s = 0;
        if (label_music_time && slider_progress && music_total_time) {
            uint32_t t = ct / 1000;
            lv_label_set_text_fmt(label_music_time, "%02d:%02d", t / 60, t % 60);
            lv_slider_set_value(slider_progress, 100 * t / music_total_time, 0);
        }
    }

    static uint16_t last_lrc_index = 0;
    if (lrc) {
        for (int i = lrc_line_num - 1; i >= 0; i--) {
            if (ct > lrc_ts[i]) {
                if (last_lrc_index != i) {
                    last_lrc_index = i;
                    printf("lrc set to %d\n", i);
                    lv_roller_set_selected(roller_lyric, i, LV_ANIM_ON);
                }
                break;
            }
        }
    }
}

static void slider_volume_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_VALUE_CHANGED == event) {
        int v = lv_slider_get_value(obj);
        ESP_LOGI(TAG, "volume=%d", v);
        bsp_codec_set_voice_volume(v);
    }
}

static void btn_play_pause_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_CLICKED == event) {
        bool is_playing = PLAYER_STATE_PLAYING == app_player_get_state();
        if (is_playing) {
            app_player_pause();
            lv_obj_set_style_local_value_str(btn_play_pause, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_PLAY);
        } else {
            app_player_play();
            lv_obj_set_style_local_value_str(btn_play_pause, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_PAUSE);
        }
    }
}

static void btn_list_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_CLICKED == event) {
        lv_obj_set_hidden(list_music_file, !lv_obj_get_hidden(list_music_file));
    }
}

static void btn_file_list_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_CLICKED == event) {
        lv_obj_t *btn = obj;
        int32_t i = lv_list_get_btn_index(list_music_file, btn);
        ESP_LOGI(TAG, "selected %d-%s", i, lv_list_get_btn_text(btn));
        app_player_play_index(i);
    }
}

static void btn_prev_next_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_CLICKED == event) {
        if (btn_prev == obj) {
            app_player_play_prev();
        } else {
            app_player_play_next();
        }
    }
}
