/**
 * @file main.c
 * @brief Music player example codes
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

#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "esp_heap_caps.h"
#include "esp_log.h"

#include "bsp_ext_io.h"
#include "bsp_i2c.h"
#include "bsp_sdcard.h"
#include "cat5171.h"
#include "ft5x06.h"
#include "lvgl_port.h"
#include "tca9554.h"
#include "tpl0401.h"

#include "app_music.h"
#include "ui_music.h"

void app_main(void)
{
    ESP_ERROR_CHECK(bsp_i2c_init(I2C_NUM_0, 400000));

    /* Initialize LCD */
    ESP_ERROR_CHECK(bsp_lcd_init());

    /* LCD touch IC init */
    ESP_ERROR_CHECK(ft5x06_init());

    /* Volume digital potentiometer */
    ESP_ERROR_CHECK(tpl0401_init());
    ESP_ERROR_CHECK(tpl0401_set_resistance(80));

    /* Initialize LVGL */
    ESP_ERROR_CHECK(lvgl_init(LVGL_SCR_SIZE / 8, LV_BUF_ALLOC_INTERNAL));

    /* Init SD card */
    ESP_ERROR_CHECK(bsp_sdcard_init());

    /* Start music task */
    app_music_start();
}
