/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#pragma once

#include "esp_err.h"

typedef enum {
    LANGUAGE_EN = 0,
    LANGUAGE_CN,
    LANGUAGE_MAX,
} LANGUAGE_SET;

typedef struct {
    uint8_t magic;
    bool need_hint;
    uint8_t language;
} sys_param_t;

esp_err_t settings_read_parameter_from_nvs(void);
esp_err_t settings_write_parameter_to_nvs(void);
sys_param_t *settings_get_parameter(void);

