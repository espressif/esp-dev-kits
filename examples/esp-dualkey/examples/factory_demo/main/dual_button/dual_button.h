/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include "keyboard_button.h"
#include "iot_button.h"
#include "esp_err.h"
#include "button_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t active_level;          /**< gpio level when press down */
    int32_t gpio_num[2];           /**< num of gpio */
    bool disable_pull;             /**< disable internal pull up or down */
    bool skip_gpio_init;           /**< skip gpio init */
} button_dual_config_t;

/*
 * @brief Create a new dual button device.
 *
 * @param button_config Button configuration.
 * @param dual_btn_cfg Dual button configuration.
 * @param ret_button Returned button handle.
 *
 * @return ESP_OK on success, otherwise an error code.
*/
esp_err_t iot_button_new_dual_button_device(const button_config_t *button_config, const button_dual_config_t *dual_btn_cfg, button_handle_t *ret_button);

#ifdef __cplusplus
}
#endif
