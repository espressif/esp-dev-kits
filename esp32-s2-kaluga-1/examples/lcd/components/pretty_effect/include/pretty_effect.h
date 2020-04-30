// Copyright 2019 Espressif Systems (Shanghai) PTE LTD
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

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "esp_err.h"

/*!< Kaluga matching to the LCD is 320 * 240 */
#define LCD_WIDTH  320 /*!< width of LCD */
#define LCD_HEIGHT 240 /*!< height of LCD */

/**
 * @brief Calculate the effect for a bunch of lines.
 *
 * @param dest Destination for the pixels. Assumed to be LINECT * 320 16-bit pixel values.
 * @param line Starting line of the chunk of lines.
 * @param frame Current frame, used for animation
 * @param linect Amount of lines to calculate
 */
void pretty_effect_calc_lines(uint16_t *dest, int line, int frame, int linect);

/**
 * @brief static the effect for a bunch of lines.
 *
 * @param dest Destination for the pixels. Assumed to be LINECT * 320 16-bit pixel values.
 * @param line Starting line of the chunk of lines.
 * @param frame Current frame, used for animation.
 * @param linect Amount of lines to calculate.
 */
void pretty_effect_static_lines(uint16_t *dest, int line, int frame, int linect);

/**
 * @brief Initialize the effect
 * 
 * @param image_jpg_start jpeg file address.
 * @param width width of jpeg file.
 * @param height height of jpeg file
 * 
 * @return ESP_OK on success, an error from the jpeg decoder otherwise.
 */
esp_err_t pretty_effect_init(const uint8_t *image_jpg_start, int width, int height);

/**
 * @brief Initialize the effect
 *
 * @param image_jpg_start jpeg file address.
 * @param width width of jpeg file.
 * @param height height of jpeg file
 * 
 * @return ESP_OK on success
 * @return ESP_FAIL on fail.
 */
esp_err_t pretty_effect_deinit(const uint8_t *image_jpg_start, int width, int height);

#ifdef __cplusplus
}
#endif

