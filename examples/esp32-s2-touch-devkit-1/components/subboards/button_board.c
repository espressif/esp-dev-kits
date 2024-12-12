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

static const char *TAG = "Button Board";

#define TOUCH_BUTTON_NUM     3

static touch_button_handle_t button_handle[TOUCH_BUTTON_NUM];

static const touch_pad_t button_channel_array[TOUCH_BUTTON_NUM] = {
    TOUCH_PAD_NUM7,
    TOUCH_PAD_NUM9,
    TOUCH_PAD_NUM11,
};

static const float button_channel_sens_array[TOUCH_BUTTON_NUM] = {
    0.5F,
    0.5F,
    0.5F
};

void touch_button_callback(touch_button_handle_t handle, touch_button_message_t *message, void *arg)
{
    uint32_t button_num = (uint32_t)arg;
    if (message->event == TOUCH_BUTTON_EVT_ON_PRESS) {
        buzzer_set_voice(1);
        if (handle == button_handle[0]) {
            rgb_light_set_characters(200, 0, 0);
        } else if (handle == button_handle[1]) {
            rgb_light_set_characters(0, 200, 0);
        } else if (handle == button_handle[2]) {
            rgb_light_set_characters(0, 0, 200);
        } else {
            rgb_light_set_characters(200, 200, 200);
        }
        digital_tube_show_x(button_num);
        ESP_LOGI(TAG, "Button[%d] pressed", button_num);
    } else if (message->event == TOUCH_BUTTON_EVT_ON_RELEASE) {
        ESP_LOGI(TAG, "Button[%d] released", button_num);
        rgb_light_set_characters(0, 0, 0);
        buzzer_set_voice(0);
    }
}

esp_err_t button_board_driver_install(void)
{
    touch_elem_global_config_t global_config = TOUCH_ELEM_GLOBAL_DEFAULT_CONFIG();
    esp_err_t ret = touch_element_install(&global_config);
    if (ret != ESP_OK)  return ret;
    touch_elem_waterproof_config_t waterproof_config = {
        .guard_channel = TOUCH_PAD_NUM13,
        .guard_sensitivity = 0.2F  //The guard sensor sensitivity has to be explored in experiments
    };
    ret = touch_element_waterproof_install(&waterproof_config);
    if (ret != ESP_OK) {
        touch_element_uninstall();
        return ret;
    }
    touch_button_global_config_t button_global_config = TOUCH_BUTTON_GLOBAL_DEFAULT_CONFIG();
    ret = touch_button_install(&button_global_config);
    if (ret != ESP_OK) {
        touch_element_waterproof_uninstall();
        touch_element_uninstall();
        return ret;
    }
    for (int i = 0; i < TOUCH_BUTTON_NUM; i++) {
        touch_button_config_t button_config = {
            .channel_num = button_channel_array[i],
            .channel_sens = button_channel_sens_array[i]
        };
        ret = touch_button_create(&button_config, &button_handle[i]);
        if (ret != ESP_OK)  goto cleanup;
        ret = touch_button_subscribe_event(button_handle[i], TOUCH_ELEM_EVENT_ON_PRESS | TOUCH_ELEM_EVENT_ON_RELEASE, (void *) (i + 1));
        if (ret != ESP_OK)  goto cleanup;
        ret = touch_button_set_dispatch_method(button_handle[i], TOUCH_ELEM_DISP_CALLBACK);
        if (ret != ESP_OK)  goto cleanup;
        ret = touch_button_set_callback(button_handle[i], touch_button_callback);
        if (ret != ESP_OK)  goto cleanup;
        ret = touch_element_waterproof_add(button_handle[i]);
        if (ret != ESP_OK)  goto cleanup;
    }
    ret = touch_element_start();
    if (ret != ESP_OK)  goto cleanup;
    return ESP_OK;

cleanup:
    touch_button_uninstall();
    touch_element_waterproof_uninstall();
    touch_element_uninstall();
    return ret;
}

esp_err_t button_board_driver_uninstall(void)
{
    esp_err_t ret = touch_element_stop();
    if (ret != ESP_OK)  return ret;
    for (int i = 0; i < TOUCH_BUTTON_NUM; i++) {
        touch_button_delete(button_handle[i]);
    }
    touch_button_uninstall();
    touch_element_waterproof_uninstall();
    touch_element_uninstall();
    return ESP_OK;
}
