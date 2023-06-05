/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
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
#include "freertos/semphr.h"

#include "esp_err.h"
#include "esp_log.h"
#include "cJSON.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    LOCATION_NUM_SHANGHAI,
    LOCATION_NUM_BEIJING,
    LOCATION_NUM_SHENZHEN,
    LOCATION_NUM_MAX,
} location_num_t;

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
    char temp;
    char icon_code;
    char humid;
    char *describe;
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
 * @brief Post weather request.
 *
 * @return esp_err_t request status.
 */
esp_err_t app_weather_request(location_num_t location);

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
esp_err_t app_weather_get_current_info(weather_info_t *info, location_num_t location);

#ifdef __cplusplus
}
#endif
