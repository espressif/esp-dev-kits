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

#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t ht8574_init(uint8_t i2c_addr);
esp_err_t ht8574_set_direction(uint8_t pin, bool is_output);
esp_err_t ht8574_set_level(uint8_t pin, bool level);
esp_err_t ht8574_read_output_pins(uint8_t *pin_val);
esp_err_t ht8574_read_input_pins(uint8_t *pin_val);

esp_err_t ht8574_multi_write_start(void);
esp_err_t ht8574_multi_write_new_level(int pin, bool new_level);
esp_err_t ht8574_multi_write_end(void);

#ifdef __cplusplus
}
#endif
