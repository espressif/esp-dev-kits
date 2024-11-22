/* SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void ui_control_msc_push(void);
void ui_control_msc_pull(void);
void ui_control_msc_ready(void);
void ui_control_up_display(void);
void ui_control_down_display(void);
void ui_control_audio_control_stop(void);

#ifdef __cplusplus
}
#endif
