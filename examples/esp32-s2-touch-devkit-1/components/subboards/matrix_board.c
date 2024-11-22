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

#include "esp_log.h"
#include "subboards.h"

static const char *TAG = "Matrix Board";

#define X_AXIS_CHANNEL_NUM     4  //Matrix x-axis
#define Y_AXIS_CHANNEL_NUM     3  //Matrix y-axis

static touch_matrix_handle_t matrix_handle;

static const touch_pad_t x_axis_channel[X_AXIS_CHANNEL_NUM] = {
    TOUCH_PAD_NUM11,
    TOUCH_PAD_NUM13,
    TOUCH_PAD_NUM3,
    TOUCH_PAD_NUM1,
};

static const touch_pad_t y_axis_channel[Y_AXIS_CHANNEL_NUM] = {
    TOUCH_PAD_NUM5,
    TOUCH_PAD_NUM7,
    TOUCH_PAD_NUM9,
};

static const float x_axis_channel_sens[X_AXIS_CHANNEL_NUM] = {
    0.2F,
    0.2F,
    0.2F,
    0.2F
};

static const float y_axis_channel_sens[Y_AXIS_CHANNEL_NUM] = {
    0.2F,
    0.2F,
    0.2F
};

const uint8_t button_num_lookup_table[] = {10, 11, 12, 7, 8, 9, 4, 5, 6, 1, 2, 3};

void touch_matrix_callback(touch_matrix_handle_t handle, touch_matrix_message_t *message, void *arg)
{
    if (handle != matrix_handle) {
        return;
    }

    uint32_t button_num = button_num_lookup_table[message->position.index];
    if (message->event == TOUCH_MATRIX_EVT_ON_PRESS) {
        ESP_LOGI(TAG, "Matrix button[%d] pressed", button_num);
        buzzer_set_voice(1);
        digital_tube_show_x(button_num);
        uint32_t color_select = button_num % 3;
        switch (color_select) {
            case 0:
                rgb_light_set_characters(button_num * 20, 0, 0);
                break;
            case 1:
                rgb_light_set_characters(0, button_num * 20, 0);
                break;
            case 2:
                rgb_light_set_characters(0, 0, button_num * 20);
                break;
            default:
                break;
        }
    } else if (message->event == TOUCH_MATRIX_EVT_ON_RELEASE) {
        ESP_LOGI(TAG, "Matrix button[%d] released", button_num);
        buzzer_set_voice(0);
    }
}

esp_err_t matrix_board_driver_install(void)
{
    touch_elem_global_config_t global_config = TOUCH_ELEM_GLOBAL_DEFAULT_CONFIG();
    esp_err_t ret = touch_element_install(&global_config);
    if (ret != ESP_OK)  return ret;
    touch_elem_waterproof_config_t waterproof_config = {
        .guard_channel = TOUCH_WATERPROOF_GUARD_NOUSE,
        .guard_sensitivity = 0.0F
    };
    ret = touch_element_waterproof_install(&waterproof_config);
    if(ret != ESP_OK) {
        touch_element_uninstall();
        return ret;
    }
    touch_matrix_global_config_t matrix_global_config = TOUCH_MATRIX_GLOBAL_DEFAULT_CONFIG();
    ret = touch_matrix_install(&matrix_global_config);
    if (ret != ESP_OK) {
        touch_element_waterproof_uninstall();
        touch_element_uninstall();
    }
    /*< Create Touch Matrix Button */
    touch_matrix_config_t matrix_config = {
        .x_channel_array = x_axis_channel,
        .y_channel_array = y_axis_channel,
        .x_sensitivity_array = x_axis_channel_sens,
        .y_sensitivity_array = y_axis_channel_sens,
        .x_channel_num = X_AXIS_CHANNEL_NUM,
        .y_channel_num = Y_AXIS_CHANNEL_NUM
    };
    ret = touch_matrix_create(&matrix_config, &matrix_handle);
    if (ret != ESP_OK)  goto cleanup;
    ret = touch_matrix_subscribe_event(matrix_handle, TOUCH_ELEM_EVENT_ON_PRESS | TOUCH_ELEM_EVENT_ON_RELEASE, NULL);
    if (ret != ESP_OK)  goto cleanup;
    ret = touch_matrix_set_dispatch_method(matrix_handle, TOUCH_ELEM_DISP_CALLBACK);
    if (ret != ESP_OK)  goto cleanup;
    ret = touch_matrix_set_callback(matrix_handle, touch_matrix_callback);
    if (ret != ESP_OK)  goto cleanup;
    ret = touch_element_start();
    if (ret != ESP_OK)  goto cleanup;
    return ESP_OK;

cleanup:
    touch_element_waterproof_uninstall();
    touch_element_uninstall();
    return ret;
}

esp_err_t matrix_board_driver_uninstall(void)
{
    esp_err_t ret = touch_element_stop();
    if (ret != ESP_OK)  return ret;
    touch_matrix_delete(matrix_handle);
    touch_matrix_uninstall();
    touch_element_waterproof_uninstall();
    touch_element_uninstall();
    return ESP_OK;
}
