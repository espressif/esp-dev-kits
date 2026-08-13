/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp/display.h"
#include "esp_lv_adapter_display.h"

lv_display_t *lvgl_adapter_init(const bsp_display_config_t *cfg);

#ifdef __cplusplus
}
#endif
