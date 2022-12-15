/**
 * @file tca9554.c
 * @brief TCA9554 driver.
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

#include "esp_log.h"
#include "bsp_i2c.h"
#include "esp_check.h"
#include "bsp_board.h"
#include "tca9554.h"

static const char *TAG = "tca9554";

#define TCA9554_INPUT_PORT_REG          (0x00)
#define TCA9554_OUTPUT_PORT_REG         (0x01)
#define TCA9554_POLARITY_INVERSION_REG  (0x02)
#define TCA9554_CONFIGURATION_REG       (0x03)

static i2c_bus_device_handle_t tca9554_handle = NULL;
static i2c_cmd_handle_t g_i2c_cmd;
static uint8_t output_reg = 0;
static uint8_t configuration_reg = 0;

static esp_err_t tca9554_read_byte(uint8_t reg_addr, uint8_t *data)
{
    return i2c_bus_read_byte(tca9554_handle, reg_addr, data);
}

static esp_err_t tca9554_write_byte(uint8_t reg_addr, uint8_t data)
{
    return i2c_bus_write_byte(tca9554_handle, reg_addr, data);
}

esp_err_t tca9554_init(uint8_t i2c_addr)
{
    esp_err_t ret = ESP_OK;
    ESP_RETURN_ON_FALSE(NULL == tca9554_handle, ESP_FAIL, TAG, "tca9554 already initialized");

    bsp_i2c_add_device(&tca9554_handle, i2c_addr);
    ESP_RETURN_ON_FALSE(NULL != tca9554_handle, ESP_FAIL, TAG, "add i2c bus device failed");

    output_reg = 0;
    configuration_reg = 0xff;
    ret = tca9554_write_byte(TCA9554_OUTPUT_PORT_REG, output_reg);
    ret |= tca9554_write_byte(TCA9554_CONFIGURATION_REG, configuration_reg);
    return ESP_OK == ret ? ESP_OK : ESP_FAIL;
}

esp_err_t tca9554_set_direction(uint8_t pin, bool is_output)
{
    ESP_RETURN_ON_FALSE(NULL != tca9554_handle, ESP_FAIL, TAG, "tca9554 is not initialized");
    ESP_RETURN_ON_FALSE(pin < 8, ESP_FAIL, TAG, "pin is invailed");
    if (is_output) {
        configuration_reg &= ~BIT(pin);
    } else {
        configuration_reg |= BIT(pin);
    }
    return tca9554_write_byte(TCA9554_CONFIGURATION_REG, configuration_reg);
}

esp_err_t tca9554_set_level(uint8_t pin, bool level)
{
    ESP_RETURN_ON_FALSE(NULL != tca9554_handle, ESP_FAIL, TAG, "tca9554 is not initialized");
    ESP_RETURN_ON_FALSE(pin < 8, ESP_FAIL, TAG, "pin is invailed");
    if (level) {
        output_reg |= BIT(pin);
    } else {
        output_reg &= ~BIT(pin);
    }
    return tca9554_write_byte(TCA9554_OUTPUT_PORT_REG, output_reg);
}

esp_err_t tca9554_read_output_pins(uint8_t *pin_val)
{
    ESP_RETURN_ON_FALSE(NULL != tca9554_handle, ESP_FAIL, TAG, "tca9554 is not initialized");
    esp_err_t ret = tca9554_read_byte(TCA9554_OUTPUT_PORT_REG, &output_reg);
    *pin_val = output_reg;
    return ret;
}

esp_err_t tca9554_read_input_pins(uint8_t *pin_val)
{
    ESP_RETURN_ON_FALSE(NULL != tca9554_handle, ESP_FAIL, TAG, "tca9554 is not initialized");
    return tca9554_read_byte(TCA9554_INPUT_PORT_REG, pin_val);
}


esp_err_t tca9554_multi_write_start(void)
{
    ESP_RETURN_ON_FALSE(NULL != tca9554_handle, ESP_ERR_INVALID_STATE, TAG, "tca9554 is not initialized");
    ESP_RETURN_ON_FALSE(NULL == g_i2c_cmd, ESP_ERR_INVALID_STATE, TAG, "tca9554_multi_write_end is not be called");
    g_i2c_cmd = i2c_cmd_link_create();
    uint8_t addr = i2c_bus_device_get_address(tca9554_handle);

    i2c_master_start(g_i2c_cmd);
    i2c_master_write_byte(g_i2c_cmd, (addr << 1) | I2C_MASTER_WRITE, I2C_ACK_CHECK_EN);
    return i2c_master_write_byte(g_i2c_cmd, TCA9554_OUTPUT_PORT_REG, I2C_ACK_CHECK_EN);
}

esp_err_t tca9554_multi_write_new_level(int pin, bool new_level)
{
    ESP_RETURN_ON_FALSE(NULL != tca9554_handle, ESP_ERR_INVALID_STATE, TAG, "tca9554 is not initialized");
    ESP_RETURN_ON_FALSE(NULL != g_i2c_cmd, ESP_ERR_INVALID_STATE, TAG, "tca9554_multi_write_start is not be called");
    if (new_level) {
        output_reg |= BIT(pin);
    } else {
        output_reg &= ~BIT(pin);
    }
    return i2c_master_write_byte(g_i2c_cmd, output_reg, I2C_ACK_CHECK_EN);
}

esp_err_t tca9554_multi_write_end(void)
{
    ESP_RETURN_ON_FALSE(NULL != tca9554_handle, ESP_ERR_INVALID_STATE, TAG, "tca9554 is not initialized");
    ESP_RETURN_ON_FALSE(NULL != g_i2c_cmd, ESP_ERR_INVALID_STATE, TAG, "tca9554_multi_write_start is not be called");
    i2c_master_stop(g_i2c_cmd);
    esp_err_t ret_val = i2c_bus_cmd_begin(tca9554_handle, g_i2c_cmd);
    i2c_cmd_link_delete(g_i2c_cmd);
    g_i2c_cmd = NULL;
    return ret_val;
}
