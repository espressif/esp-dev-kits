/**
 * @file ui_music.h
 * @brief UI header file.
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

#include "lvgl/lvgl.h"
#include "lvgl_port.h"

#include "audio_hal.h"
#include "tpl0401.h"

#include "app_music.h"

#define COLOR_BAR   lv_color_make(86, 94, 102)
#define COLOR_THEME lv_color_make(252, 199, 0)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Init music example UI
 * 
 */
void ui_music(void);

/**
 * @brief Update length of music file in music page.
 * 
 * @param data_size Size of audio file. Length will automiclly calculated by `SAMPLE_RATE` in `app_music.h`.
 */
void ui_music_update_length(size_t data_size);

/**
 * @brief Update current play time of music file in music page.
 * 
 * @param data_index Current file index.
 */
void ui_music_update_update_time(size_t data_index);

/**
 * @brief Update play/pause button accroding to audio state.
 * 
 */
void ui_music_update_btn(void);

#ifdef __cplusplus
}
#endif
