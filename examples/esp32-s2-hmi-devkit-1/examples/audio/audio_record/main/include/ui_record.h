/**
 * @file ui_record.h
 * @brief UI header file.
 * @version 0.1
 * @date 2021-02-26
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

#include "tpl0401.h"

#ifdef __cplusplus
extern "C" {
#endif

void ui_record(void);

void audio_play_start(void);
void audio_record_start(void);

#ifdef __cplusplus
}
#endif
