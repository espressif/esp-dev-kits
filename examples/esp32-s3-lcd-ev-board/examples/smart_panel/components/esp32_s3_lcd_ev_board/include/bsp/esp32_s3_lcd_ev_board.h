/*
 * SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ESP BSP: S3-LCD-EV board
 */

#pragma once

#include "driver/i2s_std.h"
#include "driver/gpio.h"
#include "soc/usb_pins.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_codec_dev.h"
#include "esp_err.h"
#include "esp_io_expander.h"
#include "esp_lcd_gc9503.h"
#include "iot_button.h"
#include "lvgl.h"

#include "sdkconfig.h"

/**************************************************************************************************
 *  ESP32-S3-LCD-EV-Board Pinout
 **************************************************************************************************/
/* I2C */
#define BSP_I2C_SCL             (GPIO_NUM_18)
#define BSP_I2C_SDA             (GPIO_NUM_8)

// Pins for board using ESP32-S3-WROOM-1-N16R16V
#define BSP_I2C_SCL_R16         (GPIO_NUM_48)
#define BSP_I2C_SDA_R16         (GPIO_NUM_47)

/* Audio */
#define BSP_I2S_SCLK            (GPIO_NUM_16)
#define BSP_I2S_MCLK            (GPIO_NUM_5)
#define BSP_I2S_LCLK            (GPIO_NUM_7)
#define BSP_I2S_DOUT            (GPIO_NUM_6)    // To Codec ES8311
#define BSP_I2S_DSIN            (GPIO_NUM_15)   // From ADC ES7210
#define BSP_POWER_AMP_IO        (IO_EXPANDER_PIN_NUM_0)

/* Display */
#define BSP_LCD_SUB_BOARD_2_3_VSYNC     (GPIO_NUM_3)
#define BSP_LCD_SUB_BOARD_2_3_HSYNC     (GPIO_NUM_46)
#define BSP_LCD_SUB_BOARD_2_3_DE        (GPIO_NUM_17)
#define BSP_LCD_SUB_BOARD_2_3_PCLK      (GPIO_NUM_9)
#define BSP_LCD_SUB_BOARD_2_3_DISP      (GPIO_NUM_NC)
#define BSP_LCD_SUB_BOARD_2_3_DATA0     (GPIO_NUM_10)
#define BSP_LCD_SUB_BOARD_2_3_DATA1     (GPIO_NUM_11)
#define BSP_LCD_SUB_BOARD_2_3_DATA2     (GPIO_NUM_12)
#define BSP_LCD_SUB_BOARD_2_3_DATA3     (GPIO_NUM_13)
#define BSP_LCD_SUB_BOARD_2_3_DATA4     (GPIO_NUM_14)
#define BSP_LCD_SUB_BOARD_2_3_DATA5     (GPIO_NUM_21)
#define BSP_LCD_SUB_BOARD_2_3_DATA6     (GPIO_NUM_47)
#define BSP_LCD_SUB_BOARD_2_3_DATA7     (GPIO_NUM_48)
#define BSP_LCD_SUB_BOARD_2_3_DATA8     (GPIO_NUM_45)
#define BSP_LCD_SUB_BOARD_2_3_DATA9     (GPIO_NUM_38)
#define BSP_LCD_SUB_BOARD_2_3_DATA10    (GPIO_NUM_39)
#define BSP_LCD_SUB_BOARD_2_3_DATA11    (GPIO_NUM_40)
#define BSP_LCD_SUB_BOARD_2_3_DATA12    (GPIO_NUM_41)
#define BSP_LCD_SUB_BOARD_2_3_DATA13    (GPIO_NUM_42)
#define BSP_LCD_SUB_BOARD_2_3_DATA14    (GPIO_NUM_2)
#define BSP_LCD_SUB_BOARD_2_3_DATA15    (GPIO_NUM_1)

// Pins for board using ESP32-S3-WROOM-1-N16R16V
#define BSP_LCD_SUB_BOARD_2_3_DATA6_R16 (GPIO_NUM_8)
#define BSP_LCD_SUB_BOARD_2_3_DATA7_R16 (GPIO_NUM_18)

#define BSP_LCD_SUB_BOARD_2_SPI_CS      (IO_EXPANDER_PIN_NUM_1)
#define BSP_LCD_SUB_BOARD_2_SPI_SCK     (IO_EXPANDER_PIN_NUM_2)
#define BSP_LCD_SUB_BOARD_2_SPI_SDO     (IO_EXPANDER_PIN_NUM_3)

/* USB */
#define BSP_USB_POS             (USBPHY_DP_NUM)
#define BSP_USB_NEG             (USBPHY_DM_NUM)

/* Button */

#define BSP_BUTTON_BOOT_IO      (GPIO_NUM_0)

typedef enum {
    BSP_BUTTON_BOOT = 0,
    BSP_BUTTON_NUM,
} bsp_button_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief BSP display configuration structure
 *
 */
typedef struct {
    void *dummy;    /*!< Prepared for future use. */
} bsp_display_cfg_t;

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
 * SPIFFS
 *
 * After mounting the SPIFFS, it can be accessed with stdio functions ie.:
 * \code{.c}
 * FILE* f = fopen(BSP_SPIFFS_MOUNT_POINT"/hello.txt", "w");
 * fprintf(f, "Hello World!\n");
 * fclose(f);
 * \endcode
 **************************************************************************************************/
#define BSP_SPIFFS_MOUNT_POINT      CONFIG_BSP_SPIFFS_MOUNT_POINT

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

/**************************************************************************************************
 *
 * IO Expander Interface
 *
 **************************************************************************************************/
#define BSP_IO_EXPANDER_I2C_ADDRESS     (ESP_IO_EXPANDER_I2C_TCA9554_ADDRESS_000)

/**
 * @brief Init IO expander chip TCA9554
 *
 * @note If the device was already initialized, users can also use it to get handle.
 * @note This function will be called in `bsp_display_start()` when using LCD sub-board 2 with the resolution of 480x480.
 * @note This function will be called in `bsp_audio_init()`.
 *
 * @return Pointer to device handle or NULL when error occured
 */
esp_io_expander_handle_t bsp_io_expander_init(void);

/**************************************************************************************************
 *
 * I2S audio interface
 *
 * There are two devices connected to the I2S peripheral:
 *  - Codec ES8311 for output (playback) path
 *  - ADC ES7210 for input (recording) path
 *
 * For speaker initialization use `bsp_audio_codec_speaker_init()` which is inside initialize I2S with `bsp_audio_init()`.
 * For microphone initialization use `bsp_audio_codec_microphone_init()` which is inside initialize I2S with `bsp_audio_init()`.
 * After speaker or microphone initialization, use functions from esp_codec_dev for play/record audio.
 * Example audio play:
 * \code{.c}
 * esp_codec_dev_set_out_vol(spk_codec_dev, DEFAULT_VOLUME);
 * esp_codec_dev_open(spk_codec_dev, &fs);
 * esp_codec_dev_write(spk_codec_dev, wav_bytes, bytes_read_from_spiffs);
 * esp_codec_dev_close(spk_codec_dev);
 * \endcode
 **************************************************************************************************/
/**
 * @brief Init audio
 *
 * @note  There is no deinit audio function. Users can free audio resources by calling `i2s_del_channel()`.
 * @note  This function wiil call `bsp_io_expander_init()` to setup and enable the control pin of audio power amplifier.
 * @note  This function will be called in `bsp_audio_codec_speaker_init()` and `bsp_audio_codec_microphone_init()`.
 *
 * @param[in] i2s_config I2S configuration. Pass NULL to use default values (Mono, duplex, 16bit, 22050 Hz)
 * @return
 *      - ESP_OK                On success
 *      - ESP_ERR_NOT_SUPPORTED The communication mode is not supported on the current chip
 *      - ESP_ERR_INVALID_ARG   NULL pointer or invalid configuration
 *      - ESP_ERR_NOT_FOUND     No available I2S channel found
 *      - ESP_ERR_NO_MEM        No memory for storing the channel information
 *      - ESP_ERR_INVALID_STATE This channel has not initialized or already started
 *      - other error codes
 */
esp_err_t bsp_audio_init(const i2s_std_config_t *i2s_config);

/**
 * @brief Initialize speaker codec device
 *
 * @note  This function will call `bsp_audio_init()` if it has not been called already.
 *
 * @return Pointer to codec device handle or NULL when error occured
 */
esp_codec_dev_handle_t bsp_audio_codec_speaker_init(void);

/**
 * @brief Initialize microphone codec device
 *
 * @note  This function will call `bsp_audio_init()` if it has not been called already.
 *
 * @return Pointer to codec device handle or NULL when error occured
 */
esp_codec_dev_handle_t bsp_audio_codec_microphone_init(void);

/**
 * @brief Enable/disable audio power amplifier (deprecated)
 *
 * @param[in] enable: Enable/disable audio power amplifier
 *
 * @return
 *      - ESP_OK:               On success
 *      - ESP_ERR_INVALID_ARG:  Invalid GPIO number
 */
esp_err_t bsp_audio_poweramp_enable(bool enable);

/********************************************************************************************************************************
 *
 * Display Interface
 *
 * ESP32-S3-LCD-EV-Board supports three different LCD sub-boards with different resolution and interface:
 *
 *      |-------------|--------------------|---------------------------|---------------------------|-----------------|---------|
 *      |             | Screen Size (inch) | Resolution (width*height) | LCD Driver IC (Interface) | Touch Driver IC | Support |
 *      |-------------|--------------------|---------------------------|---------------------------|-----------------|---------|
 *      |             |        0.9         |        128 * 64           |        SSD1315 (I2C)      |      None       | Not yet |
 *      | Sub board 1 |--------------------|---------------------------|---------------------------|-----------------|---------|
 *      |             |        2.4         |        320 * 240          |        ST7789V (SPI)      |     XTP2046     | Not yet |
 *      |-------------|--------------------|---------------------------|---------------------------|-----------------|---------|
 *      |             |        3.5         |        480 * 320          |        ST7796S (8080)     |     GT911       | Not yet |
 *      | Sub board 2 |--------------------|---------------------------|---------------------------|-----------------|---------|
 *      |             |        3.95        |        480 * 480          |        GC9503CV (RGB)     |     FT5x06      | Yes     |
 *      |-------------|--------------------|---------------------------|---------------------------|-----------------|---------|
 *      | Sub board 3 |        4.3         |        800 * 480          |       ST7262E43 (RGB)     |     GT1151      | Yes     |
 *      |-------------|--------------------|---------------------------|---------------------------|-----------------|---------|
 *
 * LVGL is used as graphics library. LVGL is NOT thread safe, therefore the user must take LVGL mutex
 * by calling `bsp_display_lock()` before calling any LVGL API (lv_...) and then give the mutex with
 * `bsp_display_unlock()`.
 *
 *******************************************************************************************************************************/
/* LCD related parameters */
#define BSP_LCD_SUB_BOARD_2_H_RES     (480)
#define BSP_LCD_SUB_BOARD_2_V_RES     (480)

#define SUB_BOARD2_480_480_PANEL_60HZ_RGB_TIMING()  GC9503_480_480_PANEL_60HZ_RGB_TIMING()
#define SUB_BOARD2_480_480_PANEL_SCL_ACTIVE_EDGE    (0)     // Rising edge, 1: Falling edge

#define BSP_LCD_SUB_BOARD_3_H_RES     (800)
#define BSP_LCD_SUB_BOARD_3_V_RES     (480)

#define SUB_BOARD3_800_480_PANEL_35HZ_RGB_TIMING()  \
    {                                               \
        .pclk_hz = 18 * 1000 * 1000,                \
        .h_res = BSP_LCD_SUB_BOARD_3_H_RES,         \
        .v_res = BSP_LCD_SUB_BOARD_3_V_RES,         \
        .hsync_pulse_width = 40,                    \
        .hsync_back_porch = 40,                     \
        .hsync_front_porch = 48,                    \
        .vsync_pulse_width = 23,                    \
        .vsync_back_porch = 32,                     \
        .vsync_front_porch = 13,                    \
        .flags.pclk_active_neg = true,              \
    }

#define BSP_LCD_H_RES   bsp_display_get_h_res()
#define BSP_LCD_V_RES   bsp_display_get_v_res()

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
 * @brief Initialize display
 *
 * This function initializes SPI, display controller and starts LVGL handling task.
 * LCD backlight must be enabled separately by calling `bsp_display_brightness_set()`
 *
 * @param cfg display configuration
 *
 * @return Pointer to LVGL display or NULL when error occured
 */
lv_disp_t *bsp_display_start_with_config(const bsp_display_cfg_t *cfg);

/**
 * @brief Get pointer to input device (touch, buttons, ...)
 *
 * @note  The LVGL input device is initialized in `bsp_display_start()` function.
 * @note  This function should be called after calling `bsp_display_start()`.
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
 * @note  Display must be already initialized by calling `bsp_display_start()`
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
 * @note  Display must be already initialized by calling `bsp_display_start()`
 *
 */
void bsp_display_unlock(void);

/**
 * @brief Rotate screen
 *
 * @note  Display must be already initialized by calling `bsp_display_start()`
 * @note  This function can't work with the anti-tearing function. Please use the `BSP_DISPLAY_LVGL_ROTATION` configuration instead.
 *
 * @param[in] disp:     Pointer to LVGL display
 * @param[in] rotation: Angle of the display rotation
 */
void bsp_display_rotate(lv_disp_t *disp, lv_disp_rot_t rotation);

/**
 * @brief Get display horizontal resolution
 *
 * @note  This function should be called after calling `bsp_display_new()` or `bsp_display_start()`
 *
 * @return Horizontal resolution. Return 0 if error occured.
 */
uint16_t bsp_display_get_h_res(void);

/**
 * @brief Get display vertical resolution
 *
 * @note  This function should be called after calling `bsp_display_new()` or `bsp_display_start()`
 *
 * @return Vertical resolution. Return 0 if error occured.
 */
uint16_t bsp_display_get_v_res(void);

/**************************************************************************************************
 *
 * Button Interface
 *
 * There are two buttons on ESP32-S3-LCD-EV-Board:
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
esp_err_t bsp_button_init(const bsp_button_t btn)
__attribute__((deprecated("use espressif/button API instead")));

/**
 * @brief Get button's state
 *
 * @param[in] btn: Button to read
 *
 * @return
 *      - true:  Button pressed
 *      - false: Button released
 */
bool bsp_button_get(const bsp_button_t btn)
__attribute__((deprecated("use espressif/button API instead")));

/**
 * @brief Initialize all buttons
 *
 * Returned button handlers must be used with espressif/button component API
 *
 * @note For LCD panel button which is defined as BSP_BUTTON_MAIN, bsp_display_start should
 *       be called before call this function.
 *
 * @param[out] btn_array      Output button array
 * @param[out] btn_cnt        Number of button handlers saved to btn_array, can be NULL
 * @param[in]  btn_array_size Size of output button array. Must be at least BSP_BUTTON_NUM
 * @return
 *     - ESP_OK               All buttons initialized
 *     - ESP_ERR_INVALID_ARG  btn_array is too small or NULL
 *     - ESP_FAIL             Underlaying iot_button_create failed
 */
esp_err_t bsp_iot_button_create(button_handle_t btn_array[], int *btn_cnt, int btn_array_size);

/**************************************************************************************************
 *
 * ADC interface
 *
 * After initialization of ADC, use `adc_handle` when using ADC driver.
 *
 **************************************************************************************************/
#define BSP_ADC_UNIT     ADC_UNIT_1

/**
 * @brief Initialize ADC
 *
 * The ADC can be initialized inside BSP, when needed.
 *
 * @param[out] adc_handle Returned ADC handle
 */
esp_err_t bsp_adc_initialize(void);


/**
 * @brief Get ADC handle
 *
 * @note This function is available only in IDF5 and higher
 *
 * @return ADC handle
 */
adc_oneshot_unit_handle_t bsp_adc_get_handle(void);

#ifdef __cplusplus
}
#endif
