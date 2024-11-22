/* SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "tusb.h"

/**
 * @brief Initializes the USB application.
 */
void app_tusb_init(void);

#ifdef __cplusplus
}
#endif
