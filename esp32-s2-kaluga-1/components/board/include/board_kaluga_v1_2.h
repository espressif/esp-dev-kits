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

/*
 *
 * 
 *  ESP32-S2-Kaluga-1  v1.2
 * 
 *  
 */

/*!< Touch PAD */
#ifdef CONFIG_TOUCH_PAD_ESP32_S2_KALUGA_V1_2
#define TOUCH_BUTTON_PLAY     TOUCH_PAD_NUM2
#define TOUCH_BUTTON_PHOTO    TOUCH_PAD_NUM6
#define TOUCH_BUTTON_NETWORK  TOUCH_PAD_NUM11
#define TOUCH_BUTTON_RECORD   TOUCH_PAD_NUM5
#define TOUCH_BUTTON_VOLUP    TOUCH_PAD_NUM1
#define TOUCH_BUTTON_VOLDOWN  TOUCH_PAD_NUM3
#define TOUCH_BUTTON_GUARD    TOUCH_PAD_NUM4
#define TOUCH_SHELED_ELECT    TOUCH_PAD_NUM14
#endif 

/*!< Audio PAD */
#ifdef CONFIG_AUDIO_PAD_ESP32_S2_KALUGA_V1_2
#define I2S_SCLK          (GPIO_NUM_18)
#define I2S_MCLK          (GPIO_NUM_35)
#define I2S_LCLK          (GPIO_NUM_17)
#define I2S_DOUT          (GPIO_NUM_12) 
#define I2S_DSIN          (GPIO_NUM_46)
#define GPIO_PA_EN        (GPIO_NUM_10)
#define I2C_SCL           (GPIO_NUM_7)
#define I2C_SDA           (GPIO_NUM_8)
#endif

/*!< Camera PAD */
#ifdef CONFIG_CAMERA_PAD_ESP32_S2_KALUGA_V1_2
#define CAM_XCLK  GPIO_NUM_1
#define CAM_PCLK  GPIO_NUM_33 
#define CAM_VSYNC GPIO_NUM_2
#define CAM_HSYNC GPIO_NUM_3
#define CAM_D0    GPIO_NUM_46  /*!< hardware pins: D2 */  
#define CAM_D1    GPIO_NUM_45  /*!< hardware pins: D3 */
#define CAM_D2    GPIO_NUM_41  /*!< hardware pins: D4 */
#define CAM_D3    GPIO_NUM_42  /*!< hardware pins: D5 */
#define CAM_D4    GPIO_NUM_39  /*!< hardware pins: D6 */
#define CAM_D5    GPIO_NUM_40  /*!< hardware pins: D7 */
#define CAM_D6    GPIO_NUM_21  /*!< hardware pins: D8 */
#define CAM_D7    GPIO_NUM_38  /*!< hardware pins: D9 */
#define CAM_SCL   GPIO_NUM_7
#define CAM_SDA   GPIO_NUM_8
#endif 

/*!< LCD PAD */
#ifdef CONFIG_LCD_PAD_ESP32_S2_KALUGA_V1_2
#define LCD_CLK   GPIO_NUM_15
#define LCD_MOSI  GPIO_NUM_9
#define LCD_DC    GPIO_NUM_13
#define LCD_RST   GPIO_NUM_16
#define LCD_CS    GPIO_NUM_11
#define LCD_BK    GPIO_NUM_6
#endif 

#ifdef __cplusplus
}
#endif