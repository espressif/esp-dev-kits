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

#include "touch_element/touch_button.h"
#include "touch_element/touch_slider.h"
#include "touch_element/touch_matrix.h"
#include "touch_element/touch_board.h"

#include "buzzer.h"
#include "rgb_light.h"
#include "digital_tube.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Button subboard initialization
 *
 * @return
 *      - ESP_OK: Successfully initialized button subboard driver
 *      - ESP_ERR_NO_MEM: Insufficient memory
 */
esp_err_t button_board_driver_install(void);

/**
 * @brief   Release resources allocated using button_board_driver_install()
 *
 * @return
 *      - ESP_OK: Successfully released resources
 *      - ESP_ERR_INVALID_STATE: Driver is not initialized
 */
esp_err_t button_board_driver_uninstall(void);

/**
 * @brief   Slider subboard initialization
 *
 * @return
 *      - ESP_OK: Successfully initialized Slider subboard driver
 *      - ESP_ERR_NO_MEM: Insufficient memory
 */
esp_err_t slider_board_driver_install(void);

/**
 * @brief   Release resources allocated using slider_board_driver_install()
 *
 * @return
 *      - ESP_OK: Successfully released resources
 *      - ESP_ERR_INVALID_STATE: Driver is not initialized
 */
esp_err_t slider_board_driver_uninstall(void);

/**
 * @brief   Matrix subboard initialization
 *
 * @return
 *      - ESP_OK: Successfully initialized Matrix subboard driver
 *      - ESP_ERR_NO_MEM: Insufficient memory
 */
esp_err_t matrix_board_driver_install(void);

/**
 * @brief   Release resources allocated using matrix_board_driver_install()
 *
 * @return
 *      - ESP_OK: Successfully released resources
 *      - ESP_ERR_INVALID_STATE: Driver is not initialized
 */
esp_err_t matrix_board_driver_uninstall(void);

/**
 * @brief   Touchpad subboard initialization
 *
 * @return
 *      - ESP_OK: Successfully initialized touchpad subboard driver
 *      - ESP_ERR_NO_MEM: Insufficient memory
 */
esp_err_t touchpad_board_driver_install(void);

/**
 * @brief   Release resources allocated using touchpad_board_driver_install()
 *
 * @return
 *      - ESP_OK: Successfully released resources
 *      - ESP_ERR_INVALID_STATE: Driver is not initialized
 */
esp_err_t touchpad_board_driver_uninstall(void);

/**
 * @brief   Proximity subboard initialization
 *
 * @return
 *      - ESP_OK: Successfully initialized Proximity subboard driver
 *      - Unknown
 */
esp_err_t proximity_board_driver_install(void);

/**
 * @brief   Release resources allocated using proximity_board_driver_install()
 *
 * @return
 *      - Unknown
 */
esp_err_t proximity_board_driver_uninstall(void);

#ifdef __cplusplus
}
#endif
