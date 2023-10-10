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
#define LOG_MEM_INFO    (0)

void app_main(void)
{
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
    /**
     * To avoid errors caused by multiple tasks simultaneously accessing LVGL,
     * should acquire a lock before operating on LVGL.
     */
    bsp_display_lock(0);

    /* Here're four internal demos of LVGL. They can run with all subboards */
    lv_demo_music();        /* A modern, smartphone-like music player demo */
    // lv_demo_widgets();      /* A widgets example */
    // lv_demo_stress();       /* A stress test for LVGL */
    // lv_demo_benchmark();    /* A demo to measure the performance of LVGL or to compare different settings */

    /* Here're two UI demos created by Squareline Studio. They can only run with subboard3(800x480) */
    // ui_printer_init();         /* A demo to show virtual printer */
#ifndef CONFIG_BSP_DISPLAY_LVGL_AVOID_TEAR
    // bsp_display_rotate(disp, LV_DISP_ROT_90);   /* Rotate screen from 800*480 to 480*800, it can't work with anti-tearing function */
    // ui_tuner_init();                            /* A demo to show virtual tuner */
#endif

    /* Release the lock */
    bsp_display_unlock();

#if LOG_MEM_INFO
    static char buffer[128];    /* Make sure buffer is enough for `sprintf` */
    while (1) {
        /**
         * It's not recommended to frequently use functions like `heap_caps_get_free_size()` to obtain memory information
         * in practical applications, especially when the application extensively uses `malloc()` to dynamically allocate
         * a significant number of memory blocks. The frequent interrupt disabling may potentially lead to issues with other functionalities.
         */
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

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
#endif
}
