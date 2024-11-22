/**
 * @file main.c
 * @brief Example main src.
 * @version 0.1
 * @date 2021-03-04
 * 
 * @copyright Copyright 2021 Espressif Systems (Shanghai) Co. Ltd.
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *               http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#include "esp_heap_caps.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "bsp_lcd.h"
#include "lvgl_port.h"
#include "lv_port_fs.h"

#include "app_music.h"
#include "device.h"
#include "qcloud.h"
#include "ui_main.h"

#define LOG_MEM_INFO    1

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    /* Init I2C devices and turn on power */
    ESP_ERROR_CHECK(device_init());

    /* Init LCD */
    ESP_ERROR_CHECK(bsp_lcd_init());

    /* Init LVGL, allocating buffer and create tasks for tick and handler */
    ESP_ERROR_CHECK(lvgl_init(LVGL_SCR_SIZE / 16, LV_BUF_ALLOC_INTERNAL));

    /* Init LVGL file system API */
    lv_port_fs_init();

    /* Start main UI */
    ui_main();

    /* Start music task */
    ESP_ERROR_CHECK(app_music_start());

    /* Start network */
    qcloud_init();

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
