/**
 * @file lv_demos.c
 * @brief Evaluate demos provided by LVGL
 * @version 0.1
 * @date 2021-10-19
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

#include "bsp_board.h"
#include "bsp_lcd.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "lv_port.h"
#include "lvgl.h"
#include "nvs_flash.h"
#include "espnow_lite.h"
#include "lv_example_scroll.h"


void espnow_start()
{
    uint8_t broadcast_mac[ESP_NOW_ETH_ALEN] = { 0xFF, 0xFF, 0xFF,0xFF, 0xFF, 0xFF };

    #if 0
    esp_now_peer_info_t* peer = malloc(sizeof(esp_now_peer_info_t));
    espnow_payload_t* send_data = malloc(sizeof(espnow_payload_t));
    #else
    esp_now_peer_info_t* peer = heap_caps_malloc(sizeof(esp_now_peer_info_t), MALLOC_CAP_SPIRAM);
    espnow_payload_t* send_data = heap_caps_malloc(sizeof(espnow_payload_t), MALLOC_CAP_SPIRAM);
    #endif
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);
    espnow_wifi_init();
    espnow_init();
}

void app_main(void)
{
    LV_LOG_USER("start up");
    ESP_ERROR_CHECK(bsp_board_init());
    ESP_ERROR_CHECK(bsp_board_power_ctrl(POWER_MODULE_AUDIO, true));

    lv_port_init();
    bsp_lcd_set_backlight(true);
    ESP_ERROR_CHECK(bsp_spiffs_init("storage", "/spiffs", 2));
    ESP_ERROR_CHECK(mp3_player_start("/spiffs"));

    lv_port_sem_take();
    lv_example_86box();
    lv_port_sem_give();

    espnow_start();
}
