/**
 * @file app_weather.h
 * @brief Weather app header.
 * @version 0.1
 * @date 2021-04-07
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
#include <stdlib.h>
#include <string.h>

#include "esp_err.h"
#include "esp_heap_caps.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "cJSON.h"
#include "device.h"
#include "https_request.h"
#include "ui_main.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    air_item_pm2p5  = 0x00,
    air_item_aqi,
    air_item_level,
    air_item_pm10,
    air_item_no2,
    air_item_so2,
    air_item_co,
} ui_air_item_t;

typedef struct {
    char *describe;
    char *temp_range;
    char *icon_code;
} weather_daily_info_t;

typedef struct {
    char *temp;
} weather_hourly_info_t;

typedef struct {
    char *temp;
    char *humid;
    char *describe;
    char *icon_code;
    char *uv_val;
} weather_info_t;

typedef struct {
    char *aqi;
    char *level;
    char *co;
    char *no2;
    char *o3;
    char *pm2p5;
    char *pm10;
    char *so2;
} air_info_t;

/**
 * @brief Start weather task.
 * 
 * @return esp_err_t Task create status.
 */
esp_err_t app_weather_start(void);

/**
 * @brief Get air info.
 * 
 * @param info Pointer to `air_info_t`.
 * @return esp_err_t Result.
 */
esp_err_t app_weather_get_air_info(air_info_t *info);

/**
 * @brief Get current weather info.
 * 
 * @param info Pointer to `weather_info_t`.
 * @return esp_err_t Result.
 */
esp_err_t app_weather_get_current_info(weather_info_t *info);

/**
 * @brief Get weather info of specified day.
 * 
 * @param day Specified day.
 * @param info Pointer to `weather_daily_info_t`.
 * @return esp_err_t Result.
 */
esp_err_t app_weather_get_daily_info(size_t day, weather_daily_info_t *info);

/**
 * @brief Get weather info of specified hour.
 * 
 * @param hour Specified hour.
 * @param info Pointer to `weather_hourly_info_t`.
 * @return esp_err_t Result.
 */
esp_err_t app_weather_get_hourly_info(size_t hour, weather_hourly_info_t *info);

#ifdef __cplusplus
}
#endif
