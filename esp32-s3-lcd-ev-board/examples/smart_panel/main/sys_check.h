/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#pragma once

#include "esp_err.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Check for system requirements.
 *
 * @return esp_err_t Check result.
 */
esp_err_t sys_check(void);

/**
 * @brief Check for image resources file.
 *
 * @param lv_img_path Path with LVGL file system type.
 * @return esp_err_t Reslut of status.
 */
esp_err_t img_file_check(const char *lv_img_path);

#ifdef __cplusplus
}
#endif

