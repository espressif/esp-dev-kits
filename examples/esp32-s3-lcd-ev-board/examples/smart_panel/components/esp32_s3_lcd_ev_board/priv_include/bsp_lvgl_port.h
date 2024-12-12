/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdint.h>

#include "esp_err.h"
#include "esp_lcd_types.h"
#include "esp_lcd_touch.h"
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize LVGL port
 *
 * @param[in] lcd: LCD panel handle
 * @param[in] tp: Touch handle
 * @param[out] disp: LVGL display device
 * @param[out] indev: LVGL input device
 *
 * @return
 *      - ESP_OK: Success
 *      - ESP_ERR_INVALID_ARG: Invalid argument
 *      - Others: Fail
 */
esp_err_t bsp_lvgl_port_init(esp_lcd_panel_handle_t lcd, esp_lcd_touch_handle_t tp, lv_disp_t **disp, lv_indev_t **indev);

/**
 * @brief Take LVGL mutex
 *
 * @param[in] timeout_ms: Timeout in [ms]. 0 will block indefinitely.
 *
 * @return
 *      - true:  Mutex was taken
 *      - false: Mutex was NOT taken
 */
bool bsp_lvgl_port_lock(uint32_t timeout_ms);

/**
 * @brief Give LVGL mutex
 *
 */
void bsp_lvgl_port_unlock(void);

#ifdef __cplusplus
}
#endif
