// Copyright 2021 Espressif Systems (Shanghai) PTE LTD
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

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "esp_log.h"

#include "buzzer.h"
#include "subboards.h"
#include "rgb_light.h"
#include "board_detect.h"
#include "digital_tube.h"

static const char *TAG = "Touch Demo";

static inline void subboard_install_prepare(uint8_t reload_time, uint8_t x_num, uint8_t y_num)
{
    digital_tube_show_reload(reload_time);
    digital_tube_show_x(x_num);
    digital_tube_show_y(y_num);
}

static void board_detect_and_setup(void *arg)
{
    esp_err_t ret;
    (void) arg;
    subboard_type_t old_subboard = BOARD_NOTHING;
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(100));
        subboard_type_t now_subboard = board_detect_get_subboard();
        if (now_subboard == old_subboard) {
            continue;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));  //Waiting for the the subboard has been completely plugged in
        now_subboard = board_detect_get_subboard();
        if (now_subboard == old_subboard) {
            continue;
        }
        switch (now_subboard) {
            case BOARD_BUTTON:
                ESP_LOGI(TAG, "Button sub-board plug in");
                subboard_install_prepare(30, 0, 0);
                ret = button_board_driver_install();
                if (ret != ESP_OK)  {
                    ESP_LOGE(TAG, "Button sub-board initialization failed, ret:%d", ret);
                    continue;
                }
                break;
            case BOARD_SLIDER:
                ESP_LOGI(TAG, "Slider sub-board plug in");
                subboard_install_prepare(30, 0, 0);
                ret = slider_board_driver_install();
                if (ret != ESP_OK)  {
                    ESP_LOGE(TAG, "Slider sub-board initialization failed, ret:%d", ret);
                    continue;
                }
                break;
            case BOARD_MATRIX:
                ESP_LOGI(TAG, "Matrix sub-board plug in");
                subboard_install_prepare(30, 0, 0);
                ret = matrix_board_driver_install();
                if (ret != ESP_OK)  {
                    ESP_LOGE(TAG, "Matrix sub-board initialization failed, ret:%d", ret);
                    continue;
                }
                break;
            case BOARD_PROXIMITY:
                ESP_LOGI(TAG, "Proximity sub-board plug in");
                ret = proximity_board_driver_install();
                if (ret != ESP_OK)  {
                    ESP_LOGE(TAG, "Proximity sub-board initialization failed, ret:%d", ret);
                    continue;
                }
                break;
            case BOARD_TOUCHPAD:
                ESP_LOGI(TAG, "Touchpad sub-board plug in");
                subboard_install_prepare(30, 0, 0);
                ret = touchpad_board_driver_install();
                if (ret != ESP_OK)  {
                    ESP_LOGE(TAG, "Touchpad sub-board initialization failed, ret:%d", ret);
                    continue;
                }
                break;
            default:
                if (old_subboard == BOARD_BUTTON) {
                    ESP_LOGI(TAG, "Button sub-board plug out");
                    ret = button_board_driver_uninstall();
                } else if (old_subboard == BOARD_SLIDER) {
                    ESP_LOGI(TAG, "Slider sub-board plug out");
                    ret = slider_board_driver_uninstall();
                } else if (old_subboard == BOARD_MATRIX) {
                    ret = matrix_board_driver_uninstall();
                    ESP_LOGI(TAG, "Matrix sub-board plug out");
                } else if (old_subboard == BOARD_PROXIMITY) {
                    ret = proximity_board_driver_uninstall();
                    ESP_LOGI(TAG, "Proximity sub-board plug out");
                } else if (old_subboard == BOARD_TOUCHPAD) {
                    ret = touchpad_board_driver_uninstall();
                    ESP_LOGI(TAG, "Touchpad sub-board plug out");
                } else {
                    ret = ESP_OK;
                }
                if (ret != ESP_OK) {
                    abort();
                }
                digital_tube_clear_all();
                rgb_light_set_characters(0, 0, 0);
                break;
        }
        old_subboard = now_subboard;
    }
}

void app_main(void)
{
    digital_tube_driver_install(I2C_NUM_0, GPIO_NUM_37, GPIO_NUM_38);
    buzzer_driver_install(GPIO_NUM_45);
    rgb_light_driver_install(GPIO_NUM_36);
    board_detect_driver_install();

    digital_tube_enable();
    subboard_install_prepare(50, 0, 0);

    xTaskCreate(board_detect_and_setup, "board_detect_and_setup", 4 * 1024, NULL, 10, NULL);
}
