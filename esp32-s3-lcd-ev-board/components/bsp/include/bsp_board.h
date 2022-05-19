/*
 * SPDX-FileCopyrightText: 2015-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "driver/adc.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_types.h"
#include "../codec/audio_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
#define ets_delay_us(t) esp_rom_delay_us(t)
#define GPIO_PIN_INTR_DISABLE GPIO_INTR_DISABLE
#define ets_printf esp_rom_printf
#endif

typedef enum {
    BOARD_S3_BOX,
    BOARD_S3_BOX_LITE,
    BOARD_LCD_EVB,
} boards_id_t;

typedef enum {
    BOARD_BTN_ID_BOOT  = 0, // The index is the same as the sequence number of the board_button_t array
    BOARD_BTN_ID_PREV  = 1,
    BOARD_BTN_ID_ENTER = 2,
    BOARD_BTN_ID_NEXT  = 3,
} board_btn_id_t;

typedef struct {
    board_btn_id_t id;
    uint32_t vol;      // for adc button
    gpio_num_t io_num; // for gpio button, Set GPIO_NUM_NC to indicate this button is adc button
    bool active_level; // for gpio button
} board_button_t;

typedef struct {
    gpio_num_t row1[4]; //The first row
    gpio_num_t row2[4];
} pmod_pins_t;

typedef struct {
    /**
    * @brief Initialize Codec on dev board
    *
    * @param sample_rate
    * @return
    *    - ESP_OK: Success
    *    - ESP_ERR_NOT_FOUND: Codec not detected on I2C bus
    *    - ESP_ERR_NOT_SUPPORTED: Unsupported Codec or t
    *    - Others: Fail
    */
    esp_err_t (*codec_init)(audio_hal_iface_samples_t sample_rate);

    /**
     * @brief Set clock & bit width
     *
     * @param rate sample rate (ex: 8000, 44100...)
     * @param bits_cfg bits configuration
     * @param ch channel
     *
     * @return
     *     - ESP_OK              Success
     *     - ESP_ERR_INVALID_ARG Parameter error
     */
    esp_err_t (*codec_set_clk)(uint32_t rate, uint32_t bits_cfg, uint32_t ch);

    /**
     * @brief Write audio data
     *
     * @param src                 Source address to write from
     * @param size                Size of data in bytes
     * @param[out] bytes_written  Number of bytes written, if timeout, the result will be less than the size passed in.
     * @param ticks_to_wait       TX buffer wait timeout in RTOS ticks. If this
     * many ticks pass without space becoming available in the DMA
     * transmit buffer, then the function will return. Pass portMAX_DELAY for no timeout.
     *
     * @return
     *     - ESP_OK               Success
     *     - ESP_ERR_INVALID_ARG  Parameter error
     */
    esp_err_t (*codec_write)(void *dest, size_t size, size_t *bytes_read, TickType_t ticks_to_wait);

    /**
     * @brief Read audio data
     *
     * @param dest            Destination address to read into
     * @param size            Size of data in bytes
     * @param[out] bytes_read Number of bytes read, if timeout, bytes read will be less than the size passed in.
     * @param ticks_to_wait   RX buffer wait timeout in RTOS ticks. If this many ticks pass without bytes becoming available in the DMA receive buffer, then the function will return (note that if data is read from the DMA buffer in pieces, the overall operation may still take longer than this timeout.) Pass portMAX_DELAY for no timeout.
     *
     * @return
     *     - ESP_OK               Success
     *     - ESP_ERR_INVALID_ARG  Parameter error
     */
    esp_err_t (*codec_read)(void *dest, size_t size, size_t *bytes_read, TickType_t ticks_to_wait);

    /**
     * @brief Zero the contents of the TX DMA buffer.
     *
     */
    esp_err_t (*codec_zero_dma_buffer)(void);

    /**
     * @brief Set input microphone gain of Codec
     *
     * @param channel_mask mask of channel
     * @param volume volume to set (0 ~ 100)
     * @return
     *    - ESP_OK: Success
     *    - ESP_ERR_NOT_FOUND: Codec not detected on I2C bus
     */
    esp_err_t (*codec_set_adc_gain)(uint8_t channel_mask, uint8_t volume);

    /**
     * @brief Set output volume of Codec
     *
     * @param volume volume to set (0 ~ 100)
     * @return
     *    - ESP_OK: Success
     *    - ESP_ERR_NOT_FOUND: Codec not detected on I2C bus
     */
    esp_err_t (*codec_set_dac_gain)(uint8_t volume);
} board_codec_ops_t;

typedef struct {
    /**
    * @brief Init LCD
    *
    * @return
    *    - ESP_OK: Success
    *    - Others: Fail
    */
    esp_err_t (*lcd_init)(void);

    /**
     * @brief Deinit LCD
     *
     * @return
     *    - ESP_OK: Success
     *    - Others: Fail
     */
    esp_err_t (*lcd_deinit)(void);

    /**
     * @brief Flush LCD
     *
     * @param x1 Start index on x-axis (x1 included)
     * @param y1 Start index on y-axis (y1 included)
     * @param x2 End index on x-axis (x2 not included)
     * @param y2 End index on y-axis (y2 not included)
     * @param p_data RGB color data that will be dumped to the specific range
     * @return
     *    - ESP_OK: Success
     *    - Others: Fail
     */
    esp_err_t (*lcd_flush)(int x1, int y1, int x2, int y2, const void *p_data);

    /**
     * @brief Set callback function when a single flush transaction is finished
     *
     * @param trans_done_cb Callback function
     * @param data User data
     * @return
     *    - ESP_OK: Success
     *    - Others: Fail
     */
    esp_err_t (*lcd_set_cb)(bool (*trans_done_cb)(void *), void *data);

    /**
     * @brief Set backlight
     *
     * @param en 0: OFF, other: ON
     * @return
     *    - ESP_OK: Success
     *    - Others: Fail
     */
    esp_err_t (*lcd_set_backlight)(bool en);
} board_lcd_ops_t;

typedef struct {
    esp_err_t (*init)(uint8_t i2c_addr);
    esp_err_t (*set_direction)(uint8_t pin, bool is_output);
    esp_err_t (*set_level)(uint8_t pin, bool level);
    esp_err_t (*read_output_pins)(uint8_t *pin_val);
    esp_err_t (*read_input_pins)(uint8_t *pin_val);
    esp_err_t (*multi_write_start)(void);
    esp_err_t (*multi_write_new_level)(int pin, bool new_level);
    esp_err_t (*multi_write_end)(void);
} io_expander_ops_t;

typedef enum {
    LCD_IFACE_I2C,
    LCD_IFACE_SPI,
    LCD_IFACE_8080,
    LCD_IFACE_RGB,
} lcd_iface_type_t;

typedef struct {
    /**
     * @brief LCD GPIO defination and config
     *
     */
    bool FUNC_LCD_EN     ;
    int LCD_BUS_WIDTH   ;
    lcd_iface_type_t LCD_IFACE  ;
    const char *LCD_DISP_IC_STR  ; // lowercase letter
    int LCD_WIDTH       ;
    int LCD_HEIGHT      ;
    int LCD_FREQ        ;
    int LCD_CMD_BITS    ;
    int LCD_PARAM_BITS  ;
    spi_host_device_t LCD_HOST;

    bool LCD_SWAP_XY     ;
    bool LCD_MIRROR_X    ;
    bool LCD_MIRROR_Y    ;
    bool LCD_COLOR_INV   ;
    esp_lcd_color_space_t LCD_COLOR_SPACE;

    // SPI interface GPIOs for LCD panel
    int GPIO_LCD_BL     ;
    int GPIO_LCD_BL_ON  ;
    int GPIO_LCD_CS     ;
    int GPIO_LCD_RST    ;
    int GPIO_LCD_DC     ;
    int GPIO_LCD_WR     ; // signal of 8080 interface
    int GPIO_LCD_CLK    ;
    int GPIO_LCD_DIN    ;
    int GPIO_LCD_DOUT   ;

    // RGB interface GPIOs for LCD panel
    int GPIO_LCD_DISP_EN ;
    int GPIO_LCD_VSYNC   ;
    int GPIO_LCD_HSYNC   ;
    int GPIO_LCD_DE      ;
    int GPIO_LCD_PCLK    ;
    int GPIO_LCD_DATA0   ; // B0
    int GPIO_LCD_DATA1   ; // B1
    int GPIO_LCD_DATA2   ; // B2
    int GPIO_LCD_DATA3   ; // B3
    int GPIO_LCD_DATA4   ; // B4
    int GPIO_LCD_DATA5   ; // G0
    int GPIO_LCD_DATA6   ; // G1
    int GPIO_LCD_DATA7   ; // G2
    int GPIO_LCD_DATA8   ; // G3
    int GPIO_LCD_DATA9   ; // G4
    int GPIO_LCD_DATA10  ; // G5
    int GPIO_LCD_DATA11  ; // R0
    int GPIO_LCD_DATA12  ; // R1
    int GPIO_LCD_DATA13  ; // R2
    int GPIO_LCD_DATA14  ; // R3
    int GPIO_LCD_DATA15  ; // R4
    int HSYNC_BACK_PORCH ;
    int HSYNC_FRONT_PORCH ;
    int HSYNC_PULSE_WIDTH ;
    int VSYNC_BACK_PORCH ;
    int VSYNC_FRONT_PORCH ;
    int VSYNC_PULSE_WIDTH ;
    int PCLK_ACTIVE_NEG ;

    bool BSP_INDEV_IS_TP;
    bool TOUCH_PANEL_SWAP_XY;
    bool TOUCH_PANEL_INVERSE_X;
    bool TOUCH_PANEL_INVERSE_Y;
    int TOUCH_PANEL_I2C_ADDR;
    bool TOUCH_WITH_HOME_BUTTON;

    bool BSP_BUTTON_EN;
    adc1_channel_t BUTTON_ADC_CHAN; // only use for adc button
    const board_button_t *BUTTON_TAB;
    uint8_t BUTTON_TAB_LEN;

    bool FUNC_IO_EXPANDER_EN;
    uint8_t IO_EXPANDER_I2C_ADDR;

    /**
     * @brief ESP-Dev-Board I2C GPIO defineation
     *
     */
    bool FUNC_I2C_EN     ;
    int GPIO_I2C_SCL    ;
    int GPIO_I2C_SDA    ;

    /**
     * @brief ESP-Dev-Board SDMMC GPIO defination
     *
     * @note Only avaliable when PMOD connected
     */
    bool FUNC_SDMMC_EN   ;
    int SDMMC_BUS_WIDTH ;
    int GPIO_SDMMC_CLK  ;
    int GPIO_SDMMC_CMD  ;
    int GPIO_SDMMC_D0   ;
    int GPIO_SDMMC_D1   ;
    int GPIO_SDMMC_D2   ;
    int GPIO_SDMMC_D3   ;
    int GPIO_SDMMC_DET  ;

    /**
     * @brief ESP-Dev-Board SDSPI GPIO definationv
     *
     */
    bool FUNC_SDSPI_EN       ;
    int SDSPI_HOST          ;
    int GPIO_SDSPI_CS       ;
    int GPIO_SDSPI_SCLK     ;
    int GPIO_SDSPI_MISO     ;
    int GPIO_SDSPI_MOSI     ;

    /**
     * @brief ESP-Dev-Board SPI GPIO defination
     *
     */
    bool FUNC_SPI_EN         ;
    int GPIO_SPI_CS         ;
    int GPIO_SPI_MISO       ;
    int GPIO_SPI_MOSI       ;
    int GPIO_SPI_SCLK       ;

    /**
     * @brief ESP-Dev-Board RMT GPIO defination
     *
     */
    bool FUNC_RMT_EN        ;
    int GPIO_RMT_IR         ;
    int GPIO_RMT_LED        ;
    int RMT_LED_NUM         ;

    /**
     * @brief ESP-Dev-Board I2S GPIO defination
     *
     */
    bool FUNC_I2S_EN         ;
    int GPIO_I2S_LRCK       ;
    int GPIO_I2S_MCLK       ;
    int GPIO_I2S_SCLK       ;
    int GPIO_I2S_SDIN       ;
    int GPIO_I2S_DOUT       ;
    int CODEC_I2C_ADDR;
    int AUDIO_ADC_I2C_ADDR;

    int IMU_I2C_ADDR;

    /**
     * @brief ESP32-S3-HMI-DevKit power control IO
     *
     * @note Some power control pins might not be listed yet
     *
     */
    int FUNC_PWR_CTRL       ;
    int GPIO_PWR_CTRL       ;
    int GPIO_PWR_ON_LEVEL   ;

    int GPIO_MUTE_NUM   ;
    int GPIO_MUTE_LEVEL ;

    const pmod_pins_t *PMOD1;
    const pmod_pins_t *PMOD2;

    const board_codec_ops_t *codec_ops;
    const board_lcd_ops_t *lcd_ops;
    const io_expander_ops_t *io_expander_ops;

} board_res_desc_t;

/**
 * @brief Power module of dev board. This can be expanded in the future.
 *
 */
typedef enum {
    POWER_MODULE_LCD = 1,       /*!< LCD power control */
    POWER_MODULE_AUDIO,         /*!< Audio PA power control */
    POWER_MODULE_ALL = 0xff,    /*!< All module power control */
} power_module_t;

typedef struct {
    boards_id_t id;
    const char *name;

    /**
     * @brief Detect whether it is the current board
     *
     * @return
     *    - ESP_OK: Success
     *    - Others: Fail
     */
    esp_err_t (*board_detect)(void);

    /**
     * @brief Special config for dev board
     *
     * @return
     *    - ESP_OK: Success
     *    - Others: Fail
     */
    esp_err_t (*board_init)(void);

    /**
    * @brief Control power of dev board
    *
    * @param module Refer to `power_module_t`
    * @param on Turn on or off specified power module. On if true
    * @return
    *    - ESP_OK: Success
    *    - Others: Fail
    */
    esp_err_t (*board_power_ctrl)(power_module_t module, bool on);

    /**
     * @brief Get board description
     *
     * @return pointer of board_res_desc_t
     */
    const board_res_desc_t *(*board_get_res_desc)(void);

} boards_info_t;

/**
 * @brief Special config for dev board
 *
 * @return
 *    - ESP_OK: Success
 *    - Others: Fail
 */
esp_err_t bsp_board_init(void);

/**
 * @brief Get the board information
 *
 * @return pointer of boards_info_t
 */
const boards_info_t *bsp_board_get_info(void);

/**
 * @brief Get board description
 *
 * @return pointer of board_res_desc_t
 */
const board_res_desc_t *bsp_board_get_description(void);

/**
 * @brief Control power of dev board
 *
 * @param module Refer to `power_module_t`
 * @param on Turn on or off specified power module. On if true
 * @return
 *    - ESP_OK: Success
 *    - Others: Fail
 */
esp_err_t bsp_board_power_ctrl(power_module_t module, bool on);

#ifdef __cplusplus
}
#endif
