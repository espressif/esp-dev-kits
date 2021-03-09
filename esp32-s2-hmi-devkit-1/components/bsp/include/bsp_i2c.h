/**
 * @file bsp_i2c.h
 * @brief Basic I2C operator on specified board
 * @version 0.1
 * @date 2021-01-13
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
#pragma once

#include <stdio.h>

#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"

#include "bsp_board.h"
#include "i2c_bus.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize I2C bus with GPIOs defined in 'bsp_board_xxx.h'
 * 
 * @param i2c_num I2C port number
 * @param clk_speed I2C bus clock speed
 * @return esp_err_t I2C bus initialize state
 *     - ESP_OK   Success
 *     - ESP_FAIL I2C bus initialize error
 */
esp_err_t bsp_i2c_init(i2c_port_t i2c_num, uint32_t clk_speed);

/**
 * @brief Add device on I2C bus initialized by `bsp_i2c_init`
 * 
 * @param i2c_device_handle Pointer to device handle, NULL if failed to add.
 * @param dev_addr Device address(7 Bit)
 * @return esp_err_t I2C device add state
 *     - ESP_OK   Success
 *     - ESP_FAIL I2C device add error
 */
esp_err_t bsp_i2c_add_device(i2c_bus_device_handle_t *i2c_device_handle, uint8_t dev_addr);

#ifdef __cplusplus
}
#endif
