/**
 * @file bsp_custom_board.h
 * @brief 
 * @version 0.1
 * @date 2021-02-26
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

#ifdef CONFIG_HMI_BOARD_CUSTOM

#include "driver/gpio.h"

/**
 * @brief ESP32-S2-HMI-DevKit SPI GPIO defination
 * 
 */
#define FUNC_LCD_EN     (0)
#define LCD_BIT_WIDTH   (16)

#define GPIO_LCD_CS     (GPIO_NUM_NC)
#define GPIO_LCD_EN     (GPIO_NUM_NC)
#define GPIO_LCD_RS     (CONFIG_LCD_RS_GPIO)
#define GPIO_LCD_WR     (CONFIG_LCD_WR_GPIO)

#define GPIO_LCD_D00    (CONFIG_LCD_D00_GPIO)
#define GPIO_LCD_D01    (CONFIG_LCD_D01_GPIO)
#define GPIO_LCD_D02    (CONFIG_LCD_D02_GPIO)
#define GPIO_LCD_D03    (CONFIG_LCD_D03_GPIO)
#define GPIO_LCD_D04    (CONFIG_LCD_D04_GPIO)
#define GPIO_LCD_D05    (CONFIG_LCD_D05_GPIO)
#define GPIO_LCD_D06    (CONFIG_LCD_D06_GPIO)
#define GPIO_LCD_D07    (CONFIG_LCD_D07_GPIO)
#define GPIO_LCD_D08    (CONFIG_LCD_D08_GPIO)
#define GPIO_LCD_D09    (CONFIG_LCD_D09_GPIO)
#define GPIO_LCD_D10    (CONFIG_LCD_D10_GPIO)
#define GPIO_LCD_D11    (CONFIG_LCD_D11_GPIO)
#define GPIO_LCD_D12    (CONFIG_LCD_D12_GPIO)
#define GPIO_LCD_D13    (CONFIG_LCD_D13_GPIO)
#define GPIO_LCD_D14    (CONFIG_LCD_D14_GPIO)
#define GPIO_LCD_D15    (CONFIG_LCD_D15_GPIO)

/**
 * @brief ESP32-S2-HMI-DevKit I2C GPIO defineation
 * 
 */
#define FUNC_I2C_EN     (0)
#define GPIO_I2C_SCL    (CONFIG_I2C_SCL_GPIO)
#define GPIO_I2C_SDA    (CONFIG_I2C_SDA_GPIO)

/**
 * @brief ESP32-S2-HMI-DevKit SPI GPIO defination
 * 
 */
#define FUNC_SPI_EN     (0)
#define GPIO_SPI_CS     (CONFIG_SPI_CS_GPIO)
#define GPIO_SPI_MISO   (CONFIG_SPI_MISO_GPIO)
#define GPIO_SPI_MOSI   (CONFIG_SPI_MOSI_GPIO)
#define GPIO_SPI_SCLK   (CONFIG_SPI_SCLK_GPIO)

/**
 * @brief ESP32-S2-HMI-DevKit RMT GPIO defination
 * 
 */
#define FUNC_RMT_EN     (0)
#define GPIO_RMT        (GPIO_NUM_21)

/**
 * @brief ESP32-S2-HMI-DevKit TWAI GPIO defination
 * 
 */
#define FUNC_TWAI_EN        (0)
#define GPIO_TWAI_BUS_OFF   (GPIO_NUM_NC)
#define GPIO_TWAI_CLKOUT    (GPIO_NUM_NC)
#define GPIO_TWAI_RX        (CONFIG_TWAI_RX_GPIO)
#define GPIO_TWAI_TX        (CONFIG_TWAI_TX_GPIO)

/**
 * @brief ESP32-S2-HMI-DevKit ADC GPIO defination
 * 
 */
#define FUNC_ADC_EN     (0)
#define GPIO_ADC_IN     (GPIO_NUM_9)

/**
 * @brief ESP32-S2-HMI-DevKit DAC GPIO defination
 * 
 */
#define FUNC_DAC_EN     (0)
#define GPIO_DAC_CH     (DAC_CHANNEL_2)
#define GPIO_DAC_OUT    (GPIO_NUM_18)

#endif /* CONFIG_HMI_BOARD_CUSTOM */
