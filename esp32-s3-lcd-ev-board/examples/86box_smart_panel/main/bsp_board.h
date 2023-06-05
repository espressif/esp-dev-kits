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

#include "file_iterator.h"

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

/**
 * @brief Set RGB for a specific pixel
 *
 * @param index: index of pixel to set
 * @param red: red part of color
 * @param green: green part of color
 * @param blue: blue part of color
 *
 * @return
 *    - ESP_OK: Success
 *    - Others: Fail
 */
esp_err_t bsp_led_set_rgb(uint8_t index, uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Get the file player instance
 *
 * @return pointer of file player
 */
file_iterator_instance_t *get_file_iterator_instance(void);

/**
 * @brief Init audio player module
 *
 * @return
 *    - ESP_OK: Success
 *    - Others: Fail
 */
esp_err_t bsp_audio_player_init(void);

/**************************************************************************************************
 *
 * SPIFFS
 *
 * After mounting the SPIFFS, it can be accessed with stdio functions ie.:
 * \code{.c}
 * FILE* f = fopen(BSP_MOUNT_POINT"/hello.txt", "w");
 * fprintf(f, "Hello World!\n");
 * fclose(f);
 * \endcode
 **************************************************************************************************/
#define BSP_MOUNT_POINT      CONFIG_BSP_SPIFFS_MOUNT_POINT

/**
 * @brief Mount SPIFFS to virtual file system
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_STATE if esp_vfs_spiffs_register was already called
 *      - ESP_ERR_NO_MEM if memory can not be allocated
 *      - ESP_FAIL if partition can not be mounted
 *      - other error codes
 */
esp_err_t bsp_spiffs_mount(void);

/**
 * @brief Unmount SPIFFS from virtual file system
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_NOT_FOUND if the partition table does not contain SPIFFS partition with given label
 *      - ESP_ERR_INVALID_STATE if esp_vfs_spiffs_unregister was already called
 *      - ESP_ERR_NO_MEM if memory can not be allocated
 *      - ESP_FAIL if partition can not be mounted
 *      - other error codes
 */
esp_err_t bsp_spiffs_unmount(void);

#ifdef __cplusplus
}
#endif
