/**
 * @file ws2812.c
 * @brief WS2812 RGB LED APIs.
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

#include "ws2812.h"

#define LED_COUNT   (1)

static const char *TAG = "ws2812";
static led_strip_t *strip = NULL;

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} led_color_t;

static led_color_t ws2812 = { 0, 0, 0};

esp_err_t ws2812_init(void)
{
    if (false == FUNC_RMT_EN) {
        ESP_LOGE(TAG, "This dev board does not support WS2812!");
        return ESP_FAIL;
    }

    rmt_config_t config = RMT_DEFAULT_CONFIG_TX(GPIO_RMT, RMT_CHANNEL_0);
    config.clk_div = 2;
    ESP_ERROR_CHECK(rmt_config(&config));
    ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));
    
    led_strip_config_t strip_config = LED_STRIP_DEFAULT_CONFIG(GPIO_RMT, (led_strip_dev_t)config.channel);
    strip = led_strip_new_rmt_ws2812(&strip_config);
    if (!strip) {
        ESP_LOGE(TAG, "install WS2812 driver failed");
    }

    for (uint8_t i = 0; i < LED_COUNT; i++) {
        ESP_ERROR_CHECK(strip->set_pixel(strip, i, ws2812.r, ws2812.g, ws2812.b));
    }

    ESP_ERROR_CHECK(strip->refresh(strip, portMAX_DELAY));

    return ESP_OK;
}

esp_err_t ws2812_set_rgb(uint8_t index, uint8_t r, uint8_t g, uint8_t b)
{
    ws2812.r = r;
    ws2812.g = g;
    ws2812.b = b;
    return strip->set_pixel(strip, index, r, g, b);
}

esp_err_t ws2812_refresh(void)
{
    return strip->refresh(strip, 0);
}
