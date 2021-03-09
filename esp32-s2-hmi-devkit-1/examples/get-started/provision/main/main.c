/**
 * @file main.c
 * @brief GUI provision example.
 * @version 0.1
 * @date 2021-03-07
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

#include "esp_heap_caps.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "nvs_flash.h"
#include "sdkconfig.h"

#include "bsp_ext_io.h"
#include "bsp_i2c.h"
#include "ft5x06.h"
#include "lvgl_port.h"
#include "tca9554.h"
#include "wifi_basic.h"

#include "app_prov.h"
#include "ui_prov.h"

static ext_io_t io_config = BSP_EXT_IO_DEFAULT_CONFIG();
static ext_io_t io_level = BSP_EXT_IO_DEFAULT_LEVEL();

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
    }

    ESP_ERROR_CHECK(bsp_i2c_init(I2C_NUM_0, 400000));
    ESP_ERROR_CHECK(tca9554_init());
    ESP_ERROR_CHECK(tca9554_set_configuration(io_config.val));
    ESP_ERROR_CHECK(tca9554_write_output_pins(io_level.val));

    /* LCD touch IC init */
    ESP_ERROR_CHECK(ft5x06_init());

    /* Initialize LCD */
    ESP_ERROR_CHECK(bsp_lcd_init());

    /* Initialize LVGL */
    ESP_ERROR_CHECK(lvgl_init(800 * 480 / 8, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));

    /*!< Initialize provision UI */
    lv_port_sem_take();
    ui_prov_init();
    lv_port_sem_give();

    /*!< Start provisioning app */
    ESP_ERROR_CHECK(app_prov_start());
}
