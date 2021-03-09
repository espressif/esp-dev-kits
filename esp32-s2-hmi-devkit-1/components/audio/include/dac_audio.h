/**
 * @file dac_audio.h
 * @brief DAC audio header file
 * @version 0.1
 * @date 2021-02-24
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
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "driver/gpio.h"
#include "driver/dac.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "soc/dac_periph.h"
#include "soc/sens_reg.h"
#include "soc/sens_struct.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes and configure the pwm audio.
 * 
 * @param cfg configurations - see pwm_audio_config_t struct.
 * 
 * @return
 *     - ESP_OK Success.
 *     - ESP_ERR_INVALID_ARG if config a wrong pin.
 */
esp_err_t dac_audio_init(gpio_num_t dac_pin);

/**
 * @brief Set DAC output value. Can be called in isr.
 * 
 * @param value Output value.
 */
IRAM_ATTR void dac_set_value_fast(uint8_t value);

#ifdef __cplusplus
}
#endif
