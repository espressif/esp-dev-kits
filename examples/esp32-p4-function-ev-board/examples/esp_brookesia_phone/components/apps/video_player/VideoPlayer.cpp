/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <algorithm>
#include <fcntl.h>
#include <dirent.h>
#include "esp_check.h"
#include "esp_heap_caps.h"
#include "bsp/esp-bsp.h"
#include "bsp_board_extra.h"
#include "esp_lvgl_simple_player/media_src_storage.h"
#include "esp_lvgl_simple_player/esp_lvgl_simple_player.h"
#include "VideoPlayer.hpp"

#define APP_SUPPORT_VIDEO_FILE_EXT  ".mjpeg"
#define APP_BGM_DIR   BSP_SPIFFS_MOUNT_POINT "/music"
#define APP_MAX_VIDEO_NUM           (15)
#define APP_VIDEO_FRAME_BUF_SIZE    (720 * 1280 * BSP_LCD_BITS_PER_PIXEL / 8)
#define APP_CACHE_BUF_SIZE          (64 * 1024)
#define APP_BREAKING_NEWS_TEXT      "This example demonstrates the JPEG decoding capability of the ESP32-P4"

using namespace std;

static const char *TAG = "AppVideoPlayer";

LV_IMG_DECLARE(breaking_news);
LV_IMG_DECLARE(img_app_video_player);

AppVideoPlayer::AppVideoPlayer():
    ESP_Brookesia_PhoneApp("Video Player", &img_app_video_player, true), // auto_resize_visual_area
    _video_name(NULL),
    img_breaking_news(NULL),
    row_edit(NULL),
    lbl_breaking_news(NULL)
{
}

AppVideoPlayer::~AppVideoPlayer()
{
}

bool AppVideoPlayer::run(void)
{
    app_show_ui();

    return true;
}

bool AppVideoPlayer::pause(void)
{
    esp_lvgl_simple_player_pause();

    return true;
}

bool AppVideoPlayer::resume(void)
{
    esp_lvgl_simple_player_resume();

    return true;
}

bool AppVideoPlayer::back(void)
{
    return notifyCoreClosed();
}

bool AppVideoPlayer::close(void)
{
    bsp_display_unlock();
    esp_lvgl_simple_player_del();
    bsp_display_lock(100);

    return true;
}

bool AppVideoPlayer::init(void)
{
    return true;
}

void AppVideoPlayer::app_show_ui(void)
{
    uint8_t i = 0;
    int sel_file = searchMideaFiles();

    lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), 0);

    /* Rows */
    lv_obj_t *cont_col = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont_col, BSP_LCD_H_RES, BSP_LCD_V_RES);
    lv_obj_set_flex_flow(cont_col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(cont_col, 0, 0);
    lv_obj_set_flex_align(cont_col, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_color(cont_col, lv_color_black(), 0);
    lv_obj_set_style_border_width(cont_col, 0, 0);

    lv_obj_t *cont_row = lv_obj_create(cont_col);
    lv_obj_set_size(cont_row, BSP_LCD_H_RES - 20, 80);
    lv_obj_set_flex_flow(cont_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_top(cont_row, 2, 0);
    lv_obj_set_style_pad_bottom(cont_row, 2, 0);
    lv_obj_set_flex_align(cont_row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_color(cont_row, lv_color_black(), 0);
    lv_obj_set_style_border_width(cont_row, 0, 0);

    /* Dropdown files */
    lv_obj_t * dd = lv_dropdown_create(cont_row);
    lv_dropdown_clear_options(dd);
    lv_obj_set_width(dd, BSP_LCD_H_RES / 3);
    for (auto &it : _midea_info_vect) {
        lv_dropdown_add_option(dd, it.video_name.c_str(), i);
        i++;
    }
    lv_obj_add_event_cb(dd, file_changed, LV_EVENT_VALUE_CHANGED, this);
    lv_dropdown_set_selected(dd, sel_file);
    lv_obj_set_style_pad_top(dd, 5, 0);

    /* Checkbox - hide controls */
    lv_obj_t * hide_ctl_cb = lv_checkbox_create(cont_row);
    lv_checkbox_set_text(hide_ctl_cb, "HIDE CONTROLS");
    lv_obj_set_style_text_color(hide_ctl_cb, lv_color_white(), 0);
    lv_obj_add_event_cb(hide_ctl_cb, hide_controls_changed, LV_EVENT_VALUE_CHANGED, this);

    /* Checkbox - breaking news */
    lv_obj_t * breaking_news_cb = lv_checkbox_create(cont_row);
    lv_checkbox_set_text(breaking_news_cb, "BREAKING NEWS");
    lv_obj_set_style_text_color(breaking_news_cb, lv_color_white(), 0);
    lv_obj_add_event_cb(breaking_news_cb, breaking_news_changed, LV_EVENT_VALUE_CHANGED, this);

    /* Create player */
    snprintf(_video_path, sizeof(_video_path), "%s/%s", BSP_SD_MOUNT_POINT, _video_name);
    esp_lvgl_simple_player_cfg_t player_cfg = {
        .video_path = _video_path,
        .screen = cont_col,
        .buff_size = APP_VIDEO_FRAME_BUF_SIZE,
        .cache_buff_size = APP_CACHE_BUF_SIZE,
        .cache_buff_in_psram = true,
        .screen_width = BSP_LCD_H_RES,
        .screen_height = (BSP_LCD_V_RES / 2),
        .flags = {
            .auto_height = true,
        },
    };
    esp_lvgl_simple_player_create(&player_cfg);

    /* Breaking news image */
    img_breaking_news = lv_img_create(lv_scr_act());
    lv_img_set_src(img_breaking_news, &breaking_news);
    lv_obj_align(img_breaking_news, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_flag(img_breaking_news, LV_OBJ_FLAG_HIDDEN);

    lbl_breaking_news = lv_label_create(img_breaking_news);
    lv_obj_set_width(lbl_breaking_news, BSP_LCD_H_RES - 155);
    lv_label_set_text(lbl_breaking_news, APP_BREAKING_NEWS_TEXT);
    lv_obj_set_style_text_font(lbl_breaking_news, &lv_font_montserrat_16, 0);
    lv_label_set_long_mode(lbl_breaking_news, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_align(lbl_breaking_news, LV_ALIGN_BOTTOM_RIGHT, 200, -25);

    /* Start playing */
    esp_lvgl_simple_player_play();
}

uint8_t AppVideoPlayer::searchMideaFiles(void)
{
    int sel_file = 0;
    int i = 0;
    struct dirent *dir;
    DIR *d;

    _midea_info_vect.clear();

    // Search and store video files
    if (DIR *d = opendir(BSP_SD_MOUNT_POINT)) {
        while (struct dirent *dir = readdir(d)) {
            if (dir->d_type != DT_DIR && strstr(dir->d_name, APP_SUPPORT_VIDEO_FILE_EXT)) {
                if (_midea_info_vect.size() >= APP_MAX_VIDEO_NUM) {
                    ESP_LOGE(TAG, "Too many video files");
                    break;
                }
                ESP_LOGI(TAG, "Found video file: %s", dir->d_name);
                _midea_info_vect.push_back({string(dir->d_name)});
            }
        }
        closedir(d);  // Always close the directory
    }

    // Select the video file based on 'sel_file'
    if (sel_file >= 0 && sel_file < _midea_info_vect.size()) {
        _video_name = _midea_info_vect[sel_file].video_name.c_str();
    }

    return sel_file;
}

void AppVideoPlayer::file_changed(lv_event_t * e)
{
    AppVideoPlayer *app = (AppVideoPlayer *)lv_event_get_user_data(e);
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    char *video_path = app->_video_path;
    char video_name[64];
    bool found_video_file = false;

    if(code == LV_EVENT_VALUE_CHANGED) {
        lv_dropdown_get_selected_str(obj, video_name, sizeof(video_name));
        snprintf(video_path, sizeof(app->_video_path), "%s/%s", BSP_SD_MOUNT_POINT, video_name);
        ESP_LOGI(TAG, "Selected file: %s", video_path);

        for (auto &it : app->_midea_info_vect) {
            if (strcmp(it.video_name.c_str(), video_name) == 0) {
                found_video_file = true;
                break;
            }
        }
        if (!found_video_file) {
            ESP_LOGE(TAG, "File not found in the map");
            return;
        }

        esp_lvgl_simple_player_stop();

        bsp_display_unlock();
        if (esp_lvgl_simple_player_wait_task_stop(-1) != ESP_OK) {
            ESP_LOGE(TAG, "Player task stop timeout");
        }
        bsp_display_lock(100);

        esp_lvgl_simple_player_change_file(video_path);
        esp_lvgl_simple_player_play();
    }
}

void AppVideoPlayer::breaking_news_changed(lv_event_t * e)
{
    AppVideoPlayer *app = (AppVideoPlayer *)lv_event_get_user_data(e);
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    lv_obj_t * img_breaking_news = app->img_breaking_news;

    if(code == LV_EVENT_VALUE_CHANGED) {
        if (img_breaking_news) {
            const bool state = (lv_obj_get_state(obj) & LV_STATE_CHECKED);
            if (state) {
                lv_obj_clear_flag(img_breaking_news, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_add_flag(img_breaking_news, LV_OBJ_FLAG_HIDDEN);
            }
        }
    }
}

void AppVideoPlayer::hide_controls_changed(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
        const bool state = (lv_obj_get_state(obj) & LV_STATE_CHECKED);
        esp_lvgl_simple_player_hide_controls(state);
    }
}
