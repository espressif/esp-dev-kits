// Copyright 2020 Espressif Systems (Shanghai) Co. Ltd.
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
#ifndef __LCD_SSD1963_H__
#define __LCD_SSD1963_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "screen_driver.h"

//LCD分辨率设置
#define SSD_HOR_RESOLUTION		800		//LCD水平分辨率
#define SSD_VER_RESOLUTION		480		//LCD垂直分辨率
//LCD驱动参数设置
#define SSD_HOR_PULSE_WIDTH		1		//水平脉宽
#define SSD_HOR_BACK_PORCH		46		//水平前廊
#define SSD_HOR_FRONT_PORCH		210		//水平后廊

#define SSD_VER_PULSE_WIDTH		1		//垂直脉宽
#define SSD_VER_BACK_PORCH		23		//垂直前廊
#define SSD_VER_FRONT_PORCH		22		//垂直前廊
//如下几个参数，自动计算
#define SSD_HT	(SSD_HOR_RESOLUTION+SSD_HOR_BACK_PORCH+SSD_HOR_FRONT_PORCH)
#define SSD_HPS	(SSD_HOR_BACK_PORCH)
#define SSD_VT 	(SSD_VER_RESOLUTION+SSD_VER_BACK_PORCH+SSD_VER_FRONT_PORCH)
#define SSD_VPS (SSD_VER_BACK_PORCH)

/**
 * @brief   device initialization
 *
 * @param   lcd_conf configuration struct of ssd1963
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Fail
 */
esp_err_t lcd_ssd1963_init(const scr_controller_config_t *lcd_conf);

/**
 * @brief   Deinitial screen
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Fail
 */
esp_err_t lcd_ssd1963_deinit(void);

/**
 * @brief Get screen information
 * 
 * @param info Pointer to a scr_info_t structure.
 * 
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Fail
 */
esp_err_t lcd_ssd1963_get_info(scr_info_t *info);

/**
 * @brief Set screen direction of rotation
 *
 * @param dir Pointer to a scr_dir_t structure.
 * 
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Fail
 */
esp_err_t lcd_ssd1963_set_rotation(scr_dir_t dir);

/**
 * @brief Set screen window
 *
 * @param x0 Starting point in X direction
 * @param y0 Starting point in Y direction
 * @param x1 End point in X direction
 * @param y1 End point in Y direction
 * 
 * @return
 *      - ESP_OK on success
 *      - ESP_FAIL Failed
 */
esp_err_t lcd_ssd1963_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

/**
 * @brief Write a RAM data
 * 
 * @param color New color of a pixel
 * 
 * @return 
 *      - ESP_OK on success
 *      - ESP_FAIL Failed
 */
esp_err_t lcd_ssd1963_write_ram_data(uint16_t color);

/**
 * @brief Draw one pixel in screen with color
 * 
 * @param x X co-ordinate of set orientation
 * @param y Y co-ordinate of set orientation
 * @param color New color of the pixel
 * 
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Fail
 */
esp_err_t lcd_ssd1963_draw_pixel(uint16_t x, uint16_t y, uint16_t color);

/**
 * @brief Fill the pixels on LCD screen with bitmap
 * 
 * @param x Starting point in X direction
 * @param y Starting point in Y direction
 * @param w width of image in bitmap array
 * @param h height of image in bitmap array
 * @param bitmap pointer to bitmap array
 * 
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Fail
 */
esp_err_t lcd_ssd1963_draw_bitmap(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t *bitmap);



#ifdef __cplusplus
}
#endif

#endif
