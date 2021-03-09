/**
 * @file adc_isr.c
 * @brief ADC API in ISR
 * @version 0.1
 * @date 2021-03-04
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

#include "adc_isr.h"

extern portMUX_TYPE rtc_spinlock;

#define RTC_ENTER_CRITICAL()    portENTER_CRITICAL(&rtc_spinlock)
#define RTC_EXIT_CRITICAL()     portEXIT_CRITICAL(&rtc_spinlock)

#define SARADC1_ENTER()         RTC_ENTER_CRITICAL()
#define SARADC1_EXIT()          RTC_EXIT_CRITICAL()

#define ADC_POWER_ENTER()       RTC_ENTER_CRITICAL()
#define ADC_POWER_EXIT()        RTC_EXIT_CRITICAL()

extern esp_err_t adc1_dma_mode_acquire(void);

extern esp_err_t adc1_rtc_mode_acquire(void);

extern esp_err_t adc1_lock_release(void);

int adc1_get_raw_from_isr(adc1_channel_t channel)
{
    int adc_value;
    adc1_rtc_mode_acquire();

    SARADC1_ENTER();

    adc_hal_set_controller(ADC_NUM_1, ADC_CTRL_RTC);    //Set controller
    adc_hal_convert(ADC_NUM_1, channel, &adc_value);   //Start conversion, For ADC1, the data always valid.

    adc_hal_rtc_reset();    //Reset FSM of rtc controller

    SARADC1_EXIT();

    adc1_lock_release();
    return adc_value;
}
