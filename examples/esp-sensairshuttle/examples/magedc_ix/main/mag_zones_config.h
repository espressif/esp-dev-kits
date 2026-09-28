/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 * @file mag_zones_config.h
 * @brief 磁感分区标定配置（极坐标 + XYZ 三轴）
 */

#ifndef MAG_ZONES_CONFIG_H
#define MAG_ZONES_CONFIG_H

/** 1=XYZ 三轴盒判定；0=极坐标 r/θ 判定 */
#ifndef ZONE_DETECT_USE_XYZ
#define ZONE_DETECT_USE_XYZ 0
#endif

/** 极坐标模式下用各点 z_min~z_max 对置信度做门控（方案 A） */
#ifndef ZONE_POLAR_Z_GATE
#define ZONE_POLAR_Z_GATE 1
#endif

/** NVS 标定数据结构版本；升级后旧数据需重新 9 点校准 */
#ifndef CALIB_SCHEMA_VER
#define CALIB_SCHEMA_VER 5
#endif

typedef struct {
    int id;
    float r_min;
    float r_max;
    float th_min;
    float th_max;
    float x_min;
    float x_max;
    float y_min;
    float y_max;
    float z_min;
    float z_max;
    float quality_score;
} polar_zone_t;

static const polar_zone_t CALIB_ZONES[] = {
    {.id = 1, .r_min = 1762.0f, .r_max = 2083.0f, .th_min = 41.9f, .th_max = 66.4f, .quality_score = 100.0f},
    {.id = 2, .r_min = 2543.0f, .r_max = 2859.0f, .th_min = -166.1f, .th_max = -140.9f, .quality_score = 100.0f},
    {.id = 3, .r_min = 2613.0f, .r_max = 2946.0f, .th_min = 134.2f, .th_max = 159.7f, .quality_score = 100.0f},
    {.id = 4, .r_min = 5268.0f, .r_max = 5577.0f, .th_min = -101.5f, .th_max = -76.8f, .quality_score = 100.0f},
    {.id = 5, .r_min = 0.0f, .r_max = 1500.0f, .th_min = -180.0f, .th_max = 180.0f, .quality_score = 100.0f},
    {.id = 6, .r_min = 3821.0f, .r_max = 4139.0f, .th_min = 94.6f, .th_max = 118.7f, .quality_score = 100.0f},
    {.id = 7, .r_min = 2785.0f, .r_max = 3099.0f, .th_min = -50.9f, .th_max = -26.5f, .quality_score = 100.0f},
    {.id = 8, .r_min = 1824.0f, .r_max = 2163.0f, .th_min = -32.6f, .th_max = -6.7f, .quality_score = 100.0f},
    {.id = 9, .r_min = 2501.0f, .r_max = 2811.0f, .th_min = 34.7f, .th_max = 59.3f, .quality_score = 100.0f},
};

#define ZONES_COUNT (sizeof(CALIB_ZONES) / sizeof(polar_zone_t))
#endif
