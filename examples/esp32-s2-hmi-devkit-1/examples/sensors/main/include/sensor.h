/**
 * @file sensor.h
 * @brief 
 * @version 0.1
 * @date 2021-01-14
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
#include "bh1750.h"
#include "ft5x06.h"
#include "hdc1080.h"
#include "mpu6050.h"
#include "tca9554.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize sensor
 * 
 * @return esp_err_t 
 *      - ESP_OK : Sensor initialize successfully
 */
esp_err_t sensor_init(void);

/**
 * @brief 
 * 
 * @return mpu6050_acce_value_t* 
 */
void sensor_get_mems_acc(mpu6050_acce_value_t *data);

/**
 * @brief 
 * 
 */
void sensor_get_mems_gyro(mpu6050_gyro_value_t *data);

/**
 * @brief 
 * 
 * @param data 
 */
void als_get_value(float *data);

/**
 * @brief 
 * 
 * @param data 
 */
void sensor_get_temp(float *data);

/**
 * @brief 
 * 
 * @param data 
 */
void sensor_get_humid(float *data);

#ifdef __cplusplus
}
#endif
