/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_err.h"

/**
 * @brief Initialize the ESP-DualKey.
 *
 * @return ESP_OK on success, otherwise an error code.
 */
esp_err_t esp_dualkey_init(void);

#ifdef __cplusplus
}
#endif
