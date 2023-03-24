/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2021 <ESPRESSIF SYSTEMS (SHANGHAI) CO., LTD>
 *
 * Permission is hereby granted for use on all ESPRESSIF SYSTEMS products, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "bsp_i2c.h"
#include "string.h"
#include "esp_log.h"
#include "i2c_bus.h"
#include "es8156.h"

static const char *TAG = "es8156";


/* ES8156 register space */
/*
* RESET Control
*/
#define ES8156_RESET_REG00             0x00
/*
* Clock Managerment
*/
#define ES8156_MAINCLOCK_CTL_REG01     0x01
#define ES8156_SCLK_MODE_REG02         0x02
#define ES8156_LRCLK_DIV_H_REG03       0x03
#define ES8156_LRCLK_DIV_L_REG04       0x04
#define ES8156_SCLK_DIV_REG05          0x05
#define ES8156_NFS_CONFIG_REG06        0x06
#define ES8156_MISC_CONTROL1_REG07     0x07
#define ES8156_CLOCK_ON_OFF_REG08      0x08
#define ES8156_MISC_CONTROL2_REG09     0x09
#define ES8156_TIME_CONTROL1_REG0A     0x0a
#define ES8156_TIME_CONTROL2_REG0B     0x0b
/*
* System Control
*/
#define ES8156_CHIP_STATUS_REG0C       0x0c
#define ES8156_P2S_CONTROL_REG0D       0x0d
#define ES8156_DAC_OSR_COUNTER_REG10   0x10
/*
* SDP Control
*/
#define ES8156_DAC_SDP_REG11           0x11
#define ES8156_AUTOMUTE_SET_REG12      0x12
#define ES8156_DAC_MUTE_REG13          0x13
#define ES8156_VOLUME_CONTROL_REG14    0x14

/*
* ALC Control
*/
#define ES8156_ALC_CONFIG1_REG15       0x15
#define ES8156_ALC_CONFIG2_REG16       0x16
#define ES8156_ALC_CONFIG3_REG17       0x17
#define ES8156_MISC_CONTROL3_REG18     0x18
#define ES8156_EQ_CONTROL1_REG19       0x19
#define ES8156_EQ_CONTROL2_REG1A       0x1a
/*
* Analog System Control
*/
#define ES8156_ANALOG_SYS1_REG20       0x20
#define ES8156_ANALOG_SYS2_REG21       0x21
#define ES8156_ANALOG_SYS3_REG22       0x22
#define ES8156_ANALOG_SYS4_REG23       0x23
#define ES8156_ANALOG_LP_REG24         0x24
#define ES8156_ANALOG_SYS5_REG25       0x25
/*
* Chip Information
*/
#define ES8156_I2C_PAGESEL_REGFC       0xFC
#define ES8156_CHIPID1_REGFD           0xFD
#define ES8156_CHIPID0_REGFE           0xFE
#define ES8156_CHIP_VERSION_REGFF      0xFF


#define ES8156_ADDR (0x08)

static i2c_bus_handle_t i2c_handle;

static esp_err_t es8156_write_reg(uint8_t reg_addr, uint8_t data)
{
    return i2c_bus_write_byte(i2c_handle, reg_addr, data);
}

static uint8_t es8156_read_reg(uint8_t reg_addr)
{
    uint8_t data;
    i2c_bus_read_byte(i2c_handle, reg_addr, &data);
    return data;
}

esp_err_t es8156_codec_init(uint8_t i2c_addr, audio_hal_codec_config_t *cfg)
{
    esp_err_t ret_val = ESP_OK;
    ret_val |= bsp_i2c_add_device(&i2c_handle, i2c_addr);

    uint8_t misc_ctrl_reg_val = 0x00;
    uint8_t dac_iface_reg_val = 0x00;

    if (AUDIO_HAL_MODE_MASTER == cfg->i2s_iface.mode) {
        misc_ctrl_reg_val |= 0b00100000;
    } else {
        misc_ctrl_reg_val |= 0b00000000;
    }

    switch (cfg->i2s_iface.bits) {
    case AUDIO_HAL_BIT_LENGTH_16BITS:
        dac_iface_reg_val |= 0b00110000;
        break;
    case AUDIO_HAL_BIT_LENGTH_24BITS:
        dac_iface_reg_val |= 0b00000000;
        break;
    case AUDIO_HAL_BIT_LENGTH_32BITS:
        dac_iface_reg_val |= 0b01000000;
        break;
    default:    /* Use 32 bit as default */
        dac_iface_reg_val |= 0b01000000;
        break;
    }

    ret_val |= es8156_write_reg(0x09, misc_ctrl_reg_val);  // MCLK from pad, Slave mode, power down DLL, enable pin pull up
    ret_val |= es8156_write_reg(0x11, dac_iface_reg_val);  // DAC Interface Config
    ret_val |= es8156_write_reg(0x14, ES8156_VOL_MIN_3dB); // Set default volume to 0dB

    return ret_val;
}

esp_err_t es8156_codec_deinit(void)
{
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t es8156_codec_set_voice_mute(bool enable)
{
    int regv = es8156_read_reg(ES8156_DAC_MUTE_REG13);
    if (enable) {
        regv = regv | BIT(1) | BIT(2);
    } else {
        regv = regv & (~(BIT(1) | BIT(2))) ;
    }
    es8156_write_reg(ES8156_DAC_MUTE_REG13, regv);
    return ESP_OK;
}

esp_err_t es8156_codec_set_voice_volume(uint8_t volume)
{
    if (volume > 100) {
        volume = 100;
    }
    uint8_t d = 0xBF - 60 + 6 * volume / 10;
    if (0 == volume) {
        d = 0;
    }
    return es8156_write_reg(ES8156_VOLUME_CONTROL_REG14, d);
}

esp_err_t es8156_codec_get_voice_volume(uint8_t *volume)
{
    *volume = es8156_read_reg(ES8156_VOLUME_CONTROL_REG14);

    return ESP_OK;
}

esp_err_t es8156_config_fmt(es_i2s_fmt_t fmt)
{
    ESP_LOGW(TAG, "Not support config format for es8156 now");
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t es8156_set_channel(uint8_t is_right)
{
    uint8_t reg = es8156_read_reg(ES8156_DAC_SDP_REG11);
    if (is_right) {
        reg |= 0b00000100;
    } else {
        reg &= 0b11111011;
    }
    return es8156_write_reg(ES8156_DAC_SDP_REG11, reg);
}
