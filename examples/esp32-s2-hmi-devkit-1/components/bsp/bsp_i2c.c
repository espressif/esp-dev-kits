/**
 * @file bsp_i2c.c
 * @brief I2C config on dev board.
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

#include "bsp_i2c.h"

static const char *TAG= "bsp_i2c";

static i2c_bus_handle_t i2c_bus_handle = NULL;

static i2c_config_t conf = {
    .mode = I2C_MODE_MASTER,
    .scl_io_num = GPIO_I2C_SCL,
    .sda_io_num = GPIO_I2C_SDA,
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
};

esp_err_t bsp_i2c_init(i2c_port_t i2c_num, uint32_t clk_speed)
{
    /* Check if bus is already created */
    if (NULL != i2c_bus_handle) {
        ESP_LOGE(TAG, "I2C bus already initialized.");
        return ESP_FAIL;
    }

    conf.master.clk_speed = clk_speed;

    i2c_bus_handle = i2c_bus_create(i2c_num, &conf);
    
    if (NULL == i2c_bus_handle) {
        ESP_LOGE(TAG, "Failed create I2C bus");
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t bsp_i2c_add_device(i2c_bus_device_handle_t *i2c_device_handle, uint8_t dev_addr)
{
    if (NULL == i2c_bus_handle) {
        ESP_LOGE(TAG, "Failed create I2C device");
        return ESP_FAIL;
    }
    
    *i2c_device_handle = i2c_bus_device_create(i2c_bus_handle, dev_addr, 400000);

    if (NULL == i2c_device_handle) {
        ESP_LOGE(TAG, "Failed create I2C device");
        return ESP_FAIL;
    }

    return ESP_OK;
}
