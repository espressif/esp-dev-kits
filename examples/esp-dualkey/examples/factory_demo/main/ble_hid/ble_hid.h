/* SPDX-FileCopyrightText: 2022-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "btn_progress.h"

/**
 * @brief Initialize BLE HID device.
 *
 * @return
 *    - ESP_OK: Success
 *    - ESP_ERR_NO_MEM: No memory
 */
esp_err_t ble_hid_init(void);

/**
 * @brief Deinitialize BLE HID device.
 *
 * @return
 *    - ESP_OK: Success
 *    - ESP_ERR_NO_MEM: No memory
 */
esp_err_t ble_hid_deinit(void);

/**
 * @brief Send ble hid keyboard report
 *
 * @param report hid report
 */
void ble_hid_keyboard_report(hid_report_t report);

/**
 * @brief Send ble hid mouse report
 *
 * @param report hid report
 */
void ble_hid_mouse_report(hid_report_t report);

/**
 * @brief Check if BLE HID is connected
 *
 * @return true if connected
 * @return false if not connected
 */
bool ble_hid_is_connected(void);

/**
 * @brief Update battery level for BLE HID device
 *
 * @param level battery level (0-100)
 * @return ESP_OK on success, ESP_FAIL on failure
 */
esp_err_t ble_hid_battery_update(uint8_t level);

#ifdef __cplusplus
}
#endif
