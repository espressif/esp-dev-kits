/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

/**
 * @file
 * @brief ESP BSP: S3-LCD-EV board
 */

#pragma once

#include "driver/i2s_std.h"
#include "driver/gpio.h"
#include "soc/usb_pins.h"
#include "esp_io_expander.h"
#include "lvgl.h"

#include "sdkconfig.h"

/**************************************************************************************************
 *  ESP32-S3-LCD-EV-BOARD Pinout
 **************************************************************************************************/
/* I2C */
#define BSP_I2C_SCL             (GPIO_NUM_18)
#define BSP_I2C_SDA             (GPIO_NUM_8)

/* Audio */
#define BSP_I2S_SCLK            (GPIO_NUM_16)
#define BSP_I2S_MCLK            (GPIO_NUM_5)
#define BSP_I2S_LCLK            (GPIO_NUM_7)
#define BSP_I2S_DOUT            (GPIO_NUM_6)    // To Codec ES8311
#define BSP_I2S_DSIN            (GPIO_NUM_15)   // From ADC ES7210
#define BSP_POWER_AMP_IO        (IO_EXPANDER_PIN_NUM_0)

/* Display */
#if CONFIG_BSP_LCD_SUB_BOARD_800_480 || CONFIG_BSP_LCD_SUB_BOARD_480_480
#define BSP_LCD_VSYNC           (GPIO_NUM_3)
#define BSP_LCD_HSYNC           (GPIO_NUM_46)
#define BSP_LCD_DE              (GPIO_NUM_17)
#define BSP_LCD_PCLK            (GPIO_NUM_9)
#define BSP_LCD_DATA0           (GPIO_NUM_10)   // B3
#define BSP_LCD_DATA1           (GPIO_NUM_11)   // B4
#define BSP_LCD_DATA2           (GPIO_NUM_12)   // B5
#define BSP_LCD_DATA3           (GPIO_NUM_13)   // B6
#define BSP_LCD_DATA4           (GPIO_NUM_14)   // B7
#define BSP_LCD_DATA5           (GPIO_NUM_21)   // G2
#define BSP_LCD_DATA6           (GPIO_NUM_47)   // G3
#define BSP_LCD_DATA7           (GPIO_NUM_48)   // G4
#define BSP_LCD_DATA8           (GPIO_NUM_45)   // G5
#define BSP_LCD_DATA9           (GPIO_NUM_38)   // G6
#define BSP_LCD_DATA10          (GPIO_NUM_39)   // G7
#define BSP_LCD_DATA11          (GPIO_NUM_40)   // R3
#define BSP_LCD_DATA12          (GPIO_NUM_41)   // R4
#define BSP_LCD_DATA13          (GPIO_NUM_42)   // R5
#define BSP_LCD_DATA14          (GPIO_NUM_2)    // R6
#define BSP_LCD_DATA15          (GPIO_NUM_1)    // R7
#define BSP_LCD_SPI_CS          (IO_EXPANDER_PIN_NUM_1)
#define BSP_LCD_SPI_SCK         (IO_EXPANDER_PIN_NUM_2)
#define BSP_LCD_SPI_SDO         (IO_EXPANDER_PIN_NUM_3)
#endif

/* USB */
#define BSP_USB_POS             (USBPHY_DP_NUM)
#define BSP_USB_NEG             (USBPHY_DM_NUM)

/* Button */
typedef enum {
    BSP_BUTTON_BOOT = GPIO_NUM_0,
} bsp_button_t;

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************
 *
 * I2C Interface
 *
 * There are multiple devices connected to I2C peripheral:
 *  - Codec ES8311 (configuration only)
 *  - ADC ES7210 (configuration only)
 *  - LCD Touch controller
 *  - IO expander chip TCA9554
 *
 * After initialization of I2C, use `BSP_I2C_NUM` macro when creating I2C devices drivers ie.:
 * \code{.c}
 * es8311_handle_t es8311_dev = es8311_create(BSP_I2C_NUM, ES8311_ADDRRES_0);
 * \endcode
 *
 **************************************************************************************************/
#define BSP_I2C_NUM             (CONFIG_BSP_I2C_NUM)

/**
 * @brief Init I2C driver
 *
 * @return
 *      - ESP_OK:               On success
 *      - ESP_ERR_INVALID_ARG:  I2C parameter error
 *      - ESP_FAIL:             I2C driver installation error
 *
 */
esp_err_t bsp_i2c_init(void);

/**
 * @brief Deinit I2C driver and free its resources
 *
 * @return
 *      - ESP_OK:               On success
 *      - ESP_ERR_INVALID_ARG:  I2C parameter error
 *
 */
esp_err_t bsp_i2c_deinit(void);

/**************************************************************************************************
 *
 * IO Expander Interface
 *
 **************************************************************************************************/
#define BSP_IO_EXPANDER_I2C_ADDRESS     (ESP_IO_EXPANDER_I2C_TCA9554_ADDRESS_000)

/**
 * @brief Init IO expander chip TCA9554
 *
 * @note If the device was already initialized, users can also call it to get handle
 * @note This function will be called in `bsp_display_start()` when using LCD sub-board 2 with the resolution of 480x480
 *
 * @return Pointer to device handle or NULL when error occured
 */
esp_io_expander_handle_t bsp_io_expander_init(void);

/**************************************************************************************************
 *
 * I2S Audio Interface
 *
 * There are two devices connected to the I2S peripheral:
 *  - Codec ES8311 for output (playback) path
 *  - ADC ES7210 for input (recording) path
 *
 * After initialization of I2S with `bsp_audio_init()`, use standard I2S drive API for reading/writing to I2S stream:
 * \code{.c}
 * i2s_write_channel(tx_handle, wav_bytes, wav_bytes_len, &i2s_bytes_written, pdMS_TO_TICKS(500));
 * \endcode
 *
 **************************************************************************************************/
/**
 * @brief ESP32-S3-LCD-EV-BOARD I2S pinout
 *
 * Can be used for `i2s_std_gpio_config_t` and/or `i2s_std_config_t` initialization
 */
#define BSP_I2S_GPIO_CFG       \
    {                          \
        .mclk = BSP_I2S_MCLK,  \
        .bclk = BSP_I2S_SCLK,  \
        .ws = BSP_I2S_LCLK,    \
        .dout = BSP_I2S_DOUT,  \
        .din = BSP_I2S_DSIN,   \
        .invert_flags = {      \
            .mclk_inv = false, \
            .bclk_inv = false, \
            .ws_inv = false,   \
        },                     \
    }

/**
 * @brief Mono Duplex I2S configuration structure
 *
 * This configuration is used by default in `bsp_audio_init()`
 */
#define BSP_I2S_DUPLEX_MONO_CFG(_sample_rate)                                                         \
    {                                                                                                 \
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(_sample_rate),                                          \
        .slot_cfg = I2S_STD_PHILIP_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO), \
        .gpio_cfg = BSP_I2S_GPIO_CFG,                                                                 \
    }

/**
 * @brief ES8311 init structure
 *
 */
#define BSP_ES8311_SCLK_CONFIG(_sample_rate) \
    {                                        \
        .mclk_from_mclk_pin = false,         \
        .mclk_inverted = false,              \
        .sclk_inverted = false,              \
        .sample_frequency = _sample_rate     \
    }

/**
 * @brief Init audio
 *
 * @note There is no deinit audio function. Users can free audio resources by calling `i2s_del_channel()`
 *
 * @param[in]  i2s_config: I2S configuration. Pass NULL to use default values (Mono, duplex, 16bit, 22050 Hz)
 * @param[out] tx_channel: I2S TX channel
 * @param[out] rx_channel: I2S RX channel
 *
 * @return
 *      - ESP_OK:                   On success
 *      - ESP_ERR_NOT_SUPPORTED:    The communication mode is not supported on the current chip
 *      - ESP_ERR_INVALID_ARG:      NULL pointer or invalid configuration
 *      - ESP_ERR_NOT_FOUND:        No available I2S channel found
 *      - ESP_ERR_NO_MEM:           No memory for storing the channel information
 *      - ESP_ERR_INVALID_STATE:    This channel has not initialized or already started
 */
esp_err_t bsp_audio_init(const i2s_std_config_t *i2s_config, i2s_chan_handle_t *tx_channel, i2s_chan_handle_t *rx_channel);

/**
 * @brief Enable/disable audio power amplifier
 *
 * @param[in] enable: Enable/disable audio power amplifier
 *
 * @return
 *      - ESP_OK:               On success
 *      - ESP_ERR_INVALID_ARG:  Invalid GPIO number
 */
esp_err_t bsp_audio_poweramp_enable(const bool enable);

/********************************************************************************************************************************
 *
 * Display Interface
 *
 * ESP32-S3-LCD-EV-BOARD supports three different LCD sub-boards with different resolution and interface:
 *
 *      ┌─────────────┬────────────────────┬───────────────────────────┬───────────────────────────┬─────────────────┬─────────┐
 *      |             │ Screen Size (inch) │ Resolution (width*height) │ LCD Driver IC (Interface) │ Touch Driver IC │ Support |
 *      ├─────────────┼────────────────────┼───────────────────────────┼───────────────────────────┼─────────────────┼─────────┤
 *      |             │        0.9         │        128 * 64           │        SSD1315 (I2C)      │      None       │ Not yet │
 *      | Sub board 1 ├────────────────────┼───────────────────────────┼───────────────────────────┼─────────────────┼─────────┤
 *      |             │        2.4         │        320 * 240          │        ST7789V (SPI)      │     XTP2046     │ Not yet │
 *      ├─────────────┼────────────────────┼───────────────────────────┼───────────────────────────┼─────────────────┼─────────┤
 *      |             │        3.5         │        480 * 320          │        ST7796S (8080)     │     GT911       │ Not yet │
 *      | Sub board 2 ├────────────────────┼───────────────────────────┼───────────────────────────┼─────────────────┼─────────┤
 *      |             │        3.95        │        480 * 480          │        GC9503CV (RGB)     │     FT5x06      │ Yes     │
 *      ├─────────────┼────────────────────┼───────────────────────────┼───────────────────────────┼─────────────────┼─────────┤
 *      | Sub board 3 │        4.3         │        800 * 480          │        Unkonw (RGB)       │     GT1151      │ Yes     │
 *      └─────────────┴────────────────────┴───────────────────────────┴───────────────────────────┴─────────────────┴─────────┘
 *
 * LVGL is used as graphics library. LVGL is NOT thread safe, therefore the user must take LVGL mutex
 * by calling `bsp_display_lock()` before calling any LVGL API (lv_...) and then give the mutex with
 * `bsp_display_unlock()`.
 *
 *******************************************************************************************************************************/
/* LCD related parameters */
#if CONFIG_BSP_LCD_SUB_BOARD_800_480
#define BSP_LCD_H_RES                   (800)
#define BSP_LCD_V_RES                   (480)
#define BSP_LCD_PIXEL_CLOCK_HZ          (18 * 1000 * 1000)
#define BSP_LCD_HSYNC_BACK_PORCH        (40)
#define BSP_LCD_HSYNC_FRONT_PORCH       (48)
#define BSP_LCD_HSYNC_PULSE_WIDTH       (40)
#define BSP_LCD_VSYNC_BACK_PORCH        (32)
#define BSP_LCD_VSYNC_FRONT_PORCH       (13)
#define BSP_LCD_VSYNC_PULSE_WIDTH       (23)
#define BSP_LCD_PCLK_ACTIVE_NEG         (true)
#elif CONFIG_BSP_LCD_SUB_BOARD_480_480
#define BSP_LCD_H_RES                   (480)
#define BSP_LCD_V_RES                   (480)
#define BSP_LCD_PIXEL_CLOCK_HZ          (18 * 1000 * 1000)
#define BSP_LCD_HSYNC_BACK_PORCH        (20)
#define BSP_LCD_HSYNC_FRONT_PORCH       (40)
#define BSP_LCD_HSYNC_PULSE_WIDTH       (13)
#define BSP_LCD_VSYNC_BACK_PORCH        (20)
#define BSP_LCD_VSYNC_FRONT_PORCH       (40)
#define BSP_LCD_VSYNC_PULSE_WIDTH       (15)
#define BSP_LCD_PCLK_ACTIVE_NEG         (false)
#endif

/* LVGL related parameters */
#define LVGL_TICK_PERIOD_MS         (CONFIG_BSP_DISPLAY_LVGL_TICK)
#define LVGL_BUFFER_HEIGHT          (CONFIG_BSP_DISPLAY_LVGL_BUF_HEIGHT)
#if CONFIG_BSP_DISPLAY_LVGL_PSRAM
#define LVGL_BUFFER_MALLOC          (MALLOC_CAP_SPIRAM)
#else
#define LVGL_BUFFER_MALLOC          (MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT)
#endif

/**
 * @brief Initialize display
 *
 * @note This function initializes display controller and starts LVGL handling task.
 * @note Users can get LCD panel handle from `user_data` in returned display.
 *
 * @return Pointer to LVGL display or NULL when error occured
 */
lv_disp_t *bsp_display_start(void);

/**
 * @brief Get pointer to input device (touch, buttons, ...)
 *
 * @note The LVGL input device is initialized in bsp_display_start() function.
 *
 * @return Pointer to LVGL input device or NULL when not initialized
 */
lv_indev_t *bsp_display_get_input_dev(void);

/**
 * @brief Set display's brightness (Useless, just for compatibility)
 *
 * @param[in] brightness_percent: Brightness in [%]
 * @return
 *      - ESP_ERR_NOT_SUPPORTED: Always
 */
esp_err_t bsp_display_brightness_set(int brightness_percent);

/**
 * @brief Turn on display backlight (Useless, just for compatibility)
 *
 * @return
 *      - ESP_ERR_NOT_SUPPORTED: Always
 */
esp_err_t bsp_display_backlight_on(void);

/**
 * @brief Turn off display backlight (Useless, just for compatibility)
 *
 * @return
 *      - ESP_ERR_NOT_SUPPORTED: Always
 */
esp_err_t bsp_display_backlight_off(void);

/**
 * @brief Take LVGL mutex
 *
 * @param[in] timeout_ms: Timeout in [ms]. 0 will block indefinitely.
 *
 * @return
 *      - true:  Mutex was taken
 *      - false: Mutex was NOT taken
 */
bool bsp_display_lock(uint32_t timeout_ms);

/**
 * @brief Give LVGL mutex
 *
 */
void bsp_display_unlock(void);

/**
 * @brief Rotate screen
 *
 * @note  Display must be already initialized by calling `bsp_display_start()`
 *
 * @param[in] disp:     Pointer to LVGL display
 * @param[in] rotation: Angle of the display rotation
 */
void bsp_display_rotate(lv_disp_t *disp, lv_disp_rot_t rotation);

/**************************************************************************************************
 *
 * Button Interface
 *
 * There are two buttons on ESP32-S3-LCD-EV-BOARD:
 *  - Reset: Not programable
 *  - Boot:  Controls boot mode during reset. Can be programmed after application starts
 *
 **************************************************************************************************/
/**
 * @brief Set button's GPIO as input
 *
 * @param[in] btn: Button to be initialized
 *
 * @return
 *     - ESP_OK:                Success
 *     - ESP_ERR_INVALID_ARG:   Parameter error
 */
esp_err_t bsp_button_init(const bsp_button_t btn);

/**
 * @brief Get button's state
 *
 * @param[in] btn: Button to read
 *
 * @return
 *      - true:  Button pressed
 *      - false: Button released
 */
bool bsp_button_get(const bsp_button_t btn);

#ifdef __cplusplus
}
#endif
