/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ESP LCD touch: JD9366
 */

#pragma once

#include "esp_lcd_touch.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create a new JD9366 touch driver
 *
 * @note The I2C communication should be initialized before use this function.
 *
 * @param io LCD/Touch panel IO handle
 * @param config Touch configuration
 * @param out_touch Touch instance handle
 * @return
 *      - ESP_OK                    on success
 *      - ESP_ERR_NO_MEM            if there is no memory for allocating main structure
 */
esp_err_t esp_lcd_touch_new_i2c_jd9366(const esp_lcd_panel_io_handle_t io, const esp_lcd_touch_config_t *config, esp_lcd_touch_handle_t *out_touch);

/**
 * @brief Debug counters for touch-issue diagnosis.
 *        Exported from the driver, dumped periodically by the diagnostic
 *        task in ksdiy_lvgl_port.c:
 *          reads  grows -> indev polling alive
 *          errors grows -> I2C bus / address problem
 *          hits   grows -> touch detected, coordinates in last_x/last_y
 */
extern volatile uint32_t jd9366_dbg_reads;
extern volatile uint32_t jd9366_dbg_errors;
extern volatile uint32_t jd9366_dbg_hits;
extern volatile uint32_t jd9366_dbg_last_x;
extern volatile uint32_t jd9366_dbg_last_y;

/**
 * @brief I2C address of the JD9366 controller
 *
 */
#define ESP_LCD_TOUCH_IO_I2C_JD9366_ADDRESS         (0x68)

/**
 * @brief JD9366 Configuration Type
 *
 */
typedef struct {
    uint8_t dev_addr;  /*!< I2C device address */
} esp_lcd_touch_io_jd9366_config_t;

/**
 * @brief Touch IO configuration structure
 *
 */
#define ESP_LCD_TOUCH_IO_I2C_JD9366_CONFIG()        \
    {                                       \
        .dev_addr = ESP_LCD_TOUCH_IO_I2C_JD9366_ADDRESS, \
        .control_phase_bytes = 1,           \
        .dc_bit_offset = 0,                 \
        .lcd_cmd_bits = 32,                 \
        .flags =                            \
        {                                   \
            .disable_control_phase = 1,     \
        }                                   \
    }

// // JD9366特定配置结构
// typedef struct {
//     // 预留将来的JD9366特定配置选项
//     bool enable_debug;          // 启用调试模式
//     uint8_t sensitivity;        // 触摸灵敏度
// } esp_lcd_touch_io_jd9366_config_t;

#ifdef __cplusplus
}
#endif 