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

typedef enum {
    ZONE_WIZARD_IDLE = 0,
    ZONE_WIZARD_PROMPT,   /* 全灯带红色频闪：初始校准提示 */
    ZONE_WIZARD_COLLECT,  /* 当前点位亮灯，等待采样 */
    ZONE_WIZARD_POINT_OK, /* 单点完成短闪 */
} zone_wizard_phase_t;

/** 进入 9 点校准向导（红闪提示，从 5 号中心位开始） */
void zone_wizard_start(void);

void zone_wizard_abort(void);

zone_wizard_phase_t zone_wizard_get_phase(void);

/** 当前待标定点位 1–9；PROMPT 阶段返回 0 */
int zone_wizard_get_target_point(void);

/** 当前连续稳定采样数；重采时归零，调用方据此同步中心累加器。 */
int zone_wizard_get_sample_count(void);

/**
 * 主循环喂入 dX/dY/dZ（相对中心）。
 * @param window_stable 磁读数滑动窗口已稳定（磁珠保持静止）
 * @param now_ms esp_log_timestamp()
 * @return true 表示 9 点区间已保存；调用方保存中心后须调用 zone_wizard_complete。
 */
bool zone_wizard_feed_sample(float dx, float dy, float dz, bool window_stable, uint32_t now_ms);

/** 中心成功持久化后写入完成标记；失败保持待完成状态，可重试。 */
bool zone_wizard_complete(void);

#ifdef __cplusplus
}
#endif
