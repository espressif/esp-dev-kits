/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __COLOR_UTILS_H__
#define __COLOR_UTILS_H__

#include "rgb_matrix.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * @brief Convert HSL color to HSV color
 *
 * @param h Hue component (0-360)
 * @param s Saturation component (0-255)
 * @param l Lightness component (0-255)
 * @param hsv Pointer to output HSV color
*/
void hsl_to_hsv(uint16_t h, uint8_t s, uint8_t l, HSV *hsv);

#ifdef __cplusplus
}
#endif

#endif
