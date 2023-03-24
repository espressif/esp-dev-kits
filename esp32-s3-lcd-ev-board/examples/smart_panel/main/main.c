/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "esp_heap_caps.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lv_port.h"
#include "lv_port_fs.h"
#include "app_player.h"
#include "ui_main.h"
#include "bsp_board.h"

#define LOG_MEM_INFO    0

bool sdcard_ok = 0;

void app_main(void)
{
    ESP_LOGI("TAG", "system start");

    // Initialize NVS.
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    ESP_ERROR_CHECK(bsp_board_init());
    bsp_board_power_ctrl(POWER_MODULE_AUDIO, true);
    lv_port_init();

    /* Init LVGL file system API */
    lv_port_fs_init();
    /* Start main UI */
    ui_main();

#if LOG_MEM_INFO
    static char buffer[128];    /* Make sure buffer is enough for `sprintf` */
    while (1) {
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

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
#endif
}
