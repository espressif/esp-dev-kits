/**
 * @file dac_audio.c
 * @brief DAC audio src file.
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

#include "dac_audio.h"

esp_err_t dac_audio_init(gpio_num_t dac_pin)
{
	if (GPIO_NUM_17 == dac_pin) {
		dac_output_enable(DAC_CHANNEL_1);
	} else if (GPIO_NUM_18 == dac_pin) {
		dac_output_enable(DAC_CHANNEL_2);
	}
	return ESP_ERR_INVALID_ARG;
}

IRAM_ATTR void dac_set_value_fast(uint8_t value)
{
	SENS.sar_dac_ctrl2.dac_cw_en2 = 0;
	RTCIO.pad_dac[DAC_CHANNEL_2].dac = value;
}
