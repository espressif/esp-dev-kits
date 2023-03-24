/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#ifndef __UI_SR_H__
#define __UI_SR_H__

#include <stddef.h>
#include "esp_err.h"
#include "lvgl.h"

#include "bsp/esp-bsp.h"
#include "lv_example_pub.h"
#include "lv_example_image.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (* anim_handle_cb)(void);
/**
 * @brief
 *
 */
void sr_anim_start(void);

/**
 * @brief
 *
 */
void sr_anim_stop(void);

/**
 * @brief
 *
 * @param text
 */
void sr_anim_set_text(char *text);

/**
 * @brief
 *
 */
anim_handle_cb sr_anim_task(lv_layer_t *layer);

#ifdef __cplusplus
}
#endif

#endif
