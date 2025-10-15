/* SPDX-FileCopyrightText: 2022-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "bsp/esp_dualkey.h"

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------+
// VIA Keymap Management Constants
//--------------------------------------------------------------------+
#define VIA_MAX_LAYERS 4
#define VIA_MATRIX_ROWS 1
#define VIA_MATRIX_COLS KBD_COL_NUM  // ESP-DualKey keyboard columns
#define VIA_NVS_NAMESPACE "via_keymap"

// Key code definitions for VIA (if not already defined elsewhere)
#ifndef LCTL
#define LCTL(kc) (0x0100 | (kc))  // Left Control modifier
#endif

#ifndef HID_KEY_C
#define HID_KEY_C           0x06  // Key C
#define HID_KEY_V           0x19  // Key V
#endif

// #ifndef KC_MS_WH_UP
// #define KC_MS_WH_UP         0xD9  // Up
// #define KC_MS_WH_DOWN       0xDA  // Down
// #endif

//--------------------------------------------------------------------+
// VIA Keymap Management Functions
//--------------------------------------------------------------------+

/**
 * @brief Function pointer types for VIA keymap NVS store/restore operations
 *
 * Both store and restore signatures:
 *   @param data   buffer pointer to read/write
 *   @param length buffer size (read/write length)
 *   @return esp_err_t ESP_OK on success
 */
typedef esp_err_t (*via_keymap_store_func_t)(const void *data, size_t length);
typedef esp_err_t (*via_keymap_restore_func_t)(void *data, size_t length);

/**
 * @brief Configuration structure for VIA keymap NVS functions
 */
typedef struct {
    via_keymap_store_func_t   store_fn;
    via_keymap_restore_func_t restore_fn;
} via_keymap_config_t;

/**
 * @brief Initialize VIA keymap system with default values and custom store/restore functions
 *
 * @param nvs_cfg Pointer to store/restore function config (can be NULL to use default)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t via_keymap_init(const via_keymap_config_t *config);

/**
 * @brief Get keycode from specified layer/row/col position
 *
 * @param layer Layer index (0-3)
 * @param row Row index (0)
 * @param col Column index (0-1)
 * @return uint16_t Keycode at specified position, 0 if invalid position
 */
uint16_t via_keymap_get_keycode(uint8_t layer, uint8_t row, uint8_t col);

/**
 * @brief Set keycode at specified layer/row/col position
 *
 * @param layer Layer index (0-3)
 * @param row Row index (0)
 * @param col Column index (0-1)
 * @param keycode Keycode to set
 * @return esp_err_t ESP_OK on success, ESP_ERR_INVALID_ARG for invalid position
 */
esp_err_t via_keymap_set_keycode(uint8_t layer, uint8_t row, uint8_t col, uint16_t keycode);

/**
 * @brief Check if layer/row/col position is valid
 *
 * @param layer Layer index
 * @param row Row index
 * @param col Column index
 * @return true if position is valid, false otherwise
 */
bool via_keymap_is_valid_position(uint8_t layer, uint8_t row, uint8_t col);

/**
 * @brief Reset keymap to default values and save to NVS
 *
 * @return esp_err_t ESP_OK on success
 */
esp_err_t via_keymap_reset(void);

#ifdef __cplusplus
}
#endif
