/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "zone_polar_detect.h"

#include <math.h>
#include <stdbool.h>

#define ZONE_SOFT_EDGE 1.30f
#define CENTER_ZONE_ID 5
#define CENTER_ZONE_RUNTIME_SCALE 0.72f

bool zone_polar_z_bounds_valid(const polar_zone_t *zone)
{
    const float span = zone->z_max - zone->z_min;
    return isfinite(zone->z_min) && isfinite(zone->z_max) && isfinite(span) && span >= 20.0f;
}

static float zone_polar_confidence(float r_xy, float theta_deg, const polar_zone_t *zone)
{
    if (zone->id == CENTER_ZONE_ID) {
        const float r_max = fmaxf(zone->r_max * CENTER_ZONE_RUNTIME_SCALE, 1.0f);
        const float r_norm = fabsf(r_xy) / r_max;
        if (r_norm > 1.0f) {
            return 0.0f;
        }
        const float weight = 0.95f + 0.05f * (zone->quality_score / 100.0f);
        return expf(-3.5f * r_norm * r_norm) * weight;
    }

    float r_mid = (zone->r_min + zone->r_max) * 0.5f;
    float r_half_width = (zone->r_max - zone->r_min) * 0.5f;
    if (r_half_width < 1.0f) {
        r_half_width = 1.0f;
    }
    const float r_norm = fabsf(r_xy - r_mid) / r_half_width;

    float th_mid;
    float th_half_width;
    if (zone->th_min <= zone->th_max) {
        th_mid = (zone->th_min + zone->th_max) * 0.5f;
        th_half_width = (zone->th_max - zone->th_min) * 0.5f;
    } else {
        th_half_width = (360.0f - (zone->th_min - zone->th_max)) * 0.5f;
        th_mid = zone->th_min + th_half_width;
        if (th_mid > 180.0f) {
            th_mid -= 360.0f;
        }
    }
    if (th_half_width < 0.5f) {
        th_half_width = 0.5f;
    }

    float th_diff = theta_deg - th_mid;
    if (th_diff > 180.0f) {
        th_diff -= 360.0f;
    }
    if (th_diff < -180.0f) {
        th_diff += 360.0f;
    }
    const float th_norm = fabsf(th_diff) / th_half_width;

    if (r_norm > ZONE_SOFT_EDGE || th_norm > ZONE_SOFT_EDGE) {
        return 0.0f;
    }

    const float GAUSSIAN_K = 3.5f;
    const float r_score = expf(-GAUSSIAN_K * r_norm * r_norm);
    const float th_score = expf(-GAUSSIAN_K * th_norm * th_norm);
    float combined_score = sqrtf(r_score * th_score);
    if (zone->id == 1 || zone->id == 8) {
        combined_score = r_score * th_score * th_score;
    } else if (zone->id == 3 || zone->id == 6 || zone->id == 9) {
        combined_score = r_score * r_score * th_score;
    }
    const float weight = 0.95f + 0.05f * (zone->quality_score / 100.0f);
    return combined_score * weight;
}

static float zone_z_gate_score(float dz, const polar_zone_t *zone)
{
#if ZONE_POLAR_Z_GATE
    if (!zone_polar_z_bounds_valid(zone)) {
        return 1.0f;
    }
    const float z_mid = (zone->z_min + zone->z_max) * 0.5f;
    const float z_half = fmaxf((zone->z_max - zone->z_min) * 0.5f, 1.0f);
    const float z_norm = fabsf(dz - z_mid) / z_half;
    if (z_norm > ZONE_SOFT_EDGE) {
        return 0.0f;
    }
    return expf(-3.5f * z_norm * z_norm);
#else
    (void)dz;
    (void)zone;
    return 1.0f;
#endif
}

float zone_polar_z_gate_score(float dz, const polar_zone_t *zone)
{
    return zone_z_gate_score(dz, zone);
}

float zone_polar_confidence_gated(float r_xy, float theta_deg, float dz, const polar_zone_t *zone)
{
    const float polar = zone_polar_confidence(r_xy, theta_deg, zone);
    if (polar <= 0.0f) {
        return 0.0f;
    }
    return polar * zone_z_gate_score(dz, zone);
}

int zone_polar_best_match(float r_xy, float theta_deg, float dz, const polar_zone_t *zones, int count,
                          float *best_confidence_out)
{
    float best_confidence = 0.0f;
    int best_id = -1;

    for (int i = 0; i < count; i++) {
        const polar_zone_t *zone = &zones[i];
        if (zone->id < 1 || zone->r_max <= zone->r_min) {
            continue;
        }
        const float confidence = zone_polar_confidence_gated(r_xy, theta_deg, dz, zone);
        if (confidence > best_confidence) {
            best_confidence = confidence;
            best_id = zone->id;
        }
    }

    if (best_confidence_out) {
        *best_confidence_out = best_confidence;
    }
    return best_id;
}
