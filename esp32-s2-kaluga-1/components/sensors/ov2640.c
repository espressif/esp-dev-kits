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

#include "ov2640.h"
#include "ov2640cfg.h"
#include "sccb.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "sensor.h"

static const char *TAG = "OV2640";

#define SCCB_ID 0x30

#define delay_ms(a) vTaskDelay(a / portTICK_RATE_MS);

esp_err_t OV2640_Init(uint8_t mode, uint8_t fre_double_en)
{
    uint16_t i = 0;
    uint16_t reg;

    SCCB_Write(SCCB_ID ,OV2640_DSP_RA_DLMT, 0x01);	/*!< Operate sensor register */
    SCCB_Write(SCCB_ID ,OV2640_SENSOR_COM7, 0x80);	/*!< Soft reset OV2640 */
    delay_ms(50);
    reg = SCCB_Read(SCCB_ID, OV2640_SENSOR_MIDH);	/*!< Read the manufacturer high 8 bits ID */
    reg <<= 8;
    reg |= SCCB_Read(SCCB_ID, OV2640_SENSOR_MIDL);	/*!< Read the manufacturer low 8 bits ID */

    if (reg != OV2640_MID) {
        ESP_LOGI(TAG, "MID:%d\r\n", reg);
        return ESP_FAIL;
    }

    reg = SCCB_Read(SCCB_ID, OV2640_SENSOR_PIDH);	/*!< Read the manufacturer high 8 bits ID */
    reg <<= 8;
    reg |= SCCB_Read(SCCB_ID, OV2640_SENSOR_PIDL);	/*!< Read the manufacturer low 8 bits ID */

    if (reg != OV2640_PID) {
        ESP_LOGI(TAG, "HID:%d\r\n", reg);
        //return ESP_FAIL;
    }

    if (mode == 0) {
        /*!< Initialize OV2640 with SVGA resolution (800*600) */
        for (i = 0; i < sizeof(ov2640_svga_init_reg_tbl) / 2; i++) {
            SCCB_Write(SCCB_ID ,ov2640_svga_init_reg_tbl[i][0], ov2640_svga_init_reg_tbl[i][1]);
        }
    } else {
        /*!< Initialize OV2640 with UXGA resolution (1600*1200) */
        for (i = 0; i < sizeof(ov2640_uxga_init_reg_tbl) / 2; i++) {
            SCCB_Write(SCCB_ID ,ov2640_uxga_init_reg_tbl[i][0], ov2640_uxga_init_reg_tbl[i][1]);
        }
    }

    if (fre_double_en) {
        SCCB_Write(SCCB_ID ,0xFF, 0x01);
        uint8_t temp = SCCB_Read(SCCB_ID, OV2640_SENSOR_CLKRC);
        SCCB_Write(SCCB_ID ,OV2640_SENSOR_CLKRC, temp | 0x80);
    }

    return ESP_OK;
}

void OV2640_YUV_Mode(void)
{
    uint16_t i = 0;

    /*!< Setting :YUV422 format */
    for (i = 0; i < (sizeof(ov2640_yuv422_reg_tbl) / 2); i++) {
        SCCB_Write(SCCB_ID ,ov2640_yuv422_reg_tbl[i][0], ov2640_yuv422_reg_tbl[i][1]);
    }

}

void OV2640_JPEG_Mode(void)
{
    uint16_t i = 0;
    OV2640_YUV_Mode();
    SCCB_Write(SCCB_ID ,0xFF, 0x00);
    uint8_t temp = SCCB_Read(SCCB_ID, OV2640_DSP_IMAGE_MODE);
    SCCB_Write(SCCB_ID ,OV2640_DSP_IMAGE_MODE, temp | 0x10);

    /*!< Settings: output JPEG data */
    for (i = 0; i < (sizeof(ov2640_jpeg_reg_tbl) / 2); i++) {
        SCCB_Write(SCCB_ID ,ov2640_jpeg_reg_tbl[i][0], ov2640_jpeg_reg_tbl[i][1]);
    }
}

void OV2640_RGB565_Mode(uint8_t byte_swap_en)
{
    uint16_t i = 0;

    /*!< Setting :RGB565 output */
    for (i = 0; i < (sizeof(ov2640_rgb565_reg_tbl) / 2); i++) {
        SCCB_Write(SCCB_ID ,ov2640_rgb565_reg_tbl[i][0], ov2640_rgb565_reg_tbl[i][1]);
    }

    if (byte_swap_en) {
        SCCB_Write(SCCB_ID ,0xFF, 0x00);
        uint8_t temp = SCCB_Read(SCCB_ID, OV2640_DSP_IMAGE_MODE);
        SCCB_Write(SCCB_ID ,OV2640_DSP_IMAGE_MODE, temp | 0x01);
    }
}

const static uint8_t OV2640_AUTOEXPOSURE_LEVEL[5][8] = {
    {
        0xFF, 0x01,
        0x24, 0x20,
        0x25, 0x18,
        0x26, 0x60,
    },
    {
        0xFF, 0x01,
        0x24, 0x34,
        0x25, 0x1c,
        0x26, 0x00,
    },
    {
        0xFF, 0x01,
        0x24, 0x3e,
        0x25, 0x38,
        0x26, 0x81,
    },
    {
        0xFF, 0x01,
        0x24, 0x48,
        0x25, 0x40,
        0x26, 0x81,
    },
    {
        0xFF, 0x01,
        0x24, 0x58,
        0x25, 0x50,
        0x26, 0x92,
    },
};

void OV2640_Auto_Exposure(uint8_t level)
{
    uint8_t i;
    uint8_t *p = (uint8_t *)OV2640_AUTOEXPOSURE_LEVEL[level];

    for (i = 0; i < 4; i++) {
        SCCB_Write(SCCB_ID ,p[i * 2], p[i * 2 + 1]);
    }
}

void OV2640_Light_Mode(uint8_t mode)
{
    uint8_t regccval = 0X5E; /*!< Sunny */
    uint8_t regcdval = 0X41;
    uint8_t regceval = 0X54;

    switch (mode) {
        case 0:/*!< auto */
            SCCB_Write(SCCB_ID ,0XFF, 0X00);
            SCCB_Write(SCCB_ID ,0XC7, 0X00); /*!< AWB ON */
            return;

        case 2:/*!< cloudy */
            regccval = 0X65;
            regcdval = 0X41;
            regceval = 0X4F;
            break;

        case 3:/*!< office */
            regccval = 0X52;
            regcdval = 0X41;
            regceval = 0X66;
            break;

        case 4:/*!< home */
            regccval = 0X42;
            regcdval = 0X3F;
            regceval = 0X71;
            break;
    }

    SCCB_Write(SCCB_ID ,0XFF, 0X00);
    SCCB_Write(SCCB_ID ,0XC7, 0X40);	/*!< AWB OFF */
    SCCB_Write(SCCB_ID ,0XCC, regccval);
    SCCB_Write(SCCB_ID ,0XCD, regcdval);
    SCCB_Write(SCCB_ID ,0XCE, regceval);
}

void OV2640_Color_Saturation(uint8_t sat)
{
    uint8_t reg7dval = ((sat + 2) << 4) | 0X08;
    SCCB_Write(SCCB_ID ,0XFF, 0X00);
    SCCB_Write(SCCB_ID ,0X7C, 0X00);
    SCCB_Write(SCCB_ID ,0X7D, 0X02);
    SCCB_Write(SCCB_ID ,0X7C, 0X03);
    SCCB_Write(SCCB_ID ,0X7D, reg7dval);
    SCCB_Write(SCCB_ID ,0X7D, reg7dval);
}

void OV2640_Brightness(uint8_t bright)
{
    SCCB_Write(SCCB_ID ,0xff, 0x00);
    SCCB_Write(SCCB_ID ,0x7c, 0x00);
    SCCB_Write(SCCB_ID ,0x7d, 0x04);
    SCCB_Write(SCCB_ID ,0x7c, 0x09);
    SCCB_Write(SCCB_ID ,0x7d, bright << 4);
    SCCB_Write(SCCB_ID ,0x7d, 0x00);
}

void OV2640_Contrast(uint8_t contrast)
{
    uint8_t reg7d0val = 0X20; /*!< Default to normal mode */
    uint8_t reg7d1val = 0X20;

    switch (contrast) {
        case 0:/*!< -2 */
            reg7d0val = 0X18;
            reg7d1val = 0X34;
            break;

        case 1:/*!< -1 */
            reg7d0val = 0X1C;
            reg7d1val = 0X2A;
            break;

        case 3:/*!< 1 */
            reg7d0val = 0X24;
            reg7d1val = 0X16;
            break;

        case 4:/*!< 2 */
            reg7d0val = 0X28;
            reg7d1val = 0X0C;
            break;
    }

    SCCB_Write(SCCB_ID ,0xff, 0x00);
    SCCB_Write(SCCB_ID ,0x7c, 0x00);
    SCCB_Write(SCCB_ID ,0x7d, 0x04);
    SCCB_Write(SCCB_ID ,0x7c, 0x07);
    SCCB_Write(SCCB_ID ,0x7d, 0x20);
    SCCB_Write(SCCB_ID ,0x7d, reg7d0val);
    SCCB_Write(SCCB_ID ,0x7d, reg7d1val);
    SCCB_Write(SCCB_ID ,0x7d, 0x06);
}

void OV2640_Special_Effects(uint8_t eft)
{
    uint8_t reg7d0val = 0X00; /*!< Default to normal mode */
    uint8_t reg7d1val = 0X80;
    uint8_t reg7d2val = 0X80;

    switch (eft) {
        case 1:
            reg7d0val = 0X40;
            break;

        case 2:
            reg7d0val = 0X18;
            break;

        case 3:
            reg7d0val = 0X18;
            reg7d1val = 0X40;
            reg7d2val = 0XC0;
            break;

        case 4:
            reg7d0val = 0X18;
            reg7d1val = 0X40;
            reg7d2val = 0X40;
            break;

        case 5:
            reg7d0val = 0X18;
            reg7d1val = 0XA0;
            reg7d2val = 0X40;
            break;

        case 6:
            reg7d0val = 0X18;
            reg7d1val = 0X40;
            reg7d2val = 0XA6;
            break;
    }

    SCCB_Write(SCCB_ID ,0xff, 0x00);
    SCCB_Write(SCCB_ID ,0x7c, 0x00);
    SCCB_Write(SCCB_ID ,0x7d, reg7d0val);
    SCCB_Write(SCCB_ID ,0x7c, 0x05);
    SCCB_Write(SCCB_ID ,0x7d, reg7d1val);
    SCCB_Write(SCCB_ID ,0x7d, reg7d2val);
}

void OV2640_Color_Bar(uint8_t sw)
{
    uint8_t reg;
    SCCB_Write(SCCB_ID ,0XFF, 0X01);
    reg = SCCB_Read(SCCB_ID, 0X12);
    reg &= ~(1 << 1);

    if (sw) {
        reg |= 1 << 1;
    }

    SCCB_Write(SCCB_ID ,0X12, reg);
}

void OV2640_Window_Set(uint16_t sx, uint16_t sy, uint16_t width, uint16_t height)
{
    uint16_t endx;
    uint16_t endy;
    uint8_t temp;
    endx = sx + width / 2;	/*!< V*2 */
    endy = sy + height / 2;

    SCCB_Write(SCCB_ID ,0XFF, 0X01);
    temp = SCCB_Read(SCCB_ID, 0X03);				/*!< Read the value before Vref */
    temp &= 0XF0;
    temp |= ((endy & 0X03) << 2) | (sy & 0X03);
    SCCB_Write(SCCB_ID ,0X03, temp);				/*!< Set the lowest two bits of start and end of Vref */
    SCCB_Write(SCCB_ID ,0X19, sy >> 2);			/*!< Set the start of Vref to 8 high bits  */
    SCCB_Write(SCCB_ID ,0X1A, endy >> 2);			/*!< Set the end of Vref to 8 high bits  */

    temp = SCCB_Read(SCCB_ID, 0X32);				/*!< Read the value before the Href */
    temp &= 0XC0;
    temp |= ((endx & 0X07) << 3) | (sx & 0X07);
    SCCB_Write(SCCB_ID ,0X32, temp);				/*!< Set the lowest 3 bits of start and end of Href */
    SCCB_Write(SCCB_ID ,0X17, sx >> 3);			/*!< Set the start of Href to 8 high bits */
    SCCB_Write(SCCB_ID ,0X18, endx >> 3);			/*!<et the end of Href to 8 high bits */
}

esp_err_t OV2640_OutSize_Set(uint16_t width, uint16_t height)
{
    uint16_t outh;
    uint16_t outw;
    uint8_t temp;

    if (width % 4) {
        return ESP_FAIL;
    }

    if (height % 4) {
        return ESP_FAIL;
    }

    outw = width / 4;
    outh = height / 4;
    SCCB_Write(SCCB_ID ,0XFF, 0X00);
    SCCB_Write(SCCB_ID ,0XE0, 0X04);
    SCCB_Write(SCCB_ID ,0X5A, outw & 0XFF);		/*!< Set the lower 8 bits of OUTW */
    SCCB_Write(SCCB_ID ,0X5B, outh & 0XFF);		/*!< Set the lower 8 bits of OUTH */
    temp = (outw >> 8) & 0X03;
    temp |= (outh >> 6) & 0X04;
    SCCB_Write(SCCB_ID ,0X5C, temp);				/*!< Set the high bits of OUTH/OUTW */
    SCCB_Write(SCCB_ID ,0XE0, 0X00);
    return ESP_OK;
}

esp_err_t OV2640_ImageWin_Set(uint16_t offx, uint16_t offy, uint16_t width, uint16_t height)
{
    uint16_t hsize;
    uint16_t vsize;
    uint8_t temp;

    if (width % 4) {
        return ESP_FAIL;
    }

    if (height % 4) {
        return ESP_FAIL;
    }

    hsize = width / 4;
    vsize = height / 4;
    SCCB_Write(SCCB_ID ,0XFF, 0X00);
    SCCB_Write(SCCB_ID ,0XE0, 0X04);
    SCCB_Write(SCCB_ID ,0X51, hsize & 0XFF);		/*!< Set the lower 8 bits of H_SIZE */
    SCCB_Write(SCCB_ID ,0X52, vsize & 0XFF);		/*!< Set the lower 8 bits of v_SIZE */
    SCCB_Write(SCCB_ID ,0X53, offx & 0XFF);		/*!< Set the lower 8 bits of offx */
    SCCB_Write(SCCB_ID ,0X54, offy & 0XFF);		/*!< Set the lower 8 bits of offy */
    temp = (vsize >> 1) & 0X80;
    temp |= (offy >> 4) & 0X70;
    temp |= (hsize >> 5) & 0X08;
    temp |= (offx >> 8) & 0X07;
    SCCB_Write(SCCB_ID ,0X55, temp);				/*!< Set H_SIZE/V_SIZE/OFFX, the high value of OFFY */
    SCCB_Write(SCCB_ID ,0X57, (hsize >> 2) & 0X80);	/*!< Set H_SIZE/V_SIZE/OFFX, the high value of OFFY */
    SCCB_Write(SCCB_ID ,0XE0, 0X00);
    return ESP_OK;
}

esp_err_t OV2640_ImageSize_Set(uint16_t width, uint16_t height)
{
    uint8_t temp;
    SCCB_Write(SCCB_ID ,0XFF, 0X00);
    SCCB_Write(SCCB_ID ,0XE0, 0X04);
    SCCB_Write(SCCB_ID ,0XC0, (width) >> 3 & 0XFF);		
    SCCB_Write(SCCB_ID ,0XC1, (height) >> 3 & 0XFF);		
    temp = (width & 0X07) << 3;
    temp |= height & 0X07;
    temp |= (width >> 4) & 0X80;
    SCCB_Write(SCCB_ID ,0X8C, temp);
    SCCB_Write(SCCB_ID ,0XE0, 0X00);
    return ESP_OK;
}