/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_check.h"
#include "nvs_flash.h"
#include "nvs.h"

#include "bsp_board_extra.h"
#include "bsp/esp-bsp.h"
#include "lv_example_pub.h"

static char *TAG = "app_main";

#define LOG_MEM_INFO    (0)

void app_main(void)
{
    ESP_LOGI(TAG, "Compile time: %s %s", __DATE__, __TIME__);
    /* Initialize NVS. */
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    bsp_spiffs_mount();

    bsp_i2c_init();
    bsp_extra_codec_init();
    bsp_extra_player_init(BSP_SPIFFS_MOUNT_POINT"/mp3");

    bsp_display_start();

    ESP_LOGI(TAG, "Display LVGL demo");
    bsp_display_lock(0);
    lv_style_pre_init();
    lv_create_home(&main_Layer);
    lv_create_clock(&clock_screen_layer, TIME_ENTER_CLOCK_2MIN);
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
