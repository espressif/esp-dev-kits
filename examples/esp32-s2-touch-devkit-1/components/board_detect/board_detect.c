// Copyright 2021 Espressif Systems (Shanghai) PTE LTD
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

#include "board_detect.h"

void board_detect_driver_install(void)
{
    gpio_num_t adc_gpio_num;
    esp_err_t ret = adc2_pad_get_io_num((adc2_channel_t)ADC_CHANNEL_6, &adc_gpio_num);
    assert(ret == ESP_OK);
    adc2_config_channel_atten((adc2_channel_t)ADC_CHANNEL_6, ADC_ATTEN_11db);
}

subboard_type_t board_detect_get_subboard(void)
{//TODO: refactor this implementation
    uint32_t adc_value = 0;
    subboard_type_t board = BOARD_NOTHING;

    for (int i = 0; i < 10; i++) {
        int raw;
        adc2_get_raw((adc2_channel_t)ADC_CHANNEL_6, ADC_WIDTH_BIT_13, &raw);
        adc_value += raw;
    }
    adc_value /= 10;
    if (adc_value < 1500) {
        board = BOARD_TOUCHPAD;//974
    } else if (adc_value > 1600 && adc_value < 2500) {
        board = BOARD_BUTTON;//1979
    } else if (adc_value > 2600 && adc_value < 3500) {
        board = BOARD_MATRIX;//3230
    } else if (adc_value > 3600 && adc_value < 4500) {
        board = BOARD_SLIDER;//4023
    } else if (adc_value > 4600 && adc_value < 5500) {
        board = BOARD_PROXIMITY;//5060
    } else {
        board = BOARD_NOTHING;
    }
    return board;
}
