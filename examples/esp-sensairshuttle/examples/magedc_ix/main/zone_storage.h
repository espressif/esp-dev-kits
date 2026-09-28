/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include "mag_zones_config.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void zone_storage_init(void);
const polar_zone_t *zone_storage_get(void);
int zone_storage_count(void);

/** 从 JSON 写入 NVS 并立即生效；返回 true 表示成功 */
bool zone_storage_save_from_json(const char *json, size_t len);

/** 清除 NVS 标定，恢复出厂默认（mag_zones_config.h）并写回 NVS */
void zone_storage_reset_factory(void);

bool zone_storage_has_custom(void);

/** NVS 中的标定与固件内置 mag_zones_config.h 一致 */
bool zone_storage_is_factory_profile(void);

/** 是否已完成设备端 9 点初始校准向导 */
bool zone_storage_wizard_done(void);

/** 中心和区间均持久化后，最后写完成标记；失败时返回 false。 */
bool zone_storage_mark_wizard_done(void);
void zone_storage_clear_wizard_done(void);

/** 清除用户 9 点标定（NVS blob + wizard 标志），RAM 恢复出厂默认 */
void zone_storage_clear_user_calibration(void);

/** 当前 NVS 标定是否满足运行要求（含 schema / 极坐标 / Z 门控） */
bool zone_storage_calibration_complete(void);

/** NVS 中保存的 calib_schema 版本；-1 表示未写入 */
int zone_storage_nvs_schema(void);

/** 保存 9 个极坐标区到 NVS 并立即生效 */
bool zone_storage_save_zones(const polar_zone_t *zones, int count);

/** 深度睡眠唤醒也检查完整标定，避免恢复部分写入的数据。 */
bool zone_storage_can_resume_after_sleep(void);

#ifdef __cplusplus
}
#endif
