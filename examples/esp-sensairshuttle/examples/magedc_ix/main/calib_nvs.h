/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 启动时比对当前固件 ELF SHA256 与 NVS 绑定。
 * 只要 NVS 中已有完整用户标定，就保留并重新绑定当前固件，不因重新烧录强制重校。
 * @param preserve_user_calibration true=ESP_RST_DEEPSLEEP 唤醒，缺少标定时也不主动清理
 * 须在 nvs_flash_init() 之后、zone_storage_init() 之前调用。
 */
void calib_nvs_sync_firmware(bool preserve_user_calibration);

/** NVS 中已有 wizard_done + 磁中心 */
bool calib_nvs_has_stored_calibration(void);

/** NVS 中 has_center 标记与三个中心坐标均存在。 */
bool calib_nvs_has_center(void);

/** 9 点校准完成后绑定当前固件 */
void calib_nvs_mark_firmware_bound(void);

/** 清除中心 baseline NVS（storage 命名空间） */
void calib_nvs_clear_center(void);

#ifdef __cplusplus
}
#endif
