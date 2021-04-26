/**
 * @file lvgl_port.h
 * @brief 
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

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "lvgl/lvgl.h"

#include "bsp_lcd.h"
#include "ft5x06.h"
#include "rm68120.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LVGL_SCR_SIZE       (LV_HOR_RES_MAX * LV_VER_RES_MAX)
#define LVGL_BUFFER_SIZE    (sizeof(lv_color_t) * LVGL_SCR_SIZE)
#define LVGL_INIT_PIXCNT    (LV_HOR_RES_MAX * LV_VER_RES_MAX)

#define LV_BUF_ALLOC_INTERNAL   (MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT)
#define LV_BUF_ALLOC_EXTERNAL   (MALLOC_CAP_SPIRAM) 

typedef struct {
    size_t buffer_size;
    uint32_t buffer_alloc_caps;
} lvgl_port_init_config_t;

/**
 * @brief Initialize LVGL with config
 * 
 * @param cfg 
 * @return esp_err_t 
 */
esp_err_t lvgl_init(size_t buffer_pix_size, uint32_t buffer_caps);

/**
 * @brief Take LVGL widget update and flush semaphore
 * 
 * @return esp_err_t 
 */
esp_err_t lv_port_sem_take(void);

/**
 * @brief Give LVGL widget update and flush semaphore
 * 
 * @return esp_err_t 
 */
esp_err_t lv_port_sem_give(void);

/**
 * @brief Use default ported lvgl handler task or not.
 * 
 * @param en Enable or not.
 */
void lv_port_use_default_handler(bool en);

#ifdef __cplusplus
}
#endif
