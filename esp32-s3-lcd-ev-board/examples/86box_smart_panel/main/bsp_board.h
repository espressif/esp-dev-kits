/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#pragma once

#include <stdbool.h>
#include "esp_err.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "driver/i2s_std.h"
#include "esp_lcd_types.h"
#include "bsp/esp-bsp.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef esp_err_t (*bsp_codec_reconfig_fn)();

typedef esp_err_t (*bsp_codec_mute_fn)(bool enable);

typedef esp_err_t (*bsp_codec_volume_fn)(int volume, int *volume_set);

typedef esp_err_t (*bsp_i2s_reconfig_clk_fn)(uint32_t rate, uint32_t bits_cfg, i2s_slot_mode_t ch);

typedef esp_err_t (*bsp_i2s_read_fn)(void *audio_buffer, size_t len, size_t *bytes_read, uint32_t timeout_ms);

typedef esp_err_t (*bsp_i2s_write_fn)(void *audio_buffer, size_t len, size_t *bytes_written, uint32_t timeout_ms);

typedef struct {
    bsp_codec_mute_fn mute_set_fn;
    bsp_codec_volume_fn volume_set_fn;
    bsp_codec_reconfig_fn codec_reconfig_fn;

    bsp_i2s_read_fn i2s_read_fn;
    bsp_i2s_write_fn i2s_write_fn;
    bsp_i2s_reconfig_clk_fn i2s_reconfig_clk_fn;
} bsp_codec_config_t;

/**
 * @brief Special config for dev board
 *
 * @return
 *    - ESP_OK: Success
 *    - Others: Fail
 */
esp_err_t bsp_board_init(void);

/**
 * @brief Get the codec operation function
 *
 * @return pointer of bsp_codec_config
 */
bsp_codec_config_t *bsp_board_get_codec_handle(void);

esp_err_t bsp_led_set_rgb(uint8_t index, uint8_t r, uint8_t g, uint8_t b);

#ifdef __cplusplus
}
#endif
