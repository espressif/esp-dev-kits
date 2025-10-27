/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_dualkey.h"
#include "esp_app_desc.h"

static const char *TAG = "main";

void app_main(void)
{
    // Print application version information
    const esp_app_desc_t *app_desc = esp_app_get_description();
    ESP_LOGI(TAG, "Application Version: %s", app_desc->version);
    ESP_LOGI(TAG, "Project Name: %s", app_desc->project_name);
    ESP_LOGI(TAG, "Compile Date: %s %s", app_desc->date, app_desc->time);
    ESP_LOGI(TAG, "ESP-IDF Version: %s", app_desc->idf_ver);

    // Initialize NVS.
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS flash erase");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ret = esp_dualkey_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize dualkey: %s", esp_err_to_name(ret));
        return;
    }
}
