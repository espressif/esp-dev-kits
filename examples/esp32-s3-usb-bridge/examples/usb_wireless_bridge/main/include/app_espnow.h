/* SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/ringbuf.h"
#include "app_mode.h"
#include "espnow.h"

typedef enum {
    LINE_STATE = 0,
    LINE_CODING,
} tud_cdc_action_t;

typedef struct {
    tud_cdc_action_t tud_cdc_action;
    union {
        char state[2];
        uint32_t baudrate;
    };
} espnow_debug_command_frame_t;

/**
 * @brief Initialize espnow and create espnow task
 * 
 */
void app_espnow_init(void);

/**
 * @brief Create espnow task in wireless host mode
 * 
 * @param ringBuf ring buffer
 * @return
 *    - ESP_OK: succeed
 *    - others: fail
 */
esp_err_t mode_wireless_host_espnow_create(RingbufHandle_t ringBuf);

/**
 * @brief Create espnow task in wireless slave mode
 * 
 * @param ringBuf ring buffer 
 * @return
 *    - ESP_OK: succeed
 *    - others: fail
 */
esp_err_t mode_wireless_slave_espnow_create(RingbufHandle_t ringBuf);

/**
 * @brief espnow send
 * 
 * @param type espnow data type
 * @param data send data
 * @param size data size
 * @param frame_config frame header of espnow
 * @param wait_ticks wait time
 * @return
 *    - ESP_OK
 *    - ESP_ERR_INVALID_ARG
 *    - ESP_ERR_TIMEOUT
 *    - ESP_ERR_WIFI_TIMEOUT
 */
esp_err_t app_espnow_send(espnow_data_type_t type, const void *data, size_t size, const espnow_frame_head_t *frame_config, TickType_t wait_ticks);

/**
 * @brief Start binding in wireless mode for the device.
 * 
 * @param wait_ms maximum waiting bind time in millisecond
 * @param rssi the minimum bind frame RSSI
 * @return
 *    - ESP_OK: succeed
 *    - others: fail
 */
esp_err_t mode_wireless_espnow_bind_begin(uint32_t wait_ms, int8_t rssi);

/**
 * @brief Process log messages received in Wireless host mode
 * 
 * @param data Received data
 * @param size Data length
 * @return
 *    - ESP_OK: succeed
 *    - others: fail
 */
esp_err_t mode_wireless_host_debug_process(uint8_t *data, size_t size);

/**
 * @brief Process log messages received in Wireless slave mode
 * 
 * @param data Received data
 * @param size Data length
 * @return
 *    - ESP_OK: succeed
 *    - others: fail
 */
esp_err_t mode_wireless_slave_debug_process(uint8_t *data, size_t size);

/**
 * @brief Delete the bound MAC address
 *
 * @return
 *    - ESP_OK: succeed
 *    - others: fail 
 */
esp_err_t app_espnow_remove_bind(void);
#ifdef __cplusplus
}
#endif
