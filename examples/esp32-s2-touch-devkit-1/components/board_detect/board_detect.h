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

#pragma once

#include "driver/adc.h"
#include "esp_adc_cal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BOARD_NOTHING = 0,
    BOARD_BUTTON,       //!< Button subboard id
    BOARD_SLIDER,       //!< Slider subboard id
    BOARD_MATRIX,       //!< Matrix subboard id
    BOARD_PROXIMITY,    //!< Proximity subboard id
    BOARD_TOUCHPAD      //!< Touchpad subboard id
} subboard_type_t;

/**
 * @brief   SubBoard detection initialization
 *
 * This function initializes the SubBoard detection driver by using ADC peripheral,
 * there is resistance voltage detection circuit inside each SubBoards and theirs resistances
 * are different.
 *
 */
void board_detect_driver_install(void);

/**
 * @brief   Get current SubBoard's ID
 *
 * @return  SubBoards' ID
 */
subboard_type_t board_detect_get_subboard(void);

#ifdef __cplusplus
}
#endif
