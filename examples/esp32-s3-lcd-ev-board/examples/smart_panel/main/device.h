/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#pragma once

#include "esp_err.h"


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
