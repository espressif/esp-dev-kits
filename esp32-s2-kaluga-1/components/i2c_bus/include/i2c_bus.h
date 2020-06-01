// Copyright 2019 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t i2c_bus_write_reg(uint8_t slave_addr, uint8_t reg_addr, uint8_t data);

esp_err_t i2c_bus_read_reg(uint8_t slave_addr, uint8_t reg_addr, uint8_t *pdata);

esp_err_t i2c_bus_read_data(uint8_t slave_addr, uint8_t *pdata, int size);

esp_err_t i2c_bus_init();

#ifdef __cplusplus
}
#endif
