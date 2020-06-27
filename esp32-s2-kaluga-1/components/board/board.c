// Copyright 2019 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include "board.h"
#include "esp_log.h"
#include "esp_err.h"

static const char *TAG = "board_verison";

esp_err_t kaluga_board_verison(void)
{
    bool ret = false;
    /*!< main PAD */
#ifdef CONFIG_ESP32_S2_KALUGA_V1_3
    ESP_LOGI(TAG, "main pad:v1.3");
    ret = true;
#endif
#ifdef CONFIG_ESP32_S2_KALUGA_V1_2
    ESP_LOGI(TAG, "main pad:v1.2");
    ret = true;
#endif
#ifdef CONFIG_ESP32_S2_KALUGA_V1_1
    ESP_LOGI(TAG, "main pad:v1.1");
    ret = true;
#endif

    /*!< aduio PAD */
#ifdef CONFIG_AUDIO_PAD_ESP32_S2_KALUGA_V1_3
    ESP_LOGI(TAG, "audio pad:v1.3");
    ret = true;
#endif
#ifdef CONFIG_AUDIO_PAD_ESP32_S2_KALUGA_V1_2
    ESP_LOGI(TAG, "audio pad:v1.2");
    ret = true;
#endif
#ifdef CONFIG_AUDIO_PAD_ESP32_S2_KALUGA_V1_1
    ESP_LOGI(TAG, "aduio pad:v1.1");
    ret = true;
#endif

    /*!< touch PAD */
#ifdef CONFIG_TOUCH_PAD_ESP32_S2_KALUGA_V1_3
    ESP_LOGI(TAG, "touch pad:v1.3");
    ret = true;
#endif
#ifdef CONFIG_TOUCH_PAD_ESP32_S2_KALUGA_V1_2
    ESP_LOGI(TAG, "touch pad:v1.2");
    ret = true;
#endif
#ifdef CONFIG_TOUCH_PAD_ESP32_S2_KALUGA_V1_2
    ESP_LOGI(TAG, "touch pad:v1.1");
    ret = true;
#endif

    /*!< camera PAD */
#ifdef CONFIG_CAMERA_PAD_ESP32_S2_KALUGA_V1_3
    ESP_LOGI(TAG, "camera pad:v1.3");
    ret = true;
#endif
#ifdef CONFIG_CAMERA_PAD_ESP32_S2_KALUGA_V1_2
    ESP_LOGI(TAG, "camera pad:v1.2");
    ret = true;
#endif
#ifdef CONFIG_CAMERA_PAD_ESP32_S2_KALUGA_V1_1
    ESP_LOGI(TAG, "camera pad:v1.1");
    ret = true;
#endif

    /*!< LCD PAD */
#ifdef CONFIG_LCD_PAD_ESP32_S2_KALUGA_V1_3
    ESP_LOGI(TAG, "LED pad:v1.3");
    ret = true;
#endif
#ifdef CONFIG_LCD_PAD_ESP32_S2_KALUGA_V1_2
    ESP_LOGI(TAG, "LED pad:v1.2");
    ret = true;
#endif
#ifdef CONFIG_LCD_PAD_ESP32_S2_KALUGA_V1_1
    ESP_LOGI(TAG, "LED pad:v1.1");
    ret = true;
#endif

    if (ret == false) {
        ESP_LOGE(TAG, "No board is defined");
        return ESP_FAIL;
    } else {
        return ESP_OK;
    }
}