/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_io_expander_tca9554.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_lcd_touch_ft5x06.h"
#include "esp_log.h"
#include "esp_rom_sys.h"

#include "sdkconfig.h"
#include "bsp_err_check.h"
#include "bsp_sub_board.h"
#include "bsp/esp32_s3_lcd_ev_board.h"

#define Delay(t)        vTaskDelay(pdMS_TO_TICKS(t))
#define udelay(t)       esp_rom_delay_us(t)
#define CS(io, n)       BSP_ERROR_CHECK_RETURN_ERR(esp_io_expander_set_level(io, BSP_LCD_SPI_CS, n))
#define SCK(io, n)      BSP_ERROR_CHECK_RETURN_ERR(esp_io_expander_set_level(io, BSP_LCD_SPI_SCK, n))
#define SDO(io, n)      BSP_ERROR_CHECK_RETURN_ERR(esp_io_expander_set_level(io, BSP_LCD_SPI_SDO, n))

static const char *TAG = "SUB-BOARD_480x480";

static bsp_lcd_trans_done_cb_t trans_done = NULL;
#if CONFIG_BSP_LCD_RGB_REFRESH_MANUALLY
static TaskHandle_t lcd_task_handle = NULL;
#endif

static esp_err_t lcd_config(esp_io_expander_handle_t io_expander);

/**************************************************************************************************
 *
 * LCD Panel Function
 *
 **************************************************************************************************/
IRAM_ATTR static bool on_vsync_event(esp_lcd_panel_handle_t panel, const esp_lcd_rgb_panel_event_data_t *edata, void *user_ctx)
{
    BaseType_t need_yield = pdFALSE;
#if CONFIG_BSP_LCD_RGB_REFRESH_MANUALLY
    xTaskNotifyFromISR(lcd_task_handle, ULONG_MAX, eNoAction, &need_yield);
#endif
    if (trans_done) {
        if (trans_done(panel)) {
            need_yield = pdTRUE;
        }
    }

    return (need_yield == pdTRUE);
}

#if CONFIG_BSP_LCD_RGB_REFRESH_MANUALLY
static void lcd_task(void *arg)
{
    ESP_LOGI(TAG, "Starting LCD refresh task");

    TickType_t tick;
    for (;;) {
        esp_lcd_rgb_panel_refresh((esp_lcd_panel_handle_t)arg);
        tick = xTaskGetTickCount();
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        vTaskDelayUntil(&tick, pdMS_TO_TICKS(CONFIG_BSP_LCD_RGB_REFRESH_TASK_PERIOD));
    }
}
#endif

esp_lcd_panel_handle_t bsp_lcd_init(void *arg)
{
    BSP_ERROR_CHECK_RETURN_ERR(lcd_config((esp_io_expander_handle_t)arg));

    ESP_LOGI(TAG, "Initialize RGB panel");
    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_rgb_panel_config_t panel_conf = {
        .clk_src = LCD_CLK_SRC_PLL160M,
        .psram_trans_align = 64,
        .data_width = 16,
        .de_gpio_num = BSP_LCD_DE,
        .pclk_gpio_num = BSP_LCD_PCLK,
        .vsync_gpio_num = BSP_LCD_VSYNC,
        .hsync_gpio_num = BSP_LCD_HSYNC,
        .data_gpio_nums = {
            BSP_LCD_DATA0,
            BSP_LCD_DATA1,
            BSP_LCD_DATA2,
            BSP_LCD_DATA3,
            BSP_LCD_DATA4,
            BSP_LCD_DATA5,
            BSP_LCD_DATA6,
            BSP_LCD_DATA7,
            BSP_LCD_DATA8,
            BSP_LCD_DATA9,
            BSP_LCD_DATA10,
            BSP_LCD_DATA11,
            BSP_LCD_DATA12,
            BSP_LCD_DATA13,
            BSP_LCD_DATA14,
            BSP_LCD_DATA15,
        },
        .timings = {
            .pclk_hz = BSP_LCD_PIXEL_CLOCK_HZ,
            .h_res = BSP_LCD_H_RES,
            .v_res = BSP_LCD_V_RES,
            .hsync_back_porch = BSP_LCD_HSYNC_BACK_PORCH,
            .hsync_front_porch = BSP_LCD_HSYNC_FRONT_PORCH,
            .hsync_pulse_width = BSP_LCD_HSYNC_PULSE_WIDTH,
            .vsync_back_porch = BSP_LCD_VSYNC_BACK_PORCH,
            .vsync_front_porch = BSP_LCD_VSYNC_FRONT_PORCH,
            .vsync_pulse_width = BSP_LCD_VSYNC_PULSE_WIDTH,
            .flags.pclk_active_neg = BSP_LCD_PCLK_ACTIVE_NEG,
        },
        .flags.fb_in_psram = 1,
#if CONFIG_BSP_LCD_RGB_REFRESH_MANUALLY
        .flags.refresh_on_demand = 1,
#endif
#if CONFIG_BSP_LCD_RGB_BUFFER_NUMS == 2
        .flags.double_fb = 1,
#elif CONFIG_BSP_LCD_RGB_BUFFER_NUMS == 3
        .num_fbs = 3,
#endif
#if CONFIG_BSP_LCD_RGB_BOUNCE_BUFFER_MODE
        .bounce_buffer_size_px = BSP_LCD_H_RES * CONFIG_BSP_LCD_RGB_BOUNCE_BUFFER_HEIGHT,
#endif
    };
    BSP_ERROR_CHECK_RETURN_NULL(esp_lcd_new_rgb_panel(&panel_conf, &panel_handle));
    esp_lcd_rgb_panel_event_callbacks_t cbs = {
        .on_vsync = on_vsync_event,
    };
    esp_lcd_rgb_panel_register_event_callbacks(panel_handle, &cbs, NULL);
    esp_lcd_panel_reset(panel_handle);
    esp_lcd_panel_init(panel_handle);
    esp_lcd_panel_disp_on_off(panel_handle, true);

#if CONFIG_BSP_LCD_RGB_REFRESH_MANUALLY
    xTaskCreate(lcd_task, "LCD task", 2048, panel_handle, CONFIG_BSP_LCD_RGB_REFRESH_TASK_PRIORITY, &lcd_task_handle);
#endif

    return panel_handle;
}

esp_err_t bsp_lcd_register_trans_done_callback(bsp_lcd_trans_done_cb_t callback)
{
#if CONFIG_LCD_RGB_ISR_IRAM_SAFE
    if (callback) {
        ESP_RETURN_ON_FALSE(esp_ptr_in_iram(callback), ESP_ERR_INVALID_ARG, TAG, "Callback not in IRAM");
    }
#endif
    trans_done = callback;

    return ESP_OK;
}

/**************************************************************************************************
 *
 * Touch Panel Function
 *
 **************************************************************************************************/
esp_lcd_touch_handle_t bsp_touch_panel_init(void)
{
    esp_lcd_panel_io_handle_t tp_io_handle = NULL;
    esp_lcd_touch_handle_t tp_handle = NULL;
    const esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_FT5x06_CONFIG();
    const esp_lcd_touch_config_t tp_cfg = {
        .x_max = BSP_LCD_H_RES,
        .y_max = BSP_LCD_V_RES,
        .rst_gpio_num = GPIO_NUM_NC,
        .int_gpio_num = GPIO_NUM_NC,
        .levels = {
            .reset = 0,
            .interrupt = 0,
        },
        .flags = {
            .swap_xy = 0,
            .mirror_x = 0,
            .mirror_y = 0,
        },
    };

    BSP_ERROR_CHECK_RETURN_ERR(esp_lcd_new_panel_io_i2c((esp_lcd_i2c_bus_handle_t)BSP_I2C_NUM, &tp_io_config, &tp_io_handle));
    BSP_ERROR_CHECK_RETURN_ERR(esp_lcd_touch_new_i2c_ft5x06(tp_io_handle, &tp_cfg, &tp_handle));

    return tp_handle;
}

/**************************************************************************************************
 *
 * LCD Configuration Function
 *
 **************************************************************************************************/
/**
 * @brief Simulate SPI to write data using io expander
 *
 * @param io; IO expander handle
 * @param data: Data to write
 *
 * @return
 *      - ESP_OK: Success, otherwise returns ESP_ERR_xxx
 */
static esp_err_t spi_write(esp_io_expander_handle_t io, uint16_t data)
{
    for (uint8_t n = 0; n < 9; n++) {
        if (data & 0x0100) {
            SDO(io, 1);
        } else {
            SDO(io, 0);
        }
        data = data << 1;

        SCK(io, 0);
        udelay(10);
        SCK(io, 1);
        udelay(10);
    }

    return ESP_OK;
}

/**
 * @brief Simulate SPI to write LCD command using io expander
 *
 * @param io; IO expander handle
 * @param data: LCD command to write
 *
 * @return
 *      - ESP_OK: Success, otherwise returns ESP_ERR_xxx
 */
static esp_err_t spi_write_cmd(esp_io_expander_handle_t io, uint16_t data)
{
    CS(io, 0);
    udelay(10);

    spi_write(io, (data & 0x00FF));

    udelay(10);
    CS(io, 1);
    SCK(io, 0);
    SDO(io, 0);
    udelay(10);

    return ESP_OK;
}

/**
 * @brief Simulate SPI to write LCD data using io expander
 *
 * @param io; IO expander handle
 * @param data: LCD data to write
 *
 * @return
 *      - ESP_OK: Success, otherwise returns ESP_ERR_xxx
 */
static esp_err_t spi_write_data(esp_io_expander_handle_t io, uint16_t data)
{
    CS(io, 0);
    udelay(10);

    data &= 0x00FF;
    data |= 0x0100;
    spi_write(io, data);

    udelay(10);
    CS(io, 1);
    SCK(io, 0);
    SDO(io, 0);
    udelay(10);

    return ESP_OK;
}

/**
 * @brief LCD configuration data structure type
 *
 */
typedef struct {
    uint8_t cmd;            // LCD command
    uint8_t data[52];       // LCD data
    uint8_t data_bytes;     // Length of data in above data array; 0xFF = end of cmds.
} lcd_config_data_t;

// *INDENT-OFF*
const static lcd_config_data_t LCD_CONFIG_CMD[] = {
    {0xf0, {0x55, 0xaa, 0x52, 0x08, 0x00}, 5},
    {0xf6, {0x5a, 0x87}, 2},
    {0xc1, {0x3f}, 1},
    {0xc2, {0x0e}, 1},
    {0xc6, {0xf8}, 1},
    {0xc9, {0x10}, 1},
    {0xcd, {0x25}, 1},
    {0xf8, {0x8a}, 1},
    {0xac, {0x45}, 1},
    {0xa0, {0xdd}, 1},
    {0xa7, {0x47}, 1},
    {0xfa, {0x00, 0x00, 0x00, 0x04}, 4},
    {0x86, {0x99, 0xa3, 0xa3, 0x51}, 4},
    {0xa3, {0xee}, 1},
    {0xfd, {0x3c, 0x3c, 0x00}, 3},
    {0x71, {0x48}, 1},
    {0x72, {0x48}, 1},
    {0x73, {0x00, 0x44}, 2},
    {0x97, {0xee}, 1},
    {0x83, {0x93}, 1},
    {0x9a, {0x72}, 1},
    {0x9b, {0x5a}, 1},
    {0x82, {0x2c, 0x2c}, 2},
    {0xb1, {0x10}, 1},
    {0x6d, {0x00, 0x1f, 0x19, 0x1a, 0x10, 0x0e, 0x0c, 0x0a, 0x02, 0x07, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
            0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x08, 0x01, 0x09, 0x0b, 0x0d, 0x0f, 0x1a, 0x19, 0x1f, 0x00}, 32},
    {0x64, {0x38, 0x05, 0x01, 0xdb, 0x03, 0x03, 0x38, 0x04, 0x01, 0xdc, 0x03, 0x03, 0x7a, 0x7a, 0x7a, 0x7a}, 16},
    {0x65, {0x38, 0x03, 0x01, 0xdd, 0x03, 0x03, 0x38, 0x02, 0x01, 0xde, 0x03, 0x03, 0x7a, 0x7a, 0x7a, 0x7a}, 16},
    {0x66, {0x38, 0x01, 0x01, 0xdf, 0x03, 0x03, 0x38, 0x00, 0x01, 0xe0, 0x03, 0x03, 0x7a, 0x7a, 0x7a, 0x7a}, 16},
    {0x67, {0x30, 0x01, 0x01, 0xe1, 0x03, 0x03, 0x30, 0x02, 0x01, 0xe2, 0x03, 0x03, 0x7a, 0x7a, 0x7a, 0x7a}, 16},
    {0x68, {0x00, 0x08, 0x15, 0x08, 0x15, 0x7a, 0x7a, 0x08, 0x15, 0x08, 0x15, 0x7a, 0x7a}, 13},
    {0x60, {0x38, 0x08, 0x7a, 0x7a, 0x38, 0x09, 0x7a, 0x7a}, 8},
    {0x63, {0x31, 0xe4, 0x7a, 0x7a, 0x31, 0xe5, 0x7a, 0x7a}, 8},
    {0x69, {0x04, 0x22, 0x14, 0x22, 0x14, 0x22, 0x08}, 7},
    {0x6b, {0x07}, 1},
    {0x7a, {0x08, 0x13}, 2},
    {0x7b, {0x08, 0x13}, 2},
    {0xd1, {0x00, 0x00, 0x00, 0x04, 0x00, 0x12, 0x00, 0x18, 0x00, 0x21, 0x00, 0x2a, 0x00, 0x35, 0x00, 0x47, 0x00,
            0x56, 0x00, 0x90, 0x00, 0xe5, 0x01, 0x68, 0x01, 0xd5, 0x01, 0xd7, 0x02, 0x36, 0x02, 0xa6, 0x02, 0xee,
            0x03, 0x48, 0x03, 0xa0, 0x03, 0xba, 0x03, 0xc5, 0x03, 0xd0, 0x03, 0xe0, 0x03, 0xea, 0x03, 0xfa, 0x03,
            0xff}, 52},
    {0xd2, {0x00, 0x00, 0x00, 0x04, 0x00, 0x12, 0x00, 0x18, 0x00, 0x21, 0x00, 0x2a, 0x00, 0x35, 0x00, 0x47, 0x00,
            0x56, 0x00, 0x90, 0x00, 0xe5, 0x01, 0x68, 0x01, 0xd5, 0x01, 0xd7, 0x02, 0x36, 0x02, 0xa6, 0x02, 0xee,
            0x03, 0x48, 0x03, 0xa0, 0x03, 0xba, 0x03, 0xc5, 0x03, 0xd0, 0x03, 0xe0, 0x03, 0xea, 0x03, 0xfa, 0x03,
            0xff}, 52},
    {0xd3, {0x00, 0x00, 0x00, 0x04, 0x00, 0x12, 0x00, 0x18, 0x00, 0x21, 0x00, 0x2a, 0x00, 0x35, 0x00, 0x47, 0x00,
            0x56, 0x00, 0x90, 0x00, 0xe5, 0x01, 0x68, 0x01, 0xd5, 0x01, 0xd7, 0x02, 0x36, 0x02, 0xa6, 0x02, 0xee,
            0x03, 0x48, 0x03, 0xa0, 0x03, 0xba, 0x03, 0xc5, 0x03, 0xd0, 0x03, 0xe0, 0x03, 0xea, 0x03, 0xfa, 0x03,
            0xff}, 52},
    {0xd4, {0x00, 0x00, 0x00, 0x04, 0x00, 0x12, 0x00, 0x18, 0x00, 0x21, 0x00, 0x2a, 0x00, 0x35, 0x00, 0x47, 0x00,
            0x56, 0x00, 0x90, 0x00, 0xe5, 0x01, 0x68, 0x01, 0xd5, 0x01, 0xd7, 0x02, 0x36, 0x02, 0xa6, 0x02, 0xee,
            0x03, 0x48, 0x03, 0xa0, 0x03, 0xba, 0x03, 0xc5, 0x03, 0xd0, 0x03, 0xe0, 0x03, 0xea, 0x03, 0xfa, 0x03,
            0xff}, 52},
    {0xd5, {0x00, 0x00, 0x00, 0x04, 0x00, 0x12, 0x00, 0x18, 0x00, 0x21, 0x00, 0x2a, 0x00, 0x35, 0x00, 0x47, 0x00,
            0x56, 0x00, 0x90, 0x00, 0xe5, 0x01, 0x68, 0x01, 0xd5, 0x01, 0xd7, 0x02, 0x36, 0x02, 0xa6, 0x02, 0xee,
            0x03, 0x48, 0x03, 0xa0, 0x03, 0xba, 0x03, 0xc5, 0x03, 0xd0, 0x03, 0xe0, 0x03, 0xea, 0x03, 0xfa, 0x03,
            0xff}, 52},
    {0xd6, {0x00, 0x00, 0x00, 0x04, 0x00, 0x12, 0x00, 0x18, 0x00, 0x21, 0x00, 0x2a, 0x00, 0x35, 0x00, 0x47, 0x00,
            0x56, 0x00, 0x90, 0x00, 0xe5, 0x01, 0x68, 0x01, 0xd5, 0x01, 0xd7, 0x02, 0x36, 0x02, 0xa6, 0x02, 0xee,
            0x03, 0x48, 0x03, 0xa0, 0x03, 0xba, 0x03, 0xc5, 0x03, 0xd0, 0x03, 0xe0, 0x03, 0xea, 0x03, 0xfa, 0x03,
            0xff}, 52},
    {0x3a, {0x66}, 1},
    {0x11, {0x00}, 0},
    {0x00, {0x00}, 0xff},
};
// *INDENT-OFF*

/**
 * @brief Configure LCD with specific commands and data
 *
 * @param[in] io_expander: IO expander handle
 *
 * @return
 *      - ESP_OK: Success, otherwise returns ESP_ERR_xxx
 *
 */
static esp_err_t lcd_config(esp_io_expander_handle_t io_expander)
{
    BSP_ERROR_CHECK_RETURN_ERR(esp_io_expander_set_dir(io_expander, BSP_LCD_SPI_CS, IO_EXPANDER_OUTPUT));
    esp_io_expander_set_dir(io_expander, BSP_LCD_SPI_SCK, IO_EXPANDER_OUTPUT);
    esp_io_expander_set_dir(io_expander, BSP_LCD_SPI_SDO, IO_EXPANDER_OUTPUT);
    esp_io_expander_set_level(io_expander, BSP_LCD_SPI_CS, 1);
    esp_io_expander_set_level(io_expander, BSP_LCD_SPI_SCK, 1);
    esp_io_expander_set_level(io_expander, BSP_LCD_SPI_SDO, 1);

    for (uint8_t i = 0; LCD_CONFIG_CMD[i].data_bytes != 0xff; i++) {
        BSP_ERROR_CHECK_RETURN_ERR(spi_write_cmd(io_expander, LCD_CONFIG_CMD[i].cmd));
        for (uint8_t j = 0; j < LCD_CONFIG_CMD[i].data_bytes; j++) {
            BSP_ERROR_CHECK_RETURN_ERR(spi_write_data(io_expander, LCD_CONFIG_CMD[i].data[j]));
        }
    }
    vTaskDelay(pdMS_TO_TICKS(120));
    BSP_ERROR_CHECK_RETURN_ERR(spi_write_cmd(io_expander, 0x29));
    vTaskDelay(pdMS_TO_TICKS(20));

    return ESP_OK;
}
