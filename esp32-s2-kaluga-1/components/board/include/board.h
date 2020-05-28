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

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_err.h"

#include "board_kaluga_v1_1.h"
#include "board_kaluga_v1_2.h"
#include "board_kaluga_v1_3.h"

/**
 * @brief print board version
 *
 * @return - ESP_OK :Print the board version successfully
 *           ESP_FAIL: No board is defined
 */
esp_err_t kaluga_board_verison(void);

#ifdef __cplusplus
}
#endif
