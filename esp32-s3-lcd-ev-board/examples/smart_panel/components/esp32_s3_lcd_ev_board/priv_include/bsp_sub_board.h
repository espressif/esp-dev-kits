/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#pragma once

#include "esp_err.h"
#include "esp_lcd_types.h"
#include "esp_lcd_touch.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief LCD transmit done callback function type
 *
 */
typedef bool (*bsp_lcd_trans_done_cb_t)(esp_lcd_panel_handle_t handle);

/**
 * @brief Init LCD panel
 *
 * @param[in] arg: User data/function, pass NULL if don't need
 *
 * @return Pointer to LCD panel handle, or NULL if error occurs
 */
esp_lcd_panel_handle_t bsp_lcd_init(void *arg);

/**
 * @brief Register a callback function which will be called when LCD finish refreshing
 *
 * @param[in] callback: The function to be registered. It should return true if need to yield, otherwise return false
 *
 * @return
 *      - ESP_OK:               Succsee
 *      - ESP_ERR_INVALID_ARG:  Callback function should be placed in IRAM, use `IRAM_ATTR` to define it
 */
esp_err_t bsp_lcd_register_trans_done_callback(bsp_lcd_trans_done_cb_t callback);

/**
 * @brief Init touch panel
 *
 * @note This function should be called after I2C bus is initialized
 *
 * @return Pointer to touch handle, or NULL if error occurs
 *
 */
esp_lcd_touch_handle_t bsp_touch_panel_init(void);

#ifdef __cplusplus
}
#endif
