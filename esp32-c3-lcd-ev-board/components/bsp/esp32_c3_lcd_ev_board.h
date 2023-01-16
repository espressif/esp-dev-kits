/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "driver/gpio.h"
#include "lvgl.h"

/**************************************************************************************************
 *  ESP32_C3_LCD_EV_BOARD pinout
 **************************************************************************************************/
/* Light */
#define BSP_RGB_CTRL          (GPIO_NUM_8)

/* Display */
#define BSP_LCD_DATA0         (GPIO_NUM_0)
#define BSP_LCD_PCLK          (GPIO_NUM_1)
#define BSP_LCD_CS            (GPIO_NUM_10)
#define BSP_LCD_DC            (GPIO_NUM_4)
#define BSP_LCD_RST           (GPIO_NUM_2)
#define BSP_LCD_BACKLIGHT     (GPIO_NUM_5)

/* USB */
#define BSP_USB_POS           USBPHY_DP_NUM
#define BSP_USB_NEG           USBPHY_DM_NUM

/* Buttons */
typedef enum {
    BSP_BTN_PRESS = GPIO_NUM_9,
} bsp_button_t;

#define BSP_ENCODER_A         (GPIO_NUM_7)
#define BSP_ENCODER_B         (GPIO_NUM_6)

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************
 *
 * LCD interface
 *
 * ESP32_C3_LCD_EV_BOARD is shipped with 1.28inch GC9A01 display controller.
 * It features 16-bit colors, 240x240 resolution.
 *
 * LVGL is used as graphics library. LVGL is NOT thread safe, therefore the user must take LVGL mutex
 * by calling bsp_display_lock() before calling and LVGL API (lv_...) and then give the mutex with
 * bsp_display_unlock().
 *
 * Display's backlight must be enabled explicitly by calling bsp_display_backlight_on()
 **************************************************************************************************/
#define BSP_LCD_H_RES              (240)
#define BSP_LCD_V_RES              (240)
#define BSP_LCD_PIXEL_CLOCK_HZ     (80 * 1000 * 1000)
#define BSP_LCD_SPI_NUM            (SPI2_HOST)

#define BSP_LCD_DRAW_BUF_HEIGHT     (240)
#define BSP_LCD_DRAW_BUF_DOUBLE     (0)

/**
 * @brief Initialize display
 *
 * This function initializes SPI, display controller and starts LVGL handling task.
 * LCD backlight must be enabled separately by calling bsp_display_brightness_set()
 *
 * @return Pointer to LVGL display or NULL when error occured
 */
lv_disp_t *bsp_display_start(void);

/**
 * @brief Take LVGL mutex
 *
 * @param timeout_ms Timeout in [ms]. 0 will block indefinitely.
 * @return true  Mutex was taken
 * @return false Mutex was NOT taken
 */
bool bsp_display_lock(uint32_t timeout_ms);

/**
 * @brief Give LVGL mutex
 *
 */
void bsp_display_unlock(void);

/**
 * @brief Set display's brightness
 *
 * Brightness is controlled with PWM signal to a pin controling backlight.
 *
 * @param[in] brightness_percent Brightness in [%]
 * @return
 *      - ESP_OK                On success
 *      - ESP_ERR_INVALID_ARG   Parameter error
 */
esp_err_t bsp_display_brightness_set(int brightness_percent);

/**
 * @brief Turn on display backlight
 *
 * Display must be already initialized by calling bsp_display_start()
 *
 * @return
 *      - ESP_OK                On success
 *      - ESP_ERR_INVALID_ARG   Parameter error
 */
esp_err_t bsp_display_backlight_on(void);

/**
 * @brief Turn off display backlight
 *
 * Display must be already initialized by calling bsp_display_start()
 *
 * @return
 *      - ESP_OK                On success
 *      - ESP_ERR_INVALID_ARG   Parameter error
 */
esp_err_t bsp_display_backlight_off(void);

/**
 * @brief Rotate screen
 *
 * Display must be already initialized by calling bsp_display_start()
 *
 * @param[in] disp Pointer to LVGL display
 * @param[in] rotation Angle of the display rotation
 */
void bsp_display_rotate(lv_disp_t *disp, lv_disp_rot_t rotation);
/**************************************************************************************************
 *
 * Button
 *
 * There are 2 buttons on ESP32_C3_LCD_EV_BOARD:
 *  - Reset:  Not programable
 *  - Config: Controls boot mode during reset. Can be programmed after application starts
 **************************************************************************************************/

/**
 * @brief Set button's GPIO as input
 *
 * @param[in] btn Button to be initialized
 * @return
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 */
esp_err_t bsp_button_init(const bsp_button_t btn);

/**
 * @brief Get button's state
 *
 * @param[in] btn Button to read
 * @return true  Button pressed
 * @return false Button released
 */
bool bsp_button_get(const bsp_button_t btn);
#ifdef __cplusplus
}
#endif
