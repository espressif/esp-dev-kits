/**
 * @file ui_sensor.h
 * @brief 
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
#include "sensor.h"

#include "adc081.h"
#include "bh1750.h"
#include "mpu6050.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    lv_obj_t **obj;
    const void *img_default;
    const void *img_press;
    lv_event_cb_t event_cb;
} ui_img_src_t;

/**
 * @brief Initialize sensor evaluation UI
 * 
 */
void ui_sensor_init(void);

#ifdef __cplusplus
}
#endif
