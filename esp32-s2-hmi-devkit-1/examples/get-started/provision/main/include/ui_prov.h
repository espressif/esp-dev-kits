/**
 * @file ui_prov.h
 * @brief Provision UI header file.
 * @version 0.1
 * @date 2021-02-24
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

#include "esp_err.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "lvgl/lvgl.h"
#include "lvgl_port.h"
#include "lv_symbol_extra_def.h"

#include "wifi_basic.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize provisioning UI
 * 
 */
void ui_prov_init(void);

/**
 * @brief Update Wi-Fi AP count after scanning
 * 
 * @param item 
 */
void ui_set_list_count(size_t item);

/**
 * @brief Update Wi-Fi to table
 * 
 */
void ui_set_list_text(size_t index, const char *ap_name, int rssi, bool secure);

/**
 * @brief 
 * 
 */
void ui_scan_start(void);

/**
 * @brief 
 * 
 * @param connected 
 */
void ui_connected(bool connected);

/**
 * @brief 
 * 
 * @param ap_info 
 */
void ui_show_ap_info(wifi_ap_record_t *ap_info);

#ifdef __cplusplus
}
#endif
