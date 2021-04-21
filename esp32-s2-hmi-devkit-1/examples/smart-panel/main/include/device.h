/**
 * @file device.h
 * @brief ICs initialize, and create task to acquire data.
 * @version 0.1
 * @date 2021-03-30
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

#include "stdio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "esp_err.h"
#include "esp_log.h"

#include "bsp_ext_io.h"
#include "adc081.h"
#include "ft5x06.h"
#include "hdc1080.h"
#include "tca9554.h"
#include "tpl0401.h"
#include "ws2812.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize sensor
 * 
 * @return esp_err_t 
 *      - ESP_OK : Sensor initialize successfully
 */
esp_err_t device_init(void);

/**
 * @brief Start sensor task.
 * 
 * @return Task create status.
 */
esp_err_t sensor_task_start(void);

/**
 * @brief Get temperature data acquired from sensor.
 * 
 * @param data 
 */
void sensor_get_temp(float *data);

/**
 * @brief Get humid data acquired from sensor.
 * 
 * @param data 
 */
void sensor_get_humid(float *data);

/**
 * @brief Stop update sensor data and stop data acquire task.
 * 
 */
void sensor_data_update_stop(void);

/**
 * @brief Get raw data of battery adc.
 * 
 * @param val 
 */
void adc_bat_get_raw(uint8_t *val);

/**
 * @brief Get voltage of battery.
 * 
 * @param voltage Pointer to voltage value.
 */
void adc_bat_get_voltage(float *voltage);

#ifdef __cplusplus
}
#endif
