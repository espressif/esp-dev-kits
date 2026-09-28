/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include "mag_zones_config.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** 与运行时相同的极坐标 + Z 门控置信度（0~1） */
float zone_polar_confidence_gated(float r_xy, float theta_deg, float dz, const polar_zone_t *zone);

/** Z 轴门控系数（0~1），未标定 Z 时返回 1 */
float zone_polar_z_gate_score(float dz, const polar_zone_t *zone);

/** Z 标定使用有限且非零的区间；宽区间同样是有效用户标定。 */
bool zone_polar_z_bounds_valid(const polar_zone_t *zone);

/** 在 zones[0..count-1] 中找置信度最高的分区；无匹配返回 -1 */
int zone_polar_best_match(float r_xy, float theta_deg, float dz, const polar_zone_t *zones, int count,
                          float *best_confidence_out);

#ifdef __cplusplus
}
#endif
