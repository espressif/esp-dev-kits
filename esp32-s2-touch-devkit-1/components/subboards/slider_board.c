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

static const char *TAG = "Slider Board";

#define SLIDER_CHANNEL_NUM     6

static touch_slider_handle_t slider_handle;

static const touch_pad_t slider_channel[SLIDER_CHANNEL_NUM] = {
    TOUCH_PAD_NUM9,
    TOUCH_PAD_NUM11,
    TOUCH_PAD_NUM13,
    TOUCH_PAD_NUM5,
    TOUCH_PAD_NUM3,
    TOUCH_PAD_NUM1,
};

/**
 * Using finger slide from slider's beginning to the ending, and output the RAW channel signal, then calculate all the
 * channels sensitivity of the slider, and you can decrease or increase the detection sensitivity by adjusting the threshold divider
 * which locates in touch_slider_global_config_t. Please keep in mind that the real sensitivity totally depends on the
 * physical characteristics, if you want to decrease or increase the detection sensitivity, keep the ratio of those channels the same.
 * TODO: add graphical debug tool to analyse/generate the sensitivity
 */
static const float slider_sens[SLIDER_CHANNEL_NUM] = {
    0.217F,
    0.210F,
    0.211F,
    0.210F,
    0.205F,
    0.221F
};

void touch_slider_callback(touch_slider_handle_t handle, touch_slider_message_t *message, void *arg)
{
    if (handle != slider_handle) {
        return;
    }
    if (message->event == TOUCH_SLIDER_EVT_ON_PRESS) {
        ESP_LOGI(TAG, "Slider pressed, position: %d ", message->position);
    } else if (message->event == TOUCH_SLIDER_EVT_ON_CALCULATION) {
        ESP_LOGI(TAG, "Position: %d ", message->position);
    } else {
        ESP_LOGI(TAG, "Slider released, position: %d ", message->position);
    }
    digital_tube_show_x(message->position);
    rgb_light_set_characters(message->position * 2.5, 0, 0);
}

esp_err_t slider_board_driver_install(void)
{
    touch_elem_global_config_t global_config = TOUCH_ELEM_GLOBAL_DEFAULT_CONFIG();
    esp_err_t ret = touch_element_install(&global_config);
    if (ret != ESP_OK)  return ret;
    touch_slider_global_config_t slider_global_config = TOUCH_SLIDER_GLOBAL_DEFAULT_CONFIG();
    ret = touch_slider_install(&slider_global_config);
    if (ret != ESP_OK) {
        touch_element_uninstall();
        return ret;
    }
    touch_slider_config_t slider_config = {
        .channel_array = slider_channel,
        .sensitivity_array = slider_sens,
        .channel_num = SLIDER_CHANNEL_NUM,
        .position_range = 99
    };
    ret = touch_slider_create(&slider_config, &slider_handle);
    if (ret != ESP_OK)  goto cleanup;;
    ret = touch_slider_subscribe_event(slider_handle, TOUCH_ELEM_EVENT_ON_PRESS | TOUCH_ELEM_EVENT_ON_RELEASE | TOUCH_ELEM_EVENT_ON_CALCULATION, NULL);
    if (ret != ESP_OK)  goto cleanup;;
    ret = touch_slider_set_dispatch_method(slider_handle, TOUCH_ELEM_DISP_CALLBACK);
    if (ret != ESP_OK)  goto cleanup;;
    ret = touch_slider_set_callback(slider_handle, touch_slider_callback);
    if (ret != ESP_OK)  goto cleanup;;
    ret = touch_element_start();
    if (ret != ESP_OK)  goto cleanup;;
    return ESP_OK;

cleanup:
    touch_slider_uninstall();
    touch_element_uninstall();
    return ret;
}

esp_err_t slider_board_driver_uninstall(void)
{
    esp_err_t ret = touch_element_stop();
    if (ret != ESP_OK)  return ret;
    touch_slider_delete(slider_handle);
    touch_slider_uninstall();
    touch_element_uninstall();
    return ESP_OK;
}
