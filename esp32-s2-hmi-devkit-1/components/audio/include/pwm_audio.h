/**
 * @file pwm_audio.h
 * @brief PWM audio header file
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

#ifndef _PWM_AUDIO_H_
#define _PWM_AUDIO_H_

#include <stdio.h>
#include <string.h>

#include "driver/ledc.h"
#include "driver/timer.h"

#include "esp_err.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_heap_caps.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "soc/ledc_struct.h"
#include "soc/ledc_reg.h"

#include "sdkconfig.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Configuration parameters for pwm_audio_init function
 */
typedef struct
{
    int gpio_num;                       /*!< the LEDC output gpio_num */
    ledc_channel_t ledc_channel;        /*!< LEDC channel (0 - 7), Corresponding to left channel*/
    ledc_timer_t ledc_timer_sel;        /*!< Select the timer source of channel (0 - 3) */
    ledc_timer_bit_t duty_resolution;   /*!< ledc pwm bits */
} pwm_audio_config_t;

/**
 * @brief Initializes and configure the pwm audio.
 * 
 * @param cfg configurations - see pwm_audio_config_t struct
 * 
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL timer_group or ledc initialize failed
 *     - ESP_ERR_INVALID_ARG if argument wrong
 *     - ESP_ERR_INVALID_STATE The pwm audio already configure
 *     - ESP_ERR_NO_MEM Memory allocate failed
 */
esp_err_t pwm_audio_init(const pwm_audio_config_t *cfg);

/**
 * @brief Deinitialize LEDC timer_group and output gpio
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL pwm_audio not initialized
 */
esp_err_t pwm_audio_deinit(void);

/**
 * @brief Set PWM output duty cycle. Can be called in isr.
 * 
 * @param duty_val Duty cycle.
 */
IRAM_ATTR void ledc_set_duty_fast(uint32_t duty_val);

#ifdef __cplusplus
}
#endif

#endif
