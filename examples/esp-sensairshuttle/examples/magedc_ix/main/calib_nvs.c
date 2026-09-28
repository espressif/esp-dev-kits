/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "calib_nvs.h"

#include <stdio.h>
#include <string.h>

#include "esp_app_desc.h"
#include "esp_log.h"
#include "nvs.h"

static const char *TAG = "CALIB_NVS";

#define NVS_NS_CENTER "storage"
#define NVS_NS_ZONES "mag_zones"
#define NVS_KEY_FW "calib_fw"
#define NVS_KEY_FW_TS "calib_fw_ts"
#define FW_STAMP_LEN 34

static bool read_bound_firmware(uint8_t sha256_out[32])
{
    nvs_handle_t handle;
    if (nvs_open(NVS_NS_ZONES, NVS_READONLY, &handle) != ESP_OK) {
        return false;
    }
    size_t len = 32;
    esp_err_t err = nvs_get_blob(handle, NVS_KEY_FW, sha256_out, &len);
    nvs_close(handle);
    return err == ESP_OK && len == 32;
}

static void firmware_stamp_str(const esp_app_desc_t *app, char *out, size_t out_len)
{
    snprintf(out, out_len, "%.15s %.15s", app->date, app->time);
}

static bool read_bound_stamp(char *stamp_out, size_t stamp_len)
{
    nvs_handle_t handle;
    if (nvs_open(NVS_NS_ZONES, NVS_READONLY, &handle) != ESP_OK) {
        return false;
    }
    size_t len = stamp_len;
    esp_err_t err = nvs_get_str(handle, NVS_KEY_FW_TS, stamp_out, &len);
    nvs_close(handle);
    return err == ESP_OK && len > 1;
}

static void write_bound_firmware(const esp_app_desc_t *app)
{
    char stamp[FW_STAMP_LEN];

    nvs_handle_t handle;
    if (nvs_open(NVS_NS_ZONES, NVS_READWRITE, &handle) != ESP_OK) {
        return;
    }
    nvs_set_blob(handle, NVS_KEY_FW, app->app_elf_sha256, 32);
    firmware_stamp_str(app, stamp, sizeof(stamp));
    nvs_set_str(handle, NVS_KEY_FW_TS, stamp);
    nvs_commit(handle);
    nvs_close(handle);
}

/** NVS 中已有完整用户标定（向导完成 + 磁中心） */
static bool nvs_has_user_calibration(void)
{
    nvs_handle_t handle;
    uint8_t done = 0;

    if (nvs_open(NVS_NS_ZONES, NVS_READONLY, &handle) != ESP_OK) {
        return false;
    }
    esp_err_t err = nvs_get_u8(handle, "wizard_done", &done);
    nvs_close(handle);
    if (err != ESP_OK || done != 1) {
        return false;
    }

    return calib_nvs_has_center();
}

bool calib_nvs_has_center(void)
{
    nvs_handle_t handle;
    if (nvs_open(NVS_NS_CENTER, NVS_READONLY, &handle) != ESP_OK) {
        return false;
    }
    uint8_t has_center = 0;
    int32_t coordinate;
    const bool valid = nvs_get_u8(handle, "has_center", &has_center) == ESP_OK && has_center == 1 &&
                       nvs_get_i32(handle, "center_x", &coordinate) == ESP_OK &&
                       nvs_get_i32(handle, "center_y", &coordinate) == ESP_OK &&
                       nvs_get_i32(handle, "center_z", &coordinate) == ESP_OK;
    nvs_close(handle);
    return valid;
}

void calib_nvs_clear_center(void)
{
    nvs_handle_t handle;
    if (nvs_open(NVS_NS_CENTER, NVS_READWRITE, &handle) != ESP_OK) {
        return;
    }
    nvs_erase_key(handle, "has_center");
    nvs_erase_key(handle, "center_x");
    nvs_erase_key(handle, "center_y");
    nvs_erase_key(handle, "center_z");
    nvs_commit(handle);
    nvs_close(handle);
}

void calib_nvs_sync_firmware(bool preserve_user_calibration)
{
    const esp_app_desc_t *app = esp_app_get_description();
    const uint8_t *cur = app->app_elf_sha256;
    char cur_stamp[FW_STAMP_LEN];
    char bound_stamp[FW_STAMP_LEN];

    firmware_stamp_str(app, cur_stamp, sizeof(cur_stamp));

    uint8_t bound[32] = {0};
    const bool has_bound = read_bound_firmware(bound);
    const bool sha_match = has_bound && memcmp(bound, cur, 32) == 0;
    const bool stamp_match = read_bound_stamp(bound_stamp, sizeof(bound_stamp)) && strcmp(bound_stamp, cur_stamp) == 0;
    const bool has_user_calibration = nvs_has_user_calibration();

    if (has_user_calibration) {
        if (!sha_match || !stamp_match) {
            write_bound_firmware(app);
            ESP_LOGI(TAG, "Stored calibration preserved and rebound to firmware (%s)", cur_stamp);
        } else if (preserve_user_calibration) {
            ESP_LOGI(TAG, "Deep-sleep resume: preserve user calibration (%s)", cur_stamp);
        } else {
            ESP_LOGI(TAG, "Firmware unchanged (%s), keep NVS calibration", cur_stamp);
        }
        return;
    }

    if (preserve_user_calibration) {
        ESP_LOGW(TAG, "Deep-sleep resume but no stored calibration in NVS");
        return;
    }

    if (sha_match && stamp_match) {
        ESP_LOGI(TAG, "Firmware unchanged (%s), but no complete NVS calibration", cur_stamp);
        return;
    }

    if (sha_match && !stamp_match) {
        ESP_LOGW(TAG, "Rebuild/reflash detected without complete calibration (stamp \"%s\" -> \"%s\")",
                 bound_stamp[0] ? bound_stamp : "(none)", cur_stamp);
        write_bound_firmware(app);
        return;
    }

    ESP_LOGW(TAG, "New firmware ELF detected without complete calibration; wizard will run if required");
    write_bound_firmware(app);
}

bool calib_nvs_has_stored_calibration(void)
{
    return nvs_has_user_calibration();
}

void calib_nvs_mark_firmware_bound(void)
{
    const esp_app_desc_t *app = esp_app_get_description();
    char stamp[FW_STAMP_LEN];

    write_bound_firmware(app);
    firmware_stamp_str(app, stamp, sizeof(stamp));
    ESP_LOGI(TAG, "Calibration bound to firmware (%s)", stamp);
}
