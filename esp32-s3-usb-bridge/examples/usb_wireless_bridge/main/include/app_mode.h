/* SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "freertos/FreeRTOS.h"
#include "freertos/ringbuf.h"
#include "app_indicator.h"

#define KEY_BIND_MAC              "mac"

typedef enum {
    MODE_WIRED = 0,           /* Wired connection: Supports JTAG debugging, sending and receiving logs, burning, UF2 */
    MODE_WIRELESS_HOST,       /* Wireless connection host: Supports one-to-many wireless transmission and reception of logs, burning, UF2 */
    MODE_WIRELESS_SLAVE,      /* Wireless connection slave: Supports receiving logs, wireless burning */
    MODE_MAX,                 /* Maximum mode value */
} work_mode_t;

typedef struct {
    work_mode_t mode;
    uint16_t hue;
    bool if_bind;
    uint8_t bind_mac[6];
    esp_err_t (*init)(void);
    esp_err_t (*serial_init)(RingbufHandle_t *ringbuf);
    esp_err_t (*espnow_int)(RingbufHandle_t ringbuf);
    esp_err_t (*espnow_bind_begin)(uint32_t wait_ms, int8_t rssi);
    esp_err_t (*espnow_debug_process)(uint8_t *data, size_t size);
    esp_err_t (*write_cdc_rx_data)(const void *data, size_t size);
    esp_err_t (*set_baudrate)(const int baudrate);
    esp_err_t (*set_boot_reset)(const bool dtr, const bool rts);
    esp_err_t (*led_update_mode)(led_indicator_mode_type mode_type, bool if_start);
} usb_bridge_mode_t;

/**
 * @brief Stops the timer.
 */
void timer_stop(void);

/**
 * @brief Checks if the timer is active.
 * 
 * @return true if the timer is active, false otherwise.
 */
bool timer_is_active(void);

/**
 * @brief Creates a usb bridge mode object
 * 
 * @param mode The work mode to be used
 * @return usb_bridge_mode_t* A pointer to the created usb bridge mode object
 */
usb_bridge_mode_t *usb_bridge_mode_create(work_mode_t mode);

/**
 * @brief Gets a handle to the usb bridge mode object
 * 
 * @return usb_bridge_mode_t* A pointer to the usb bridge mode object
 */
usb_bridge_mode_t *usb_bridge_get_handle(void);

#ifdef __cplusplus
}
#endif
