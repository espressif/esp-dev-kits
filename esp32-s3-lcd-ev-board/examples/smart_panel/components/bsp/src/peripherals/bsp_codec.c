/*
 * SPDX-FileCopyrightText: 2015-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#include <stdint.h>
#include "esp_log.h"
#include "bsp_board.h"
#include "esp_err.h"
#include "esp_check.h"
#include "virtual_mic.h"

static const char *TAG = "bsp codec";

esp_err_t bsp_codec_init(audio_hal_iface_samples_t sample_rate)
{
    const board_res_desc_t *brd = bsp_board_get_description();
    ESP_RETURN_ON_FALSE(NULL != brd && NULL != brd->codec_ops && NULL != brd->codec_ops->codec_init, ESP_ERR_INVALID_STATE, TAG, "board codec ops is invalid");
    return brd->codec_ops->codec_init(sample_rate);
}

esp_err_t bsp_codec_set_clk(uint32_t rate, uint32_t bits_cfg, uint32_t ch)
{
    const board_res_desc_t *brd = bsp_board_get_description();
    ESP_RETURN_ON_FALSE(NULL != brd && NULL != brd->codec_ops && NULL != brd->codec_ops->codec_set_clk, ESP_ERR_INVALID_STATE, TAG, "board codec ops is invalid");
    return brd->codec_ops->codec_set_clk(rate, bits_cfg, ch);
}

esp_err_t bsp_codec_write(void *dest, size_t size, size_t *bytes_read, TickType_t ticks_to_wait)
{
    const board_res_desc_t *brd = bsp_board_get_description();
    ESP_RETURN_ON_FALSE(NULL != brd && NULL != brd->codec_ops && NULL != brd->codec_ops->codec_write, ESP_ERR_INVALID_STATE, TAG, "board codec ops is invalid");
    return brd->codec_ops->codec_write(dest, size, bytes_read, ticks_to_wait);
}

esp_err_t bsp_codec_read(void *dest, size_t size, size_t *bytes_read, TickType_t ticks_to_wait)
{
    const board_res_desc_t *brd = bsp_board_get_description();
    ESP_RETURN_ON_FALSE(NULL != brd && NULL != brd->codec_ops && NULL != brd->codec_ops->codec_read, ESP_ERR_INVALID_STATE, TAG, "board codec ops is invalid");
    return brd->codec_ops->codec_read(dest, size, bytes_read, ticks_to_wait);
}

esp_err_t bsp_codec_zero_dma_buffer(void)
{
    const board_res_desc_t *brd = bsp_board_get_description();
    ESP_RETURN_ON_FALSE(NULL != brd && NULL != brd->codec_ops && NULL != brd->codec_ops->codec_zero_dma_buffer, ESP_ERR_INVALID_STATE, TAG, "board codec ops is invalid");
    return brd->codec_ops->codec_zero_dma_buffer();
}

esp_err_t bsp_codec_set_voice_volume(uint8_t volume)
{
    const board_res_desc_t *brd = bsp_board_get_description();
    ESP_RETURN_ON_FALSE(NULL != brd && NULL != brd->codec_ops && NULL != brd->codec_ops->codec_set_dac_gain, ESP_ERR_INVALID_STATE, TAG, "board codec ops is invalid");
    return brd->codec_ops->codec_set_dac_gain(volume);
}

esp_err_t bsp_codec_set_fmt(audio_hal_iface_format_t fmt)
{
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t bsp_codec_set_voice_gain(uint8_t channel_mask, uint8_t volume)
{
    return ESP_ERR_NOT_SUPPORTED;
}
