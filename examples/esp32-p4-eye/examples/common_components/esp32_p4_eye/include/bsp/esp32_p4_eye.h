/*
 * SPDX-FileCopyrightText: 2024-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ESP BSP: p4-eye
 */

#pragma once

#include "sdkconfig.h"
#include "driver/gpio.h"
#include "driver/sdmmc_host.h"
#include "driver/i2s_std.h"
#include "driver/i2s_pdm.h"
#include "driver/i2c_master.h"
#include "esp_codec_dev.h"
#include "iot_button.h"
#include "iot_knob.h"
#include "esp_lvgl_port.h"
#include "bsp/display.h"

/**************************************************************************************************
 *  BSP Capabilities
 **************************************************************************************************/

#define BSP_CAPS_DISPLAY        1
#define BSP_CAPS_TOUCH          0
#define BSP_CAPS_BUTTONS        1
#define BSP_CAPS_AUDIO          1
#define BSP_CAPS_AUDIO_SPEAKER  0
#define BSP_CAPS_AUDIO_MIC      1
#define BSP_CAPS_SDCARD         1
#define BSP_CAPS_IMU            1 // There is an IMU, but not supported in this BSP

/**************************************************************************************************
 * ESP32-P4-EYE pinout
 **************************************************************************************************/

/* I2C */
#define BSP_I2C_SCL           (GPIO_NUM_13)
#define BSP_I2C_SDA           (GPIO_NUM_14)

/* Audio */
#define BSP_I2S_DAT           (GPIO_NUM_21)
#define BSP_I2S_CLK           (GPIO_NUM_22)

/* Display */
#define BSP_LCD_SPI_MOSI      (GPIO_NUM_16)
#define BSP_LCD_SPI_CLK       (GPIO_NUM_17)
#define BSP_LCD_SPI_CS        (GPIO_NUM_18)
#define BSP_LCD_DC            (GPIO_NUM_19)
#define BSP_LCD_RST           (GPIO_NUM_15)
#define BSP_LCD_BACKLIGHT     (GPIO_NUM_20)

#define BSP_KNOB_A            (GPIO_NUM_48)
#define BSP_KNOB_B            (GPIO_NUM_47)

/* Camera */
#define BSP_MIPI_CAMERA_XCLK_FREQUENCY              (24000000)       // Frequency in Hertz
#define BSP_CAMERA_EN_PIN       (GPIO_NUM_12)
#define BSP_CAMERA_RST_PIN      (GPIO_NUM_26)
#define BSP_CAMERA_XCLK_PIN     (GPIO_NUM_11)

/* C6 */
#define BSP_C6_EN_PIN           (GPIO_NUM_9)

/* uSD card */
#define BSP_SD_SLOT_0_DEFAULT_INIT     \
    .clk = 0,                          \
    .cmd = 0,                          \
    .d0  = 0,                          \
    .d1  = 0,                          \
    .d2  = 0,                          \
    .d3  = 0,                          \
    .d4  = 0,                          \
    .d5  = 0,                          \
    .d6  = 0,                          \
    .d7  = 0
#define BSP_SD_EN_PIN         (GPIO_NUM_46)
#define BSP_SD_DETECT_PIN     (GPIO_NUM_45)

/* Buttons */
#define BSP_BUTTON_NUM1      (GPIO_NUM_3)
#define BSP_BUTTON_NUM2      (GPIO_NUM_4)
#define BSP_BUTTON_NUM3      (GPIO_NUM_5)
#define BSP_BUTTON_ENCODER   (GPIO_NUM_2)

typedef enum bsp_led_t {
    BSP_LED_FLASHLIGHT = GPIO_NUM_23,
} bsp_led_t;

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************
 *
 * Buttons interface
 *
 * Example configuration:
 * \code{.c}
 * button_handle_t btns[BSP_BUTTON_NUM];
 * bsp_iot_button_create(btns, NULL, BSP_BUTTON_NUM);
 * iot_button_register_cb(btns[0], ...
 * \endcode
 **************************************************************************************************/
typedef enum {
    BSP_BUTTON_1 = 0,
    BSP_BUTTON_2,
    BSP_BUTTON_3,
    BSP_BUTTON_ED,
    BSP_BUTTON_NUM
} bsp_button_t;

/**
 * @brief Initialize ESP32-P4-EYE board
 * 
 * @return esp_err_t 
 */
esp_err_t bsp_p4_eye_init(void);

/**
 * @brief Initialize sleep IO
 * 
 * @return esp_err_t 
 */
esp_err_t bsp_enter_sleep_init(void);

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
 * I2C interface
 *
 * There are two devices connected to I2C peripheral:
 *  - QMA7981 Inertial measurement unit
 *  - OV2640 Camera module
 **************************************************************************************************/
#define BSP_I2C_NUM     CONFIG_BSP_I2C_NUM

/**
 * @brief Init I2C driver
 *
 * @return
 *      - ESP_OK                On success
 *      - ESP_ERR_INVALID_ARG   I2C parameter error
 *      - ESP_FAIL              I2C driver installation error
 *
 */
esp_err_t bsp_i2c_init(void);

/**
 * @brief Deinit I2C driver and free its resources
 *
 * @return
 *      - ESP_OK                On success
 *      - ESP_ERR_INVALID_ARG   I2C parameter error
 *
 */
esp_err_t bsp_i2c_deinit(void);

/**
 * @brief Get I2C bus handle
 * 
 * @param handle I2C bus handle
 * @return esp_err_t 
 */
esp_err_t bsp_get_i2c_bus_handle(i2c_master_bus_handle_t *handle);

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
 *      - ESP_ERR_INVALID_STATE if already unmounted
 */
esp_err_t bsp_spiffs_unmount(void);

/**************************************************************************************************
 *
 * uSD card
 *
 * After mounting the uSD card, it can be accessed with stdio functions ie.:
 * \code{.c}
 * FILE* f = fopen(BSP_MOUNT_POINT"/hello.txt", "w");
 * fprintf(f, "Hello %s!\n", bsp_sdcard->cid.name);
 * fclose(f);
 * \endcode
 **************************************************************************************************/
#define BSP_SD_MOUNT_POINT      CONFIG_BSP_SD_MOUNT_POINT

/**
 * @brief Mount microSD card to virtual file system
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_STATE if esp_vfs_fat_sdmmc_mount was already called
 *      - ESP_ERR_NO_MEM if memory cannot be allocated
 *      - ESP_FAIL if partition cannot be mounted
 *      - other error codes from SDMMC or SPI drivers, SDMMC protocol, or FATFS drivers
 */
esp_err_t bsp_sdcard_mount(void);

/**
 * @brief Unmount microSD card from virtual file system
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_NOT_FOUND if the partition table does not contain FATFS partition with given label
 *      - ESP_ERR_INVALID_STATE if esp_vfs_fat_spiflash_mount was already called
 *      - ESP_ERR_NO_MEM if memory can not be allocated
 *      - ESP_FAIL if partition can not be mounted
 *      - other error codes from wear levelling library, SPI flash driver, or FATFS drivers
 */
esp_err_t bsp_sdcard_unmount(void);

/**
 * @brief Initialize microSD card detection
 * 
 * @return esp_err_t 
 */
esp_err_t bsp_sdcard_detect_init(void);

/**
 * @brief Check if microSD card is present
 * 
 * @return true if microSD card is present, false otherwise
 */
bool bsp_sdcard_is_present(void);

/**
 * @brief Get microSD card handle
 * 
 * @param[out] card Pointer to sdmmc_card_t
 * @return esp_err_t 
 */
esp_err_t bsp_get_sdcard_handle(sdmmc_card_t **card);

/**
 * @brief Initialize knob
 * 
 * @return esp_err_t 
 */
esp_err_t bsp_knob_init(void);

/**
 * @brief Register knob callback
 * 
 * @param event Knob event
 * @param cb Callback function
 * @param usr_data User data
 * @return esp_err_t 
 */
esp_err_t bsp_knob_register_cb(knob_event_t event, knob_cb_t cb, void *usr_data);

/**************************************************************************************************
 *
 * LCD interface
 *
 * ESP32-P4-EYE is shipped with 1.3inch ST7789 display controller.
 * It features 16-bit colors and 240x240 resolution.
 *
 * LVGL is used as graphics library. LVGL is NOT thread safe, therefore the user must take LVGL mutex
 * by calling bsp_display_lock() before calling any LVGL API (lv_...) and then give the mutex with
 * bsp_display_unlock().
 *
 * If you want to use the display without LVGL, see bsp/display.h API and use BSP version with 'noglib' suffix.
 **************************************************************************************************/
#define BSP_LCD_PIXEL_CLOCK_HZ     (80 * 1000 * 1000)
#define BSP_LCD_SPI_NUM            (SPI2_HOST)

#define BSP_LCD_DRAW_BUFF_SIZE     (BSP_LCD_H_RES * BSP_LCD_V_RES)
#define BSP_LCD_DRAW_BUFF_DOUBLE   (0)

/**
 * @brief BSP display configuration structure
 */
typedef struct {
    lvgl_port_cfg_t lvgl_port_cfg;  /*!< LVGL port configuration */
    uint32_t        buffer_size;    /*!< Size of the buffer for the screen in pixels */
    bool            double_buffer;  /*!< True, if should be allocated two buffers */
    struct {
        unsigned int buff_dma: 1;    /*!< Allocated LVGL buffer will be DMA capable */
        unsigned int buff_spiram: 1; /*!< Allocated LVGL buffer will be in PSRAM */
    } flags;
} bsp_display_cfg_t;

/**
 * @brief Initialize display
 *
 * This function initializes SPI, display controller and starts LVGL handling task.
 *
 * @return Pointer to LVGL display or NULL when error occurred
 */
lv_disp_t *bsp_display_start(void);

/**
 * @brief Delete display
 * 
 * @return esp_err_t 
 */
esp_err_t bsp_display_del(void);

/**
 * @brief Initialize display brightness
 * 
 * @return esp_err_t 
 */
esp_err_t bsp_display_brightness_init(void);

/**
 * @brief Set display brightness
 * 
 * @param brightness Brightness value
 * @return esp_err_t 
 */
esp_err_t bsp_display_brightness_set(int brightness_percent);

/**
 * @brief Initialize display
 *
 * This function initializes SPI, display controller and starts LVGL handling task.
 * LCD backlight must be enabled separately by calling bsp_display_brightness_set()
 *
 * @param cfg display configuration
 *
 * @return Pointer to LVGL display or NULL when error occurred
 */
lv_disp_t *bsp_display_start_with_config(const bsp_display_cfg_t *cfg);

/**
 * @brief Get pointer to input device (touch, buttons, ...)
 *
 * @note The LVGL input device is initialized in bsp_display_start() function.
 *
 * @return Pointer to LVGL input device or NULL when not initialized
 */
lv_indev_t *bsp_display_get_input_dev(void);

/**
 * @brief Rotate screen
 *
 * Display must be already initialized by calling bsp_display_start()
 *
 * @param[in] disp Pointer to LVGL display
 * @param[in] rotation Angle of the display rotation
 */
esp_err_t bsp_display_rotate(lv_disp_rot_t rotation);

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

/**************************************************************************************************
 *
 * LEDs
 *
 **************************************************************************************************/

/**
 * @brief Initialize flashlight
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 */
esp_err_t bsp_flashlight_init(void);

/**
 * @brief Turn flashlight on/off
 *
 * @param on Switch flashlight on/off
 * @return
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 */
esp_err_t bsp_flashlight_set(const bool on);

/**
 * @brief Get flashlight status
 *
 * @return flashlight status
 */
bool bsp_get_flashlight_status(void);

/**************************************************************************************************
 *
 * I2S audio interface
 *
 * There is only one device connected to the I2S peripheral
 *  - MEMSensing Microsystems MSM261S4030H0: 48kHz, 24bit mono digital microphone
 *
 * For microphone initialization use bsp_audio_codec_microphone_init() which is inside initialize I2S with bsp_audio_init().
 * After microphone initialization, use functions from esp_codec_dev for record audio.
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
 * @note There is no deinit audio function. Users can free audio resources by calling i2s_del_channel()
 * @warning The type of i2s_config param is depending on IDF version.
 * @param[in]  i2s_config I2S configuration. Pass NULL to use default values (Mono, duplex, 16bit, 22050 Hz)
 * @param[out] tx_channel I2S TX channel
 * @param[out] rx_channel I2S RX channel
 * @return
 *      - ESP_OK                On success
 *      - ESP_ERR_NOT_SUPPORTED The communication mode is not supported on the current chip
 *      - ESP_ERR_INVALID_ARG   NULL pointer or invalid configuration
 *      - ESP_ERR_NOT_FOUND     No available I2S channel found
 *      - ESP_ERR_NO_MEM        No memory for storing the channel information
 *      - ESP_ERR_INVALID_STATE This channel has not initialized or already started
 */
esp_err_t bsp_extra_pdm_i2s_read(void *audio_buffer, size_t len, size_t *bytes_read, uint32_t timeout_ms);

/**
 * @brief Initialize PDM audio
 * 
 * @param i2s_config I2S configuration
 * @return esp_err_t 
 */
esp_err_t bsp_pdm_audio_init(const i2s_pdm_rx_config_t *i2s_config);

/**
 * @brief Initialize PDM microphone
 * 
 * @return esp_err_t 
 */
esp_codec_dev_handle_t bsp_audio_pdm_microphone_init(void);

/**
 * @brief Initialize PDM codec
 * 
 * @return esp_err_t 
 */
esp_err_t bsp_extra_pdm_codec_init();

#ifdef __cplusplus
}
#endif
