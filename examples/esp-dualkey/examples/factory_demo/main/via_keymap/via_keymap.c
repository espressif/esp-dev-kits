/* SPDX-FileCopyrightText: 2022-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include "esp_log.h"
#include "app_nvs.h"
#include "via_keymap.h"
#include "nvs.h"
#include "bsp/keycodes.h"
#include "btn_progress.h"
static const char *TAG = "via_keymap";

//--------------------------------------------------------------------+
// NVS Manager for VIA Keymap
//--------------------------------------------------------------------+

#define VIA_KEYMAP_KEY "keymap"
//--------------------------------------------------------------------+
// Default Keymap Configuration
//--------------------------------------------------------------------+

// Default keymap configuration for ESP-DualKey (1 row, 2 columns, 4 layers)
static const uint16_t keymaps_default[VIA_MAX_LAYERS][VIA_MATRIX_ROWS][VIA_MATRIX_COLS] = {
    /*
     * ┌────────┬────────┐
     * │CTRL + C│CTRL + V│
     * └────────┴────────┘
     */
    [0] = {
        {KC_KB_VOLUME_UP, KC_KB_VOLUME_DOWN},
    },
    [1] = {
        {KC_PAGE_UP, KC_PAGE_DOWN},
    },
    [2] = {
        {LCTL(HID_KEY_C),  LCTL(HID_KEY_V)},
    },
    [3] = {
        {KC_MS_WH_UP, KC_MS_WH_DOWN},
    },
};

// Dynamic keymap storage (modifiable copy of keymaps)
static uint16_t via_keymaps[VIA_MAX_LAYERS][VIA_MATRIX_ROWS][VIA_MATRIX_COLS];
static via_keymap_config_t via_keymap_config = {
    .store_fn = NULL,
    .restore_fn = NULL,
};

// Load keymap from NVS
esp_err_t via_keymap_load_from_nvs(void)
{
    if (via_keymap_config.restore_fn == NULL) {
        ESP_LOGE(TAG, "VIA: Restore function is not set");
        return ESP_ERR_INVALID_ARG;
    }
    esp_err_t ret = via_keymap_config.restore_fn(via_keymaps, sizeof(via_keymaps));
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "VIA: Successfully loaded keymap from NVS");
    } else {
        ESP_LOGE(TAG, "VIA: Failed to load keymap from NVS: %s", esp_err_to_name(ret));
    }
    return ret;
}

// Save keymap to NVS
esp_err_t via_keymap_save_to_nvs(void)
{
    // Save keymap data using NVS manager
    if (via_keymap_config.store_fn == NULL) {
        ESP_LOGE(TAG, "VIA: Store function is not set");
        return ESP_ERR_INVALID_ARG;
    }
    esp_err_t ret = via_keymap_config.store_fn(via_keymaps, sizeof(via_keymaps));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "VIA: Failed to save keymap to NVS: %s", esp_err_to_name(ret));
        return ret;
    }

    return ESP_OK;
}

//--------------------------------------------------------------------+
// VIA Keymap Management Implementation
//--------------------------------------------------------------------+

// Check if the layer/row/col position is valid
bool via_keymap_is_valid_position(uint8_t layer, uint8_t row, uint8_t col)
{
    return (layer < VIA_MAX_LAYERS && row < VIA_MATRIX_ROWS && col < VIA_MATRIX_COLS);
}

// Get keycode from dynamic keymap
uint16_t via_keymap_get_keycode(uint8_t layer, uint8_t row, uint8_t col)
{
    if (!via_keymap_is_valid_position(layer, row, col)) {
        ESP_LOGW(TAG, "VIA: Invalid keymap position L%d R%d C%d", layer, row, col);
        return 0;
    }

    uint16_t keycode = via_keymaps[layer][row][col];
    ESP_LOGD(TAG, "VIA: Get keycode L%d R%d C%d = 0x%04X", layer, row, col, keycode);
    return keycode;
}

// Set keycode in dynamic keymap
esp_err_t via_keymap_set_keycode(uint8_t layer, uint8_t row, uint8_t col, uint16_t keycode)
{
    if (!via_keymap_is_valid_position(layer, row, col)) {
        ESP_LOGW(TAG, "VIA: Invalid keymap position L%d R%d C%d", layer, row, col);
        return ESP_ERR_INVALID_ARG;
    }

    via_keymaps[layer][row][col] = keycode;
    ESP_LOGI(TAG, "VIA: Set keycode L%d R%d C%d = 0x%04X", layer, row, col, keycode);

    // Save to NVS after modification
    return via_keymap_save_to_nvs();
}

// Initialize VIA keymap with default values
esp_err_t via_keymap_init(const via_keymap_config_t *config)
{

    ESP_LOGI(TAG, "VIA: Initializing keymap (%dx%dx%d)", VIA_MAX_LAYERS, VIA_MATRIX_ROWS, VIA_MATRIX_COLS);

    if (config) {
        memcpy(&via_keymap_config, config, sizeof(via_keymap_config));
    }

    // Copy default keymap to dynamic keymap
    memcpy(via_keymaps, keymaps_default, sizeof(via_keymaps));

    ESP_LOGI(TAG, "VIA: Default keymap initialized:");
    ESP_LOGI(TAG, "  Layer 0: Ctrl+C, Ctrl+V (Copy/Paste)");
    ESP_LOGI(TAG, "  Layer 1: Page Up, Page Down");
    ESP_LOGI(TAG, "  Layer 2: Volume Up, Volume Down");
    ESP_LOGI(TAG, "  Layer 3: Mouse Wheel Up, Mouse Wheel Down");

    // Try to load custom keymap from NVS (will override defaults if found)
    esp_err_t ret = via_keymap_load_from_nvs();
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "VIA: Loaded custom keymap from NVS (overriding defaults)");
    } else {
        ESP_LOGI(TAG, "VIA: Using default keymap (NVS load failed: %s)", esp_err_to_name(ret));
    }

    return ESP_OK;
}

// Reset keymap to default values and save to NVS
esp_err_t via_keymap_reset(void)
{
    ESP_LOGI(TAG, "VIA: Resetting keymap to default values");

    // Copy default keymap to dynamic keymap
    memcpy(via_keymaps, keymaps_default, sizeof(via_keymaps));

    // Save reset keymap to NVS
    esp_err_t ret = via_keymap_save_to_nvs();
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "VIA: Keymap reset and saved to NVS successfully");
    } else {
        ESP_LOGE(TAG, "VIA: Failed to save reset keymap to NVS: %s", esp_err_to_name(ret));
    }

    return ret;
}
