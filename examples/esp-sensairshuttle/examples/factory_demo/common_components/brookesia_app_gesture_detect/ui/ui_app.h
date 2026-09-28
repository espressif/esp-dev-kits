/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef GESTURE_DETECT_UI_APP_H
#define GESTURE_DETECT_UI_APP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ui.h"

typedef enum {
    GESTURE_NONE = -1,
    GESTURE_PICKUP = 0,
    GESTURE_PUTDOWN = 1,
    GESTURE_SHAKE = 2,
    GESTURE_ROLL = 3,
    GESTURE_THROW = 4,
    GESTURE_MAX = 5,
} gesture_t;

void set_current_gesture(gesture_t gesture);
gesture_t get_current_gesture(void);
void update_gesture_ui(gesture_t gesture);
void update_gesture_detail(const char *detail_text);

#ifdef __cplusplus
}
#endif

#endif
