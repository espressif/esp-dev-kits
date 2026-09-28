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

/** 0=关闭扬声器；1=启用 PDM 点位提示音 */
#ifndef ENABLE_ZONE_TONE
#define ENABLE_ZONE_TONE 1
#endif

typedef struct {
    int volume_pct;
    uint32_t max_play_ms;
    uint32_t pa_settle_ms;
    uint32_t pa_hold_ms;
    int attack_pct;
    int release_pct;
} zone_tone_config_t;

/** 初始化 PDM 扬声器并加载各点位 WAV（SensairShuttle 官方 PDM 参数） */
void zone_tone_init(void);

/** 播放对应点位嵌入 WAV（5 号中心静音） */
void zone_tone_set_zone(int zone_id);

/** 点位稳定变化时触发提示音（5 号位除外） */
void zone_tone_update(int zone_id);

/** 运行时开关点位提示音（true=开启，false=关闭） */
void zone_tone_set_enabled(bool enabled);

/** 查询点位提示音当前是否开启 */
bool zone_tone_is_enabled(void);

/** 获取/更新运行时调音参数（不改动嵌入 WAV 素材本身） */
void zone_tone_get_config(zone_tone_config_t *cfg);
void zone_tone_set_config(const zone_tone_config_t *cfg);
void zone_tone_reset_config(void);

/** 用当前调音参数播放指定点位，供上位机试听 */
void zone_tone_play_test(int zone_id);

#ifdef __cplusplus
}
#endif
