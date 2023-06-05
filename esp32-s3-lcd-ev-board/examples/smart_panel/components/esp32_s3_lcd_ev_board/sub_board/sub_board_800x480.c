/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_lcd_touch_gt1151.h"
#include "esp_log.h"

#include "sdkconfig.h"
#include "bsp_err_check.h"
#include "bsp_sub_board.h"
#include "bsp/esp32_s3_lcd_ev_board.h"

static const char *TAG = "SUB-BOARD_800x480";

static bsp_lcd_trans_done_cb_t trans_done = NULL;
#if CONFIG_BSP_LCD_RGB_REFRESH_MANUALLY
static TaskHandle_t lcd_task_handle = NULL;
#endif

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
    (void)arg;
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
    const esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_GT1151_CONFIG();
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
    BSP_ERROR_CHECK_RETURN_ERR(esp_lcd_touch_new_i2c_gt1151(tp_io_handle, &tp_cfg, &tp_handle));

    return tp_handle;
}
