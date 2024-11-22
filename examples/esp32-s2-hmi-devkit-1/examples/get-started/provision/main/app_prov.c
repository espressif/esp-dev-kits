/**
 * @file app_prov.c
 * @brief Provision task.
 * @version 0.1
 * @date 2021-03-07
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

#include "app_prov.h"

static void app_prov_task(void *pvParam)
{
    (void) pvParam;

    uint16_t ap_count = 0;
    wifi_ap_record_t *ap_info = heap_caps_malloc(sizeof(wifi_ap_record_t), MALLOC_CAP_INTERNAL);

    /*!< Init Wi-Fi with STA mode */
    wifi_sta_start();

    /*!< Start WI-Fi scan */
    wifi_scan_start();
    ui_scan_start();

    /*!< Wait for scanning done */
    wifi_scan_wait_done(portMAX_DELAY);

    /*!< Get total scanned ap count */
    wifi_scan_get_count(&ap_count);
    

    if (ap_count > 20) {
        ui_set_list_count(20);
    } else {
        ui_set_list_count(ap_count);
    }

    for (int i = 0; i < 20; i++) {
        wifi_scan_get_ap_info(i, &ap_info);
        ui_set_list_text(i, (const char *)(ap_info->ssid), ap_info->rssi, true);
    }

    wifi_scan_free_memory();

    while (ESP_OK != wifi_sta_wait_for_connect(portMAX_DELAY, true)) {
        ui_connected(false);
    }

    ui_connected(true);
    esp_wifi_sta_get_ap_info(ap_info);
    ui_show_ap_info(ap_info);

    vTaskDelete(NULL);
}

esp_err_t app_prov_start(void)
{
    if (pdPASS != xTaskCreate(
		(TaskFunction_t)        app_prov_task,
		(const char * const)    "Provision Task",
		(const uint32_t)        4 * 1024,
		(void * const)          NULL,
		(UBaseType_t)           configMAX_PRIORITIES - 3,
		(TaskHandle_t * const)  NULL)) {
        return ESP_ERR_NO_MEM;
    }

    return ESP_OK;
}
