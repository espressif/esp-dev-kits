/**
 * @file app_music.h
 * @brief Music app header.
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

#pragma once

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "audio_hal.h"
#include "bsp_sdcard.h"
#include "fs_hal.h"
#include "lvgl/lvgl.h"
#include "lvgl_port.h"

#include "ui_music.h"

#define SAMPLE_RATE     (44100)

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MUSIC_EVENT_NONE = 0,
	MUSIC_EVENT_PREV,
	MUSIC_EVENT_NEXT,
} music_event_t;

typedef struct _audio_file_info_list_t {
    char *file_name;
    size_t file_size;
    struct _audio_file_info_list_t *prev;
    struct _audio_file_info_list_t *next;
} audio_file_info_list_t;

/**
 * @brief Start music player app.
 * 
 * @return esp_err_t Task create state.
 */
esp_err_t app_music_start(void);

/**
 * @brief Send event to music app.
 * 
 * @param event Event to send. See `music_event_t`.
 * @return esp_err_t Send state.
 */
esp_err_t app_music_send_event(music_event_t event);

/**
 * @brief Scan audio files with given path.
 * 
 * @param path The full path want to scan. Like "/sdcard/music" or "/spiffs/audio".
 * @param p_file_count pointer to count of files.
 * @return esp_err_t Scan result.
 */
esp_err_t app_music_update_music_info(const char *path, size_t *p_file_count);

/**
 * @brief Get head of audio info link, NULL if no file or not scaned.
 * 
 * @param head poniter to the pointer of audio file info list.
 * @return esp_err_t Result of link head get result.
 */
esp_err_t app_music_get_audio_info_link_head(audio_file_info_list_t **head);

#ifdef __cplusplus
}
#endif
