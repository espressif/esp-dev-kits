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

#include "rgb_light.h"

static led_strip_t *rgb_light_handle = NULL;

esp_err_t rgb_light_driver_install(gpio_num_t rgb_tx_pin)
{
    rmt_config_t config = RMT_DEFAULT_CONFIG_TX(rgb_tx_pin, RMT_CHANNEL_0);
    config.clk_div = 2;
    esp_err_t ret = rmt_config(&config);
    if (ret != ESP_OK)  return ret;
    ret = rmt_driver_install(config.channel, 0, 0);
    if (ret != ESP_OK)  return ret;
    led_strip_config_t strip_config = LED_STRIP_DEFAULT_CONFIG(1, (led_strip_dev_t)config.channel);
    rgb_light_handle = led_strip_new_rmt_ws2812(&strip_config);
    if (!rgb_light_handle) {
        esp_err_t del_ret = rmt_driver_uninstall(RMT_CHANNEL_0);
        if (del_ret != ESP_OK) {
            abort();
        }
        return ESP_FAIL;
    }
    ret = rgb_light_handle->clear(rgb_light_handle, 0);
    if (ret != ESP_OK) {
        esp_err_t del_ret = led_strip_deinit(rgb_light_handle);
        if (del_ret != ESP_OK) {
            abort();
        }
        return ret;
    }
    vTaskDelay(pdMS_TO_TICKS(10));
    return ESP_OK;
}

esp_err_t rgb_light_set_characters(uint8_t red, uint8_t green, uint8_t blue)
{
    if (rgb_light_handle == NULL)   return ESP_ERR_INVALID_STATE;
    esp_err_t ret = rgb_light_handle->set_pixel(rgb_light_handle, 0, red, green, blue);
    if (ret != ESP_OK)  return ret;
    ret = rgb_light_handle->refresh(rgb_light_handle, 0);
    if (ret != ESP_OK)  return ret;
    return ESP_OK;
}
