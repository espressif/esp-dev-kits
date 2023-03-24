/**
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
#include "ht8574.h"

static const char *TAG = "ht8574";

static i2c_bus_device_handle_t ht8574_handle = NULL;
static i2c_cmd_handle_t g_i2c_cmd = NULL;
static uint8_t output_reg = 0;

static esp_err_t ht8574_read_byte(uint8_t reg_addr, uint8_t *data)
{
    return i2c_bus_read_byte(ht8574_handle, reg_addr, data);
}

static esp_err_t ht8574_write_byte(uint8_t reg_addr, uint8_t data)
{
    return i2c_bus_write_byte(ht8574_handle, reg_addr, data);
}

esp_err_t ht8574_init(uint8_t i2c_addr)
{
    esp_err_t ret = ESP_OK;
    ESP_RETURN_ON_FALSE(NULL == ht8574_handle, ESP_FAIL, TAG, "ht8574 already initialized");

    bsp_i2c_add_device(&ht8574_handle, i2c_addr);
    ESP_RETURN_ON_FALSE(NULL != ht8574_handle, ESP_FAIL, TAG, "add i2c bus device failed");

    output_reg = 0xff;
    ret = ht8574_write_byte(NULL_I2C_MEM_ADDR, output_reg);
    return ESP_OK == ret ? ESP_OK : ESP_FAIL;
}

esp_err_t ht8574_set_direction(uint8_t pin, bool is_output)
{
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t ht8574_set_level(uint8_t pin, bool level)
{
    ESP_RETURN_ON_FALSE(NULL != ht8574_handle, ESP_FAIL, TAG, "ht8574 is not initialized");
    ESP_RETURN_ON_FALSE(pin < 8, ESP_FAIL, TAG, "pin is invailed");
    if (level) {
        output_reg |= BIT(pin);
    } else {
        output_reg &= ~BIT(pin);
    }
    return ht8574_write_byte(NULL_I2C_MEM_ADDR, output_reg);
}

esp_err_t ht8574_read_output_pins(uint8_t *pin_val)
{
    ESP_RETURN_ON_FALSE(NULL != ht8574_handle, ESP_FAIL, TAG, "ht8574 is not initialized");
    esp_err_t ret = ht8574_read_byte(NULL_I2C_MEM_ADDR, &output_reg);
    *pin_val = output_reg;
    return ret;
}

esp_err_t ht8574_read_input_pins(uint8_t *pin_val)
{
    ESP_RETURN_ON_FALSE(NULL != ht8574_handle, ESP_FAIL, TAG, "ht8574 is not initialized");
    return ht8574_read_byte(NULL_I2C_MEM_ADDR, pin_val);
}


esp_err_t ht8574_multi_write_start(void)
{
    ESP_RETURN_ON_FALSE(NULL != ht8574_handle, ESP_ERR_INVALID_STATE, TAG, "ht8574 is not initialized");
    ESP_RETURN_ON_FALSE(NULL == g_i2c_cmd, ESP_ERR_INVALID_STATE, TAG, "ht8574_multi_write_end is not be called");
    g_i2c_cmd = i2c_cmd_link_create();
    uint8_t addr = i2c_bus_device_get_address(ht8574_handle);

    i2c_master_start(g_i2c_cmd);
    return i2c_master_write_byte(g_i2c_cmd, (addr << 1) | I2C_MASTER_WRITE, I2C_ACK_CHECK_EN);
}

esp_err_t ht8574_multi_write_new_level(int pin, bool new_level)
{
    ESP_RETURN_ON_FALSE(NULL != ht8574_handle, ESP_ERR_INVALID_STATE, TAG, "ht8574 is not initialized");
    ESP_RETURN_ON_FALSE(NULL != g_i2c_cmd, ESP_ERR_INVALID_STATE, TAG, "ht8574_multi_write_start is not be called");
    if (new_level) {
        output_reg |= BIT(pin);
    } else {
        output_reg &= ~BIT(pin);
    }
    return i2c_master_write_byte(g_i2c_cmd, output_reg, I2C_ACK_CHECK_EN);
}

esp_err_t ht8574_multi_write_end(void)
{
    ESP_RETURN_ON_FALSE(NULL != ht8574_handle, ESP_ERR_INVALID_STATE, TAG, "ht8574 is not initialized");
    ESP_RETURN_ON_FALSE(NULL != g_i2c_cmd, ESP_ERR_INVALID_STATE, TAG, "ht8574_multi_write_start is not be called");
    i2c_master_stop(g_i2c_cmd);
    esp_err_t ret_val = i2c_bus_cmd_begin(ht8574_handle, g_i2c_cmd);
    i2c_cmd_link_delete(g_i2c_cmd);
    g_i2c_cmd = NULL;
    return ret_val;
}
