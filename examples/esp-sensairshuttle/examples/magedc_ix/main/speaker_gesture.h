/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SPEAKER_GESTURE_MAX_LEN 8

void speaker_gesture_init(void);
size_t speaker_gesture_get_sequence(int *out_seq, size_t max_len);
bool speaker_gesture_set_sequence(const int *seq, size_t len);
void speaker_gesture_reset_matcher(void);
bool speaker_gesture_match_step(int zone_id, float zone_conf, uint32_t now_ms, uint32_t step_timeout_ms,
                                uint32_t hold_ms, float min_conf);

#ifdef __cplusplus
}
#endif
