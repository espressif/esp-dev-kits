/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "app_utils.h"
#include "math.h"
#include "esp_log.h"

/**
 * @brief Convert HSL color to HSV color
 *
 * @param h Hue component (0-360)
 * @param s Saturation component (0-255)
 * @param l Lightness component (0-255)
 * @param out_h Pointer to output hue (0-360)
 * @param out_s Pointer to output saturation (0-255)
 * @param out_v Pointer to output value (0-255)
 */
void hsl_to_hsv(uint16_t h, uint8_t s, uint8_t l, HSV *hsv)
{
    // Convert s and l from 0-255 to 0-1 float
    float sf = (float)s / 255.0f;
    float lf = (float)l / 255.0f;

    // Calculate value (v)
    float vf = lf + sf * fminf(lf, 1.0f - lf);
    // Avoid division by zero
    float sf_hsv = (vf == 0.0f) ? 0.0f : 2.0f * (1.0f - lf / vf);

    // Convert back to 0-255
    if (hsv) {
        hsv->h = (uint8_t)(h * 255.0f / 360.0f + 0.5f);
        hsv->s = (uint8_t)(sf_hsv * 255.0f + 0.5f);
        hsv->v = (uint8_t)(vf * 255.0f + 0.5f);
    }
}
