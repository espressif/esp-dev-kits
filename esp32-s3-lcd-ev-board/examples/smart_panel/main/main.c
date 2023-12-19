/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_check.h"
#include "nvs_flash.h"

#include "ui_main.h"

#include "bsp_board_extra.h"
#include "bsp/esp-bsp.h"
#include "settings.h"

#include "app_wifi.h"
#include "app_weather.h"

#define LOG_MEM_INFO    0

static char *TAG = "app_main";

void app_main(void)
{
    ESP_LOGI(TAG, "system start");

    // Initialize NVS.
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // 1.OTA app partition table has a smaller NVS partition size than the non-OTA
        // partition table. This size mismatch may cause NVS initialization to fail.
        // 2.NVS partition contains data in new format and cannot be recognized by this version of code.
        // If this happens, we erase NVS partition and initialize NVS again.
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    ESP_ERROR_CHECK(settings_read_parameter_from_nvs());

    bsp_spiffs_mount();

    bsp_i2c_init();
    bsp_extra_led_init();
    bsp_extra_codec_init();
    bsp_extra_player_init(BSP_SPIFFS_MOUNT_POINT"/Music");

    bsp_display_start();

    app_weather_start();
    app_network_start();

    ESP_LOGI(TAG, "Display LVGL demo");
    ui_main();

    sys_param_t *sys_set = settings_get_parameter();
    while (true == sys_set->need_hint) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    if (sys_set->sr_enable) {
        ESP_LOGI(TAG, "speech recognition Enable");
        app_sr_start(false);
        bsp_audio_poweramp_enable(true);
    } else {
        ESP_LOGI(TAG, "speech recognition Disable");
    }

#if LOG_MEM_INFO
    static char buffer[128];    /* Make sure buffer is enough for `sprintf` */
    while (1) {
        /**
         * It's not recommended to frequently use functions like `heap_caps_get_free_size()` to obtain memory information
         * in practical applications, especially when the application extensively uses `malloc()` to dynamically allocate
         * a significant number of memory blocks. The frequent interrupt disabling may potentially lead to issues with other functionalities.
         */
        sprintf(buffer, "   Biggest /     Free /    Total\n"
                "\t DRAM : [%8d / %8d / %8d]\n"
                "\tPSRAM : [%8d / %8d / %8d]",
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
