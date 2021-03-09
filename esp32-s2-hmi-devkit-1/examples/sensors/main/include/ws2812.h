/**
 * @file ws2812.h
 * @brief WS2812 APIs header
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

#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_log.h"

#include "bsp_board.h"

#include "driver/rmt.h"
#include "led_strip.h"

esp_err_t ws2812_init(void);
esp_err_t ws2812_set_rgb(uint8_t index, uint8_t r, uint8_t g, uint8_t b);
esp_err_t ws2812_refresh(void);

void ws2812_set_r(uint8_t r);
void ws2812_set_g(uint8_t g);
void ws2812_set_b(uint8_t b);

uint8_t ws2812_get_r(void);
uint8_t ws2812_get_g(void);
uint8_t ws2812_get_b(void);
