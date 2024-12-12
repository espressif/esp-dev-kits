/**
 * @file adc_isr.h
 * @brief ADC API in isr.
 * @version 0.1
 * @date 2021-02-26
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

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "sdkconfig.h"
#include "driver/gpio.h"
#include "hal/adc_types.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "esp_log.h"
#include "esp_pm.h"
#include "soc/rtc.h"
#include "driver/rtc_io.h"
#include "sys/lock.h"
#include "driver/gpio.h"
#include "driver/adc.h"

#include "hal/adc_types.h"
#include "hal/adc_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

int adc1_get_raw_from_isr(adc1_channel_t channel);

#ifdef __cplusplus
}
#endif
