/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "driver/gpio.h"
#include "driver/i2s_pdm.h"
#include "lvgl.h"

#define HARDWARE_V1_0           0

/**************************************************************************************************
 *  ESP32_C3_LCD_EV_BOARD pinout
 **************************************************************************************************/

/* Display */
#if HARDWARE_V1_0

/* Light */
#define BSP_RGB_CTRL          (GPIO_NUM_3)

/* Audio */
#define BSP_I2S_DOUT          (GPIO_NUM_20)//TX
#define BSP_I2S_CTRL          (GPIO_NUM_21)//RX
#define BSP_I2S_CLK           (GPIO_NUM_NC)

#define BSP_LCD_DATA0         (GPIO_NUM_0)
#define BSP_LCD_PCLK          (GPIO_NUM_1)
#define BSP_LCD_CS            (GPIO_NUM_10)
#define BSP_LCD_DC            (GPIO_NUM_4)
#define BSP_LCD_RST           (GPIO_NUM_2)
// #define BSP_LCD_BACKLIGHT     (GPIO_NUM_5)
#define BSP_LCD_BACKLIGHT     (GPIO_NUM_8) //yysj
#else

/* Light */
#define BSP_RGB_CTRL          (GPIO_NUM_8)

/* Audio */
#define BSP_I2S_DOUT          (GPIO_NUM_3)
#define BSP_I2S_CLK           (GPIO_NUM_NC)

#define BSP_LCD_DATA0         (GPIO_NUM_0)
#define BSP_LCD_PCLK          (GPIO_NUM_1)
#define BSP_LCD_CS            (GPIO_NUM_7)
#define BSP_LCD_DC            (GPIO_NUM_2)
#define BSP_LCD_RST           (GPIO_NUM_NC)
#define BSP_LCD_BACKLIGHT     (GPIO_NUM_5)
#endif

/* USB */
#define BSP_USB_POS           USBPHY_DP_NUM
#define BSP_USB_NEG           USBPHY_DM_NUM

/* Buttons */
typedef enum {
    BSP_BTN_PRESS = GPIO_NUM_9,
} bsp_button_t;

#if HARDWARE_V1_0
#define BSP_ENCODER_A         (GPIO_NUM_6)
#define BSP_ENCODER_B         (GPIO_NUM_7)
#else
#define BSP_ENCODER_A         (GPIO_NUM_10)
#define BSP_ENCODER_B         (GPIO_NUM_6)
#endif

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

/**************************************************************************************************
 *
 * WS2812
 *
 * There's one RGB light on ESP32_C3_LCD_EV_BOARD:
 **************************************************************************************************/

/**
 * @brief Initialize WS2812
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 */
esp_err_t bsp_led_init();

/**
 * @brief Set RGB for a specific pixel
 *
 * @param r: red part of color
 * @param g: green part of color
 * @param b: blue part of color
 *
 * @return
 *      - ESP_OK: Set RGB for a specific pixel successfully
 *      - ESP_ERR_INVALID_ARG: Set RGB for a specific pixel failed because of invalid parameters
 *      - ESP_FAIL: Set RGB for a specific pixel failed because other error occurred
 */
esp_err_t bsp_led_RGB_set(uint8_t r, uint8_t g, uint8_t b);


/**************************************************************************************************
 *
 * I2S audio interface
 *
 * There's one device connected to the I2S peripheral:
 *
 * After initialization of I2S with bsp_audio_init(), use standard I2S drive API for writing to I2S stream:
 * \code{.c}
 * i2s_write_channel(tx_handle, wav_bytes, wav_bytes_len, &i2s_bytes_written, pdMS_TO_TICKS(500));
 * \endcode
 **************************************************************************************************/
/**
 * @brief ESP32_C3_LCD_EV_BOARD I2S pinout
 *
 * Can be used for i2s_std_gpio_config_t and/or i2s_std_config_t initialization
 */
#define BSP_I2S_GPIO_CFG       \
    {                          \
        .clk = BSP_I2S_CLK,    \
        .dout = BSP_I2S_DOUT,  \
        .invert_flags = {      \
            .clk_inv = false, \
        },                     \
    }

/**
 * @brief Mono Duplex I2S configuration structure
 *
 * This configuration is used by default in bsp_audio_init()
 */
#define BSP_I2S_DUPLEX_MONO_CFG(_sample_rate)                                                         \
    {                                                                                                 \
        .clk_cfg = I2S_PDM_TX_CLK_DEFAULT_CONFIG(_sample_rate),                                          \
        .slot_cfg = I2S_PDM_TX_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO), \
        .gpio_cfg = BSP_I2S_GPIO_CFG,                                                                 \
    }

/**
 * @brief Audio write data
 *
 * @param[in]   audio_buffer    The pointer of sent data buffer
 * @param[in]   len             Max data buffer length
 * @param[out]  bytes_written   Byte number that actually be sent
 * @param[in]   timeout_ms      Max block time
 * @return
 *      - ESP_OK    Write successfully
 *      - ESP_ERR_INVALID_ARG   NULL pointer or this handle is not tx handle
 *      - ESP_ERR_TIMEOUT       Writing timeout, no writing event received from ISR within ticks_to_wait
 *      - ESP_ERR_INVALID_STATE I2S is not ready to write
 */
esp_err_t bsp_audio_write(void *audio_buffer, size_t len, size_t *bytes_written, uint32_t timeout_ms);

/**
 * @brief Reconfigure the I2S slot/clock for PDM TX mode
 *
 * @param[in]   rate        sample_rate_hz
 * @param[in]   bits_cfg    i2s_data_bit_width
 * @param[in]   ch          i2s_slot_mode
 * @return
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 */
esp_err_t bsp_audio_reconfig_clk(uint32_t rate, uint32_t bits_cfg, i2s_slot_mode_t ch);

/**
 * @brief Initialize Led, Audio, spiffs
 *
 * @return
 *     - ESP_OK Success
 */
esp_err_t bsp_board_init(void);

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
