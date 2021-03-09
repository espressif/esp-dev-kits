/**
 * @file bsp_hmi_dev_kit.h
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

#ifdef CONFIG_ESP32_S2_HMI_DEVKIT_BOARD

#include "driver/gpio.h"

/**
 * @brief ESP32-S2-HMI-DevKit LCD GPIO defination and config
 * 
 */
#define FUNC_LCD_EN     (1)
#define LCD_BIT_WIDTH   (16)

#define GPIO_LCD_BL     (GPIO_NUM_NC)
#define GPIO_LCD_CS     (GPIO_NUM_NC)
#define GPIO_LCD_EN     (GPIO_NUM_NC)
#define GPIO_LCD_RS     (GPIO_NUM_33)
#define GPIO_LCD_WR     (GPIO_NUM_34)

#define GPIO_LCD_D00    (GPIO_NUM_1)
#define GPIO_LCD_D01    (GPIO_NUM_10)
#define GPIO_LCD_D02    (GPIO_NUM_2)
#define GPIO_LCD_D03    (GPIO_NUM_11)
#define GPIO_LCD_D04    (GPIO_NUM_3)
#define GPIO_LCD_D05    (GPIO_NUM_12)
#define GPIO_LCD_D06    (GPIO_NUM_4)
#define GPIO_LCD_D07    (GPIO_NUM_13)
#define GPIO_LCD_D08    (GPIO_NUM_5)
#define GPIO_LCD_D09    (GPIO_NUM_14)
#define GPIO_LCD_D10    (GPIO_NUM_6)
#define GPIO_LCD_D11    (GPIO_NUM_15)
#define GPIO_LCD_D12    (GPIO_NUM_7)
#define GPIO_LCD_D13    (GPIO_NUM_16)
#define GPIO_LCD_D14    (GPIO_NUM_8)
#define GPIO_LCD_D15    (GPIO_NUM_17)

#define LCD_WIDTH       (800)
#define LCD_HEIGHT      (480)

/**
 * @brief ESP32-S2-HMI-DevKit I2C GPIO defineation
 * 
 */
#define FUNC_I2C_EN     (1)
#define GPIO_I2C_SCL    (GPIO_NUM_36)
#define GPIO_I2C_SDA    (GPIO_NUM_35)

/**
 * @brief ESP32-S2-HMI-DevKit SPI GPIO defination
 * 
 */
#define FUNC_SPI_EN     (1)
#define GPIO_SPI_CS     (GPIO_NUM_42)
#define GPIO_SPI_MISO   (GPIO_NUM_39)
#define GPIO_SPI_MOSI   (GPIO_NUM_41)
#define GPIO_SPI_SCLK   (GPIO_NUM_40)

/**
 * @brief ESP32-S2-HMI-DevKit RMT GPIO defination
 * 
 */
#define FUNC_RMT_EN     (1)
#define GPIO_RMT        (GPIO_NUM_21)

/**
 * @brief ESP32-S2-HMI-DevKit TWAI GPIO defination
 * 
 */
#define FUNC_TWAI_EN        (1)
#define GPIO_TWAI_BUS_OFF   (GPIO_NUM_NC)
#define GPIO_TWAI_CLKOUT    (GPIO_NUM_NC)
#define GPIO_TWAI_RX        (GPIO_NUM_37)
#define GPIO_TWAI_TX        (GPIO_NUM_38)

/**
 * @brief ESP32-S2-HMI-DevKit ADC GPIO defination
 * 
 */
#define FUNC_ADC_EN     (1)
#define GPIO_ADC_IN     (GPIO_NUM_9)

/**
 * @brief ESP32-S2-HMI-DevKit DAC GPIO defination
 * 
 */
#define FUNC_DAC_EN     (1)
#define GPIO_DAC_CH     (DAC_CHANNEL_2)
#define GPIO_DAC_OUT    (GPIO_NUM_18)

#endif /* CONFIG_ESP32_S2_HMI_DEVKIT_BOARD */
