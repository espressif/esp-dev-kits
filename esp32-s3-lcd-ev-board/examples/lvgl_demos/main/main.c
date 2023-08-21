/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "lv_demos.h"
#include "bsp/esp-bsp.h"
#include "ui_printer.h"
#include "ui_tuner.h"

static char *TAG = "app_main";

/* Print log about SRAM and PSRAM memory */
#define LOG_MEM_INFO    (1)

void app_main(void)
{
#if CONFIG_BSP_LCD_SUB_BOARD_480_480
    // For the newest version sub board, we need to set `BSP_LCD_VSYNC` to high before initialize LCD
    // It's a requirement of the LCD module and will be added into BSP in the future
    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = BIT64(BSP_LCD_VSYNC);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = true;
    gpio_config(&io_conf);
    gpio_set_level(BSP_LCD_VSYNC, 1);
#endif

    bsp_i2c_init();
    lv_disp_t *disp = bsp_display_start();

#if CONFIG_BSP_DISPLAY_LVGL_AVOID_TEAR
    ESP_LOGI(TAG, "Avoid lcd tearing effect");
#if CONFIG_BSP_DISPLAY_LVGL_FULL_REFRESH
    ESP_LOGI(TAG, "LVGL full-refresh");
#elif CONFIG_BSP_DISPLAY_LVGL_DIRECT_MODE
    ESP_LOGI(TAG, "LVGL direct-mode");
#endif
#endif

    ESP_LOGI(TAG, "Display LVGL demo");
    bsp_display_lock(0);

    /* Here're four internal demos of LVGL. They can run with all subboards */
    lv_demo_music();        /* A modern, smartphone-like music player demo */
    // lv_demo_widgets();      /* A widgets example */
    // lv_demo_stress();       /* A stress test for LVGL */
    // lv_demo_benchmark();    /* A demo to measure the performance of LVGL or to compare different settings */

    /* Here're two UI demos created by Squareline Studio. They can only run with subboard3(800x480) */
#if CONFIG_BSP_LCD_SUB_BOARD_800_480
    // ui_printer_init();         /* A demo to show virtual printer */
#ifndef CONFIG_BSP_DISPLAY_LVGL_AVOID_TEAR
    // bsp_display_rotate(disp, LV_DISP_ROT_90);   /* Rotate screen from 800*480 to 480*800, it can't work with anti-tearing function */
    // ui_tuner_init();                            /* A demo to show virtual tuner */
#endif
#endif

    bsp_display_unlock();

#if LOG_MEM_INFO
    static char buffer[128];    /* Make sure buffer is enough for `sprintf` */
    while (1) {
        sprintf(buffer, "   Biggest /     Free /    Total\n"
                "\t  SRAM : [%8d / %8d / %8d]\n"
                "\t PSRAM : [%8d / %8d / %8d]",
                heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL),
                heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
                heap_caps_get_total_size(MALLOC_CAP_INTERNAL),
                heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM),
                heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
                heap_caps_get_total_size(MALLOC_CAP_SPIRAM));
        ESP_LOGI("MEM", "%s", buffer);

        vTaskDelay(pdMS_TO_TICKS(500));
    }
#endif
}
