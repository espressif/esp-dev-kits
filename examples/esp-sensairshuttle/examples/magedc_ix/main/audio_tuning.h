/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t settle_hold_ms;
    float min_conf;
    uint32_t cooldown_ms;
} audio_gate_config_t;

void audio_gate_get_config(audio_gate_config_t *cfg);
void audio_gate_set_config(const audio_gate_config_t *cfg);
void audio_gate_reset_config(void);

#ifdef __cplusplus
}
#endif
