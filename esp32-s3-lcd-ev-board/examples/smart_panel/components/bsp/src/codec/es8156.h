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

#ifndef _ES8156_H
#define _ES8156_H

#include "audio_hal.h"
#include "esp_types.h"
#include "esxxx_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ES8156_VOL_MIN = 0x00,      /*!< -95.5dB */
    ES8156_VOL_MIN_10dB = 0xAB, /*!< -10dB */
    ES8156_VOL_MIN_9dB = 0xAD,  /*!< -9dB */
    ES8156_VOL_MIN_6dB = 0xB3,  /*!< -6dB */
    ES8156_VOL_MIN_3dB = 0xB9,  /*!< -3dB */
    ES8156_VOL_0dB = 0xBF,      /*!< 0dB */
    ES8156_VOL_3dB = 0xC5,      /*!< +3dB */
    ES8156_VOL_10dB = 0xD3,     /*!< +10dB */
    ES8156_VOL_MAX = 0xFF,      /*!< +32dB */
} es8156_volume_t;

/*
 * @brief Initialize ES8156 codec chip
 *
 * @param codec_cfg  configuration of ES8156
 *
 * @return
 *      - ESP_OK
 *      - ESP_FAIL
 */
esp_err_t es8156_codec_init(uint8_t i2c_addr, audio_hal_codec_config_t *codec_cfg);

/**
 * @brief Deinitialize ES8156 codec chip
 *
 * @return
 *     - ESP_OK
 *     - ESP_FAIL
 */
esp_err_t es8156_codec_deinit(void);


esp_err_t es8156_config_fmt(es_i2s_fmt_t fmt);

/**
 * @brief Configure ES8156 DAC mute or not. Basically you can use this function to mute the output or unmute
 *
 * @param enable enable(1) or disable(0)
 *
 * @return
 *     - ESP_FAIL Parameter error
 *     - ESP_OK   Success
 */
esp_err_t es8156_codec_set_voice_mute(bool enable);

/**
 * @brief  Set voice volume
 *
 * @param volume:  voice volume (0~100)
 *
 * @return
 *     - ESP_OK
 *     - ESP_FAIL
 */
esp_err_t es8156_codec_set_voice_volume(uint8_t volume);

/**
 * @brief Get voice volume
 *
 * @param[out] *volume:  voice volume (0~255)
 *
 * @return
 *     - ESP_OK
 *     - ESP_FAIL
 */
esp_err_t es8156_codec_get_voice_volume(uint8_t *volume);

/**
 * @brief 
 * 
 * @param is_right 
 * @return esp_err_t 
 */
esp_err_t es8156_set_channel(uint8_t is_right);

#ifdef __cplusplus
}
#endif
#endif
