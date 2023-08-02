/* SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include "esp_err.h"
#include "esp_log.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize audio codec
 * 
 * @return esp_err_t 
 *         - ESP_OK: success
 *         - others: fail
 */
esp_err_t audio_init(void);

#ifdef __cplusplus
}
#endif
