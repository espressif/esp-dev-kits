/**
 * @file bsp_lcd.h
 * @brief Header of bsp_lcd.
 * @version 0.1
 * @date 2021-01-14
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

#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "bsp_board.h"
#include "screen_driver.h"

#include "tca9554.h"
#include "bsp_ext_io.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize LCD
 * 
 * @return esp_err_t Initialize state
 */
esp_err_t bsp_lcd_init(void);

/**
 * @brief Flush specified area of screen.
 * 
 * @param x Horizon coordinate of start point [0..Width - 1]
 * @param y Vertical coordinate of start point [0..Height - 1]
 * @param w Width of flush area [0..Width - x]
 * @param h Height of flush area [0..Height - y]
 * @param data Pixel data pointer
 * @return esp_err_t 
 *      ESO_OK : Flush successfully
 */
esp_err_t bsp_lcd_flush(uint16_t x, uint16_t y, uint16_t w, uint16_t h, void *data);

#ifdef __cplusplus
}
#endif
