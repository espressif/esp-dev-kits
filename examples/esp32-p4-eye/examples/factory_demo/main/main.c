/*
 * SPDX-FileCopyrightText: 2024-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include "esp_log.h"
#include "esp_memory_utils.h"
#include "nvs_flash.h"
#include "nvs.h"

#include "bsp/esp-bsp.h"

#include "ui_extra.h"

#include "app_control.h"
#include "app_video_stream.h"
#include "app_storage.h"
#include "app_ai_detect.h"
#include "app_qma6100.h"

static const char *TAG = "main";

void app_main(void)
{
    // Initialize NVS
    ESP_LOGI(TAG, "Initialize NVS");
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize the flashlight
    ESP_LOGI(TAG, "Initialize the flashlight");
    ESP_ERROR_CHECK(bsp_flashlight_init());

    // Initialize the I2C first (needed for QMA6100)
    ESP_LOGI(TAG, "Initialize the I2C");
    i2c_master_bus_handle_t i2c_handle;
    ESP_ERROR_CHECK(bsp_i2c_init());
    bsp_get_i2c_bus_handle(&i2c_handle);

    // Initialize the AI detect
    ESP_LOGI(TAG, "Initialize the AI detect");
    ESP_ERROR_CHECK(app_ai_detect_init());

    // Initialize the display
    ESP_LOGI(TAG, "Initialize the display");
    bsp_display_start();

    bsp_display_lock(0);
    ui_extra_init();
    bsp_display_unlock();

    // Initialize the QMA6100 IMU sensor with integrated display auto-rotation
    ESP_LOGI(TAG, "Initialize the QMA6100 IMU sensor with display auto-rotation");
    ret = app_qma6100_init(i2c_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize QMA6100: %s", esp_err_to_name(ret));
        return;
    }

    // Initialize the storage
    ESP_LOGI(TAG, "Initialize the storage");
    ESP_ERROR_CHECK(app_storage_init());

    // Turn on the display backlight
    bsp_display_backlight_on();

    // Initialize the application control module
    ESP_LOGI(TAG, "Initialize the application control module");
    ESP_ERROR_CHECK(app_control_init());

    // Initialize the video streaming application
    ESP_LOGI(TAG, "Initialize the video streaming application");
    ESP_ERROR_CHECK(app_video_stream_init(i2c_handle));
    
    ESP_LOGI(TAG, "Application initialization completed");
}