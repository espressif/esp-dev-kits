/* SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"
#include "stdbool.h"
#include "esp_err.h"

typedef enum {
    BLINK_EUB_ABORT,             /* Flashing - EUB Abort */
    BLINK_MODE,                  /* LED Blinking Mode */
    BLINK_WIRELESS_CONNECTING,   /* Flashing - Wireless Connecting */
    BLINK_WIRELESS_DISCONNECTED, /* Breathing - Wireless Disconnected */
    BLINK_NUM,                   /* LED Blinking Mode Summary - Number of LED Blinking Modes */
} led_indicator_mode_type;

extern const char* work_mode_str[];

/**
 * @brief Initialize the indicator with a given hue value
 * 
 * @param hue The hue value to be used for the indicator
 * @return
 *    - ESP_OK: succeed
 *    - others: fail
 */
esp_err_t app_indicator_init(uint16_t hue);

/**
 * @brief Set the hue of the indicator
 * 
 * @param hue The hue value to be set for the indicator
 */
void app_indicator_set_hue(uint16_t hue);

/** 
 * @brief This function returns the hue value of the indicator.
 * 
 * @return uint16_t The hue value of the indicator.
 */
uint16_t app_indicator_get_hue(void);

/**
 * @brief This function updates the mode of the LED indicator.
 * 
 * @param hue The hue value of the LED indicator.
 * @param mode_type The type of mode to be set for the LED indicator.
 * @param if_start A boolean value indicating whether the LED indicator should start or not.
 * @return esp_err_t
 *     - ESP_ERR_INVALID_ARG if parameter is invalid
 *     - ESP_ERR_NOT_FOUND no predefined blink_type found
 *     - ESP_OK Success
 */
esp_err_t app_indicator_update_mode(uint16_t hue, const led_indicator_mode_type mode_type, const bool if_start);

#ifdef __cplusplus
}
#endif
