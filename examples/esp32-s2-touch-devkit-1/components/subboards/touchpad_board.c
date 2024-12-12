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

static const char *TAG = "Touchpad Board";

#define TOUCH_SLIDER1_SUM    7
#define TOUCH_SLIDER2_SUM    6

static touch_board_handle_t board_handle = NULL;

static const touch_pad_t channel1_array[TOUCH_SLIDER1_SUM] = {
    TOUCH_PAD_NUM1,
    TOUCH_PAD_NUM3,
    TOUCH_PAD_NUM5,
    TOUCH_PAD_NUM7,
    TOUCH_PAD_NUM9,
    TOUCH_PAD_NUM11,
    TOUCH_PAD_NUM13,
};

/**
 * Please refer to the slider sensitivity geter method
 * TODO: add graphical debug tool to analyse/generate the sensitivity
 */
static const float channel1_sens_array[TOUCH_SLIDER1_SUM] = {
    0.115F,
    0.115F,
    0.115F,
    0.115F,
    0.115F,
    0.115F,
    0.115F
};

static const touch_pad_t channel2_array[TOUCH_SLIDER2_SUM] = {
    TOUCH_PAD_NUM8,
    TOUCH_PAD_NUM6,
    TOUCH_PAD_NUM4,
    TOUCH_PAD_NUM2,
    TOUCH_PAD_NUM10,
    TOUCH_PAD_NUM12,
};

static const float channel2_sens_array[TOUCH_SLIDER2_SUM] = {
    0.115F,
    0.115F,
    0.115F,
    0.115F,
    0.115F,
    0.115F
};

static void touch_board_handler(touch_board_handle_t handle, touch_board_message_t *message, void *arg)
{
    digital_tube_show_x(message->position.x_axis);
    digital_tube_show_y(99 - message->position.y_axis);
    rgb_light_set_characters(message->position.x_axis, 99 -  message->position.y_axis, 0);
    if(message->event == TOUCH_BOARD_EVT_ON_PRESS) {
        ESP_LOGI(TAG, "Touchpad pressed, position: [%d, %d]", message->position.x_axis, 99 -  message->position.y_axis);
    } else if (message->event == TOUCH_BOARD_EVT_ON_CALCULATION) {
        ESP_LOGI(TAG, "Position: [%d, %d]", message->position.x_axis, 99 -  message->position.y_axis);
    } else {
        ESP_LOGI(TAG, "Touchpad released, position: [%d, %d]", message->position.x_axis, 99 -  message->position.y_axis);
    }
}

esp_err_t touchpad_board_driver_install(void)
{
    touch_elem_global_config_t global_config = TOUCH_ELEM_GLOBAL_DEFAULT_CONFIG();
    esp_err_t ret = touch_element_install(&global_config);
    if(ret != ESP_OK)   return ret;
    touch_board_global_config_t board_global_config = {
        .horizontal_global_config =  TOUCH_SLIDER_GLOBAL_DEFAULT_CONFIG(),
        .vertical_global_config = TOUCH_SLIDER_GLOBAL_DEFAULT_CONFIG()
    };
    ret = touch_board_install(&board_global_config);
    if(ret != ESP_OK) {
        touch_element_uninstall();
        return ret;
    }

    touch_board_config_t board_config = {
        .horizontal_config.channel_array = channel1_array,
        .horizontal_config.sensitivity_array = channel1_sens_array,
        .horizontal_config.channel_num = TOUCH_SLIDER1_SUM,
        .horizontal_config.position_range = 99,
        .vertical_config.channel_array = channel2_array,
        .vertical_config.sensitivity_array = channel2_sens_array,
        .vertical_config.channel_num = TOUCH_SLIDER2_SUM,
        .vertical_config.position_range = 99,
    };
    ret = touch_board_create(&board_config, &board_handle);
    if (ret != ESP_OK)  goto cleanup;
    ret = touch_board_subscribe_event(board_handle, TOUCH_ELEM_EVENT_ON_PRESS | TOUCH_ELEM_EVENT_ON_RELEASE | TOUCH_ELEM_EVENT_ON_CALCULATION, NULL);
    if (ret != ESP_OK)  goto cleanup;
    ret = touch_board_set_dispatch_method(board_handle, TOUCH_ELEM_DISP_CALLBACK);
    if (ret != ESP_OK)  goto cleanup;
    ret = touch_board_set_callback(board_handle, touch_board_handler);
    if (ret != ESP_OK)  goto cleanup;
    ret = touch_element_start();
    if (ret != ESP_OK)  goto cleanup;
    return ESP_OK;

cleanup:
    touch_board_uninstall();
    touch_element_uninstall();
    return ret;
}

esp_err_t touchpad_board_driver_uninstall(void)
{
    esp_err_t ret = touch_element_stop();
    if (ret != ESP_OK)  return ret;
    touch_board_delete(board_handle);
    touch_board_uninstall();
    touch_element_uninstall();
    return ESP_OK;
}
