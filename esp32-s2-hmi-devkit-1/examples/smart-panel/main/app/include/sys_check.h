/**
 * @file sys_check.h
 * @brief System check function header.
 * @version 0.1
 * @date 2021-04-14
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
#include "esp_heap_caps.h"
#include "fs_hal.h"
#include "lvgl/lvgl.h"
#include "lvgl_port.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Check for system requirements.
 * 
 * @return esp_err_t Check result.
 */
esp_err_t sys_check(void);

/**
 * @brief Check for image resources file.
 * 
 * @param lv_img_path Path with LVGL file system type.
 * @return esp_err_t Reslut of status.
 */
esp_err_t img_file_check(const char *lv_img_path);

#ifdef __cplusplus
}
#endif

