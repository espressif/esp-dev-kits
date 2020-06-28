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

#include <stdio.h>
#include <stdint.h>
#include "sccb.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define OV2640_MID 0X7FA2
// #define OV2640_PID 0X2642

/* When the DSP address (0XFF=0X00) is selected, the DSP register address map table for OV2640 */
#define OV2640_DSP_R_BYPASS 0x05
#define OV2640_DSP_Qs 0x44
#define OV2640_DSP_CTRL 0x50
#define OV2640_DSP_HSIZE1 0x51
#define OV2640_DSP_VSIZE1 0x52
#define OV2640_DSP_XOFFL 0x53
#define OV2640_DSP_YOFFL 0x54
#define OV2640_DSP_VHYX 0x55
#define OV2640_DSP_DPRP 0x56
#define OV2640_DSP_TEST 0x57
#define OV2640_DSP_ZMOW 0x5A
#define OV2640_DSP_ZMOH 0x5B
#define OV2640_DSP_ZMHH 0x5C
#define OV2640_DSP_BPADDR 0x7C
#define OV2640_DSP_BPDATA 0x7D
#define OV2640_DSP_CTRL2 0x86
#define OV2640_DSP_CTRL3 0x87
#define OV2640_DSP_SIZEL 0x8C
#define OV2640_DSP_HSIZE2 0xC0
#define OV2640_DSP_VSIZE2 0xC1
#define OV2640_DSP_CTRL0 0xC2
#define OV2640_DSP_CTRL1 0xC3
#define OV2640_DSP_R_DVP_SP 0xD3
#define OV2640_DSP_IMAGE_MODE 0xDA
#define OV2640_DSP_RESET 0xE0
#define OV2640_DSP_MS_SP 0xF0
#define OV2640_DSP_SS_ID 0x7F
#define OV2640_DSP_SS_CTRL 0xF8
#define OV2640_DSP_MC_BIST 0xF9
#define OV2640_DSP_MC_AL 0xFA
#define OV2640_DSP_MC_AH 0xFB
#define OV2640_DSP_MC_D 0xFC
#define OV2640_DSP_P_STATUS 0xFE
#define OV2640_DSP_RA_DLMT 0xFF

/* When the sensor address (0XFF=0X01) is selected, the DSP register address map table for OV2640 */
#define OV2640_SENSOR_GAIN 0x00
#define OV2640_SENSOR_COM1 0x03
#define OV2640_SENSOR_REG04 0x04
#define OV2640_SENSOR_REG08 0x08
#define OV2640_SENSOR_COM2 0x09
#define OV2640_SENSOR_PIDH 0x0A
#define OV2640_SENSOR_PIDL 0x0B
#define OV2640_SENSOR_COM3 0x0C
#define OV2640_SENSOR_COM4 0x0D
#define OV2640_SENSOR_AEC 0x10
#define OV2640_SENSOR_CLKRC 0x11
#define OV2640_SENSOR_COM7 0x12
#define OV2640_SENSOR_COM8 0x13
#define OV2640_SENSOR_COM9 0x14
#define OV2640_SENSOR_COM10 0x15
#define OV2640_SENSOR_HREFST 0x17
#define OV2640_SENSOR_HREFEND 0x18
#define OV2640_SENSOR_VSTART 0x19
#define OV2640_SENSOR_VEND 0x1A
#define OV2640_SENSOR_MIDH 0x1C
#define OV2640_SENSOR_MIDL 0x1D
#define OV2640_SENSOR_AEW 0x24
#define OV2640_SENSOR_AEB 0x25
#define OV2640_SENSOR_W 0x26
#define OV2640_SENSOR_REG2A 0x2A
#define OV2640_SENSOR_FRARL 0x2B
#define OV2640_SENSOR_ADDVSL 0x2D
#define OV2640_SENSOR_ADDVHS 0x2E
#define OV2640_SENSOR_YAVG 0x2F
#define OV2640_SENSOR_REG32 0x32
#define OV2640_SENSOR_ARCOM2 0x34
#define OV2640_SENSOR_REG45 0x45
#define OV2640_SENSOR_FLL 0x46
#define OV2640_SENSOR_FLH 0x47
#define OV2640_SENSOR_COM19 0x48
#define OV2640_SENSOR_ZOOMS 0x49
#define OV2640_SENSOR_COM22 0x4B
#define OV2640_SENSOR_COM25 0x4E
#define OV2640_SENSOR_BD50 0x4F
#define OV2640_SENSOR_BD60 0x50
#define OV2640_SENSOR_REG5D 0x5D
#define OV2640_SENSOR_REG5E 0x5E
#define OV2640_SENSOR_REG5F 0x5F
#define OV2640_SENSOR_REG60 0x60
#define OV2640_SENSOR_HISTO_LOW 0x61
#define OV2640_SENSOR_HISTO_HIGH 0x62

/**
 * @brief Initialize OV2640, XCLK input 12MHz clock.
 *
 * @param mode 0: Initialize OV2640 with SVGA resolution (800*600)
 *             1: Initialize OV2640 with UXGA resolution (1600*1200) 
 * @param fre_double_en 0: disable double frequency
 *                      1: enable double frequency
 * @return - ESP_FAIL Initialization failure
 *         - ESP_OK on Successful initialization
 */
esp_err_t OV2640_Init(uint8_t mode, uint8_t fre_double_en);

/**
 * @brief Set OV2640 to JPEG mode.
 */
void OV2640_JPEG_Mode(void);

/**
 * @brief Set OV2640 to RGB565 mode.
 * 
 * @param byte_swap_en 0:disable swap byte.(default)
 *                     1:enable swap byte.
 */
void OV2640_RGB565_Mode(uint8_t byte_swap_en);

/**
 * @brief Automatic exposure level Settings for OV2640.
 * 
 * @param level set level (0 ~ 4)
 */
void OV2640_Auto_Exposure(uint8_t level);

/**
 * @brief White balance setting
 * 
 * @param mode 0: auto
 *             1: sunny
 *             2: cloudy
 *             3: office
 *             4: home
 */
void OV2640_Light_Mode(uint8_t mode);

/**
 * @brief Chroma set
 * 
 * @param sat  0: -2
 *             1: -1
 *             2:  0
 *             3: +1
 *             4: +2
 */
void OV2640_Color_Saturation(uint8_t Chroma);

/**
 * @brief The brightness Settings
 * 
 * @param bright 0: (0X00)-2
 *               1: (0X10)-1
 *               2: (0X20) 0
 *               3: (0X30)+1
 *               4: (0X40)+2
 */
void OV2640_Brightness(uint8_t bright);

/**
 * @brief contrast Settings
 * 
 * @param contrast  0: -2
 *                  1: -1
 *                  2:  0
 *                  3: +1
 *                  4: +2
 */
void OV2640_Contrast(uint8_t contrast);

/**
 * @brief Special effects Settings
 * 
 * @param contrast  0: Normal mode
 *                  1: negative
 *                  2: Black and white
 *                  3: red
 *                  4: green
 *                  5: blue
 *                  6: Restoring ancient ways
 */
void OV2640_Special_Effects(uint8_t eft);

/**
 * @brief Striped test
 * 
 * @param sw  0: disable
 *            1: enable (note: the color bar of OV2640 is superimposed on the image)
 */
void OV2640_Color_Bar(uint8_t sw);

/**
 * @brief Set the sensor output window
 * 
 * @param sx x of the starting address 
 * @param sy y of the starting address 
 * @param width Width (horizontal) 
 * @param height height (vertical) 
 */
void OV2640_Window_Set(uint16_t sx, uint16_t sy, uint16_t width, uint16_t height);

/**
 * @brief Sets the image output size.
 *        The size (resolution) of the OV2640 output image is completely determined by this function.
 * @param width Width (horizontal)
 * @param height Height (vertical)
 *               Width and height must be multiples of 4
 * @return -ESP_FAIL setting fail
 *         -ESP_OK   setting success
 */
esp_err_t OV2640_OutSize_Set(uint16_t width, uint16_t height);

/**
 * @brief Set the image window size.
 *        OV2640_ImageSize_Set determines the sensor output resolution from the size.
 *        The function opens a window over this scope for the output of OV2640_OutSize_Set.
 *        note: the width and height of this function must be greater than or equal to the width and 
 *        height of the OV2640_OutSize_Set function.
 *        OV2640_OutSize_Set set the width and height, according to the function set the width and 
 *        height, DSP automatically calculate the scaling, output to the external device.
 * 
 * @param offx x of the starting address 
 * @param offy y of the starting address 
 * @param width Width (horizontal)
 * @param height Height (vertical)
 *               Width and height must be multiples of 4
 * 
 * @return -ESP_FAIL setting fail
 *         -ESP_OK   setting success
 */
esp_err_t OV2640_ImageWin_Set(uint16_t offx, uint16_t offy, uint16_t width, uint16_t height);

/**
 * @brief this function sets the size of the image, which is the output resolution of the selected format.
 *        UXGA: 1600 * 1200, 
 *        SVGA: 800 * 600, 
 *        CIF: 352 * 288
 * 
 * @param width Width (horizontal)
 * @param height Height (vertical)
 * 
 * @return -ESP_FAIL setting fail
 *         -ESP_OK   setting success
 */
esp_err_t OV2640_ImageSize_Set(uint16_t width, uint16_t height);

#ifdef __cplusplus
}
#endif
