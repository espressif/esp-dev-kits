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

#define TAG     ("app_main")

static ext_io_t io_config = BSP_EXT_IO_DEFAULT_CONFIG();
static ext_io_t io_level = BSP_EXT_IO_DEFAULT_LEVEL();

void app_main(void)
{
    ESP_ERROR_CHECK(bsp_i2c_init(I2C_NUM_0, 400000));
    ESP_ERROR_CHECK(tca9554_init());
    ESP_ERROR_CHECK(tca9554_set_configuration(io_config.val));
    ESP_ERROR_CHECK(tca9554_write_output_pins(io_level.val));

    /* LCD backlight brightness digital potentiometer */
    ESP_ERROR_CHECK(cat5171_init());
    ESP_ERROR_CHECK(cat5171_set_resistance(250));

    /* Volume digital potentiometer */
    ESP_ERROR_CHECK(tpl0401_init());
    ESP_ERROR_CHECK(tpl0401_set_resistance(80));

    /* LCD touch IC init */
    ESP_ERROR_CHECK(ft5x06_init());

    /* Initialize LCD */
    ESP_ERROR_CHECK(bsp_lcd_init());

    /* Initialize LVGL */
    ESP_ERROR_CHECK(lvgl_init(LVGL_SCR_SIZE / 8, LV_BUF_ALLOC_INTERNAL));

    /* Init SD card */
    ESP_ERROR_CHECK(bsp_sdcard_init());
    
    /*!< Init music player UI */
    lv_port_sem_take();
    ui_music();
    lv_port_sem_give();

    /* Start music task */
    app_music_start();
}
