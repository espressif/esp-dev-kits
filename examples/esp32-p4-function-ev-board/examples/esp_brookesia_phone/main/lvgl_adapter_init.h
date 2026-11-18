/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

/* Brings up the selected 800x1280 MIPI-DSI panel (JD9366 / ILI9881C, see
 * menuconfig "LCD Panel Selection"), then registers it with the LVGL
 * adapter. Backlight (GPIO47) is turned on inside. Returns the LVGL
 * display, or NULL on failure. */
lv_disp_t *lvgl_adapter_init(void);

#ifdef __cplusplus
}
#endif
