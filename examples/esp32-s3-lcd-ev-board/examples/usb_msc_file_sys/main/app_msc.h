/* SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize USB MSC
 * 
 * @return esp_err_t 
 *         - ESP_OK: Success
 *         - Others: Fail
 */
esp_err_t msc_init(void);

#ifdef __cplusplus
}
#endif
