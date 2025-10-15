/*
 * SPDX-FileCopyrightText: 2021-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#ifndef _APP_EXAMPLE_H_
#define _APP_EXAMPLE_H_

#ifdef __cplusplus
extern "C" {
#endif /**< __cplusplus */

#include <stdint.h>
#include "esp_err.h"
#include "nvs_flash.h"

esp_err_t app_nvs_open(nvs_handle_t *handle);

void app_nvs_close(nvs_handle_t handle);

esp_err_t app_nvs_store(nvs_handle_t handle, const char *key, const void *data, size_t length);

esp_err_t app_nvs_get_length(nvs_handle_t handle, const char *key, size_t *length);

esp_err_t app_nvs_restore(nvs_handle_t handle, const char *key, void *data, size_t length, bool *exist);

esp_err_t app_nvs_erase(nvs_handle_t handle, const char *key);

#ifdef __cplusplus
}
#endif /**< __cplusplus */

#endif /* _APP_NVS_H_ */
