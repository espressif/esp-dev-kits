/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#pragma once

#include "esp_err.h"
#include <stdbool.h>
#include "esp_a2dp_api.h"
#include "esp_gap_bt_api.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Bluetooth scan state definitions */
typedef enum {
    BT_SCAN_IDLE = 0,
    BT_SCAN_BUSY,
    BT_SCAN_RENEW,
    BT_SCAN_UPDATE
} bt_scan_state_t;

/* Bluetooth connection status */
typedef enum {
    BT_STATUS_DISCONNECTED = 0,
    BT_STATUS_CONNECTING,
    BT_STATUS_CONNECTED,
    BT_STATUS_CONNECT_FAILED,
} bt_connect_status_t;

/* Bluetooth events */
typedef enum {
    BT_EVENT_SCAN = 0,
    BT_EVENT_CONNECT,
    BT_EVENT_DISCONNECT,
    BT_EVENT_RECONNECT
} bt_event_t;

/* Device information structure */
typedef struct {
    uint8_t addr[6];
    char name[ESP_BT_GAP_MAX_BDNAME_LEN + 1];
    int32_t rssi;
} bt_device_info_t;

/* Scan result structure (similar to WiFi scan_result) */
typedef struct {
    bt_device_info_t devices[20];
    uint8_t device_count;
    bt_scan_state_t scan_state;
} bt_scan_result_t;

/* Global scan result (similar to WiFi) */
extern bt_scan_result_t bt_scan_result;

/**
 * @brief Initialize Bluetooth module (A2DP Source for connecting to speakers)
 *
 * @return esp_err_t
 *      - ESP_OK : Bluetooth initialized successfully
 */
esp_err_t app_bluetooth_init(void);

/**
 * @brief Start Bluetooth scanning for nearby devices (speakers/headphones)
 *
 * @return esp_err_t
 */
esp_err_t app_bluetooth_start_scan(void);

/**
 * @brief Stop Bluetooth scanning
 *
 * @return esp_err_t
 */
esp_err_t app_bluetooth_stop_scan(void);

/**
 * @brief Connect to a Bluetooth speaker/device by address
 *
 * @param addr Bluetooth device address (6 bytes)
 * @return esp_err_t
 */
esp_err_t app_bluetooth_connect(const uint8_t *addr);

/**
 * @brief Disconnect from current Bluetooth device
 *
 * @return esp_err_t
 */
esp_err_t app_bluetooth_disconnect(void);

/**
 * @brief Get current Bluetooth connection status
 *
 * @return bt_connect_status_t Current status
 */
bt_connect_status_t app_bluetooth_get_connect_status(void);

/**
 * @brief Check if device is already connected
 *
 * @return bt_connect_status_t Connection status
 */
bt_connect_status_t bluetooth_connected_already(void);

/**
 * @brief Send Bluetooth event (scan, connect, etc.)
 *
 * @param event Event type to send
 */
void app_bluetooth_send_event(bt_event_t event);

/**
 * @brief Get scan result structure
 *
 * @return bt_scan_result_t Current scan results
 */
bt_scan_result_t app_bluetooth_get_scan_result(void);

/**
 * @brief Set scan state
 *
 * @param state New scan state
 */
void app_bluetooth_set_scan_state(bt_scan_state_t state);

/**
 * @brief Lock Bluetooth resources (for thread safety)
 *
 * @param timeout_ms Timeout in milliseconds (0 = wait forever)
 * @return true if lock acquired, false on timeout
 */
bool app_bluetooth_lock(uint32_t timeout_ms);

/**
 * @brief Unlock Bluetooth resources
 */
void app_bluetooth_unlock(void);

#ifdef __cplusplus
}
#endif