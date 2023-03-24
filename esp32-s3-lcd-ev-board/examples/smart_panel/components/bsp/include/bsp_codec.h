/*
 * SPDX-FileCopyrightText: 2015-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "../codec/audio_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize Codec on dev board
 * 
 * @param sample_rate 
 * @return 
 *    - ESP_OK: Success
 *    - ESP_ERR_NOT_FOUND: Codec not detected on I2C bus
 *    - ESP_ERR_NOT_SUPPORTED: Unsupported Codec or t
 *    - Others: Fail
 */
esp_err_t bsp_codec_init(audio_hal_iface_samples_t sample_rate);

/**
 * @brief Set output volume of Codec
 * 
 * @param volume volume to set
 * @return 
 *    - ESP_OK: Success
 *    - ESP_ERR_NOT_FOUND: Codec not detected on I2C bus
 */
esp_err_t bsp_codec_set_voice_volume(uint8_t volume);

/**
 * @brief Set clock & bit width
 *
 * @param rate sample rate (ex: 8000, 44100...)
 * @param bits_cfg bits configuration
 * @param ch channel
 *
 * @return
 *     - ESP_OK              Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 */
esp_err_t bsp_codec_set_clk(uint32_t rate, uint32_t bits_cfg, uint32_t ch);

/**
 * @brief Write audio data
 *
 * @param src                 Source address to write from
 * @param size                Size of data in bytes
 * @param[out] bytes_written  Number of bytes written, if timeout, the result will be less than the size passed in.
 * @param ticks_to_wait       TX buffer wait timeout in RTOS ticks. If this
 * many ticks pass without space becoming available in the DMA
 * transmit buffer, then the function will return. Pass portMAX_DELAY for no timeout.
 *
 * @return
 *     - ESP_OK               Success
 *     - ESP_ERR_INVALID_ARG  Parameter error
 */
esp_err_t bsp_codec_write(void *dest, size_t size, size_t *bytes_read, TickType_t ticks_to_wait);

/**
 * @brief Read audio data
 *
 * @param dest            Destination address to read into
 * @param size            Size of data in bytes
 * @param[out] bytes_read Number of bytes read, if timeout, bytes read will be less than the size passed in.
 * @param ticks_to_wait   RX buffer wait timeout in RTOS ticks. If this many ticks pass without bytes becoming available in the DMA receive buffer, then the function will return (note that if data is read from the DMA buffer in pieces, the overall operation may still take longer than this timeout.) Pass portMAX_DELAY for no timeout.
 *
 * @return
 *     - ESP_OK               Success
 *     - ESP_ERR_INVALID_ARG  Parameter error
 */
esp_err_t bsp_codec_read(void *dest, size_t size, size_t *bytes_read, TickType_t ticks_to_wait);

/**
 * @brief Zero the contents of the TX DMA buffer.
 * 
 */
esp_err_t bsp_codec_zero_dma_buffer(void);

/**
 * @brief Set input microphone gain of Codec
 * 
 * @param channel_mask mask of channel
 * @param volume volume to set
 * @return 
 *    - ESP_OK: Success
 *    - ESP_ERR_NOT_FOUND: Codec not detected on I2C bus
 */
esp_err_t bsp_codec_set_voice_gain(uint8_t channel_mask, uint8_t volume);

/**
 * @brief Configure I2S format
 *
 * @param fmt: I2S format
 * @return
 *     - ESP_OK: Success
 *     - ESP_FAIL: Fail
 */
esp_err_t bsp_codec_set_fmt(audio_hal_iface_format_t fmt);

#ifdef __cplusplus
}
#endif
