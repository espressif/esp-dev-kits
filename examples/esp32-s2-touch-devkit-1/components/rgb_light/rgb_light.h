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

#pragma once

#include "driver/rmt.h"
#include "led_strip.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   RGB light controller (ws2812) initialization
 *
 * This function initializes RGB light controller driver by using RMT peripheral
 *
 * @param rgb_tx_pin    RMT peripheral transmission pin number
 *
 * @return
 *      - ESP_OK: Successfully initialized RGB light controller
 *      - ESP_FAIL: Insufficient memory or other esp-idf RMT driver errors
 */
esp_err_t rgb_light_driver_install(gpio_num_t rgb_tx_pin);

/**
 * @brief   RGB light configures characters(color)
 *
 * @return
 *      - ESP_OK: Successfully configured RMT light color
 *      - ESP_ERR_INVALID_STATE: RGB light driver is not initialized
 */
esp_err_t rgb_light_set_characters(uint8_t red, uint8_t green, uint8_t blue);

#ifdef __cplusplus
}
#endif
