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

#include "driver/gpio.h"
#include "driver/spi_master.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t clk_fre;
    uint8_t pin_clk;
    uint8_t pin_mosi;
    uint8_t pin_dc;
    uint8_t pin_cs;
    uint8_t pin_rst;
    uint8_t pin_bk;
    uint8_t horizontal;
    uint32_t max_buffer_size; // DMA used
    bool swap_data;
} lcd_config_t;

/**
 * @brief  lcd restart
 */
void lcd_rst();

/**
 * @brief write data in LCD
 *
 * @param data spi data
 * @param len len of data 
 */
void lcd_write_data(uint8_t *data, size_t len);

/**
 * @brief set lcd address from start to end
 *
 * @param x_start start address of x
 * @param y_start start address of y 
 * @param x_end end address of x
 * @param y_end end address of y
 */
void lcd_set_index(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end);

/**
 * @brief Initialize the LCD
 *
 * @param config lcd config about pin
 */
int lcd_init(lcd_config_t *config);

#ifdef __cplusplus
}
#endif