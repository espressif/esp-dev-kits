/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "zone_storage.h"
#include "zone_polar_detect.h"
#include "calib_nvs.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "mag_zones_config.h"
#include "nvs.h"
#include "nvs_flash.h"

static const char *TAG = "ZONE_NVS";

#define NVS_NS_ZONES "mag_zones"
#define NVS_KEY_BLOB "calib_v2"
#define NVS_KEY_WIZARD "wizard_done"
#define NVS_KEY_SCHEMA "calib_schema"
#define ZONE_MAGIC 0x4D41475Au /* MAGZ */

typedef struct
{
    uint32_t magic;
    uint32_t count;
    polar_zone_t zones[9];
} zone_nvs_blob_t;

_Static_assert(offsetof(zone_nvs_blob_t, zones) == 8, "Preserve the existing NVS zone offset");
_Static_assert(sizeof(zone_nvs_blob_t) == 8 + 9 * sizeof(polar_zone_t), "Preserve the existing NVS blob size");

static polar_zone_t s_active_zones[9];
static int s_active_count = 0;
static bool s_loaded_user_zones = false;

static void seed_xyz_from_polar(polar_zone_t *z)
{
    /* 保留向导写入的 Z 范围，避免每次启动被出厂占位覆盖 */
    if (zone_polar_z_bounds_valid(z)) {
        return;
    }
    if (z->x_max > z->x_min && z->y_max > z->y_min) {
        return;
    }

    if (z->id == 5) {
        z->x_min = -1500.0f;
        z->x_max = 1500.0f;
        z->y_min = -1500.0f;
        z->y_max = 1500.0f;
#if ZONE_DETECT_USE_XYZ
        z->z_min = -400.0f;
        z->z_max = 400.0f;
#endif
        return;
    }

    const float deg = (float)M_PI / 180.0f;
    float xs[4];
    float ys[4];
    const float rs[2] = {z->r_min, z->r_max};
    const float ts[2] = {z->th_min, z->th_max};
    int idx = 0;
    for (int ri = 0; ri < 2; ri++) {
        for (int ti = 0; ti < 2; ti++) {
            const float t = ts[ti] * deg;
            xs[idx] = rs[ri] * cosf(t);
            ys[idx] = rs[ri] * sinf(t);
            idx++;
        }
    }

    z->x_min = z->x_max = xs[0];
    z->y_min = z->y_max = ys[0];
    for (int i = 1; i < 4; i++) {
        if (xs[i] < z->x_min) {
            z->x_min = xs[i];
        }
        if (xs[i] > z->x_max) {
            z->x_max = xs[i];
        }
        if (ys[i] < z->y_min) {
            z->y_min = ys[i];
        }
        if (ys[i] > z->y_max) {
            z->y_max = ys[i];
        }
    }

    const float pad = 250.0f;
    z->x_min -= pad;
    z->x_max += pad;
    z->y_min -= pad;
    z->y_max += pad;
#if ZONE_DETECT_USE_XYZ
    z->z_min = -300.0f;
    z->z_max = 300.0f;
#endif
}

static void load_factory_defaults(void)
{
    s_loaded_user_zones = false;
    s_active_count = (int)ZONES_COUNT;
    memcpy(s_active_zones, CALIB_ZONES, sizeof(CALIB_ZONES));
    for (int i = 0; i < s_active_count; i++) {
        seed_xyz_from_polar(&s_active_zones[i]);
    }
}

static bool zones_valid(const polar_zone_t *zones, int count)
{
    if (count != 9) {
        return false;
    }
    unsigned int seen = 0;
    for (int i = 0; i < count; i++) {
        const polar_zone_t *z = &zones[i];
        if (z->id < 1 || z->id > 9 || (seen & (1U << z->id)) || !isfinite(z->quality_score) ||
                z->quality_score < 0.0f || z->quality_score > 100.0f) {
            return false;
        }
        seen |= 1U << z->id;
#if ZONE_DETECT_USE_XYZ
        if (!isfinite(z->x_min) || !isfinite(z->x_max) || z->x_max <= z->x_min ||
                !isfinite(z->y_min) || !isfinite(z->y_max) || z->y_max <= z->y_min ||
                !zone_polar_z_bounds_valid(z)) {
            return false;
        }
#else
        if (!isfinite(z->r_min) || !isfinite(z->r_max) || z->r_min < 0.0f || z->r_max <= z->r_min ||
                !isfinite(z->th_min) || !isfinite(z->th_max) || z->th_min == z->th_max) {
            return false;
        }
#endif
    }
    return true;
}

static bool load_from_nvs(void)
{
    nvs_handle_t handle;
    if (nvs_open(NVS_NS_ZONES, NVS_READONLY, &handle) != ESP_OK) {
        return false;
    }

    zone_nvs_blob_t blob = {0};
    size_t len = sizeof(blob);
    esp_err_t err = nvs_get_blob(handle, NVS_KEY_BLOB, &blob, &len);
    nvs_close(handle);

    if (err != ESP_OK || len != sizeof(blob) || blob.magic != ZONE_MAGIC ||
            !zones_valid(blob.zones, (int)blob.count)) {
        return false;
    }

    s_active_count = (int)blob.count;
    memcpy(s_active_zones, blob.zones, sizeof(polar_zone_t) * s_active_count);
    for (int i = 0; i < s_active_count; i++) {
        seed_xyz_from_polar(&s_active_zones[i]);
    }
    s_loaded_user_zones = true;
    ESP_LOGI(TAG, "Loaded %d custom zones from NVS", s_active_count);
    return true;
}

static bool save_zones_to_nvs(const polar_zone_t *zones, int count, bool invalidate_completion)
{
    zone_nvs_blob_t blob = {
        .magic = ZONE_MAGIC,
        .count = (uint32_t)count,
    };
    memcpy(blob.zones, zones, sizeof(polar_zone_t) * (size_t)count);

    nvs_handle_t handle;
    if (nvs_open(NVS_NS_ZONES, NVS_READWRITE, &handle) != ESP_OK) {
        return false;
    }
    esp_err_t err = ESP_OK;
    if (invalidate_completion) {
        err = nvs_erase_key(handle, NVS_KEY_WIZARD);
        if (err == ESP_ERR_NVS_NOT_FOUND) {
            err = ESP_OK;
        }
        if (err == ESP_OK) {
            err = nvs_commit(handle);
        }
    }
    if (err == ESP_OK) {
        err = nvs_set_blob(handle, NVS_KEY_BLOB, &blob, sizeof(blob));
    }
    if (err == ESP_OK) {
        err = nvs_commit(handle);
    }
    nvs_close(handle);
    return err == ESP_OK;
}

static bool zones_match_embedded(void)
{
    if (s_active_count != (int)ZONES_COUNT) {
        return false;
    }
    for (int i = 0; i < s_active_count; i++) {
        const polar_zone_t *ref = &CALIB_ZONES[i];
        const polar_zone_t *cur = &s_active_zones[i];
        if (cur->id != ref->id || cur->r_min != ref->r_min || cur->r_max != ref->r_max || cur->th_min != ref->th_min ||
                cur->th_max != ref->th_max || cur->quality_score != ref->quality_score) {
            return false;
        }
    }
    return true;
}

static bool zones_z_bounds_ready(void)
{
#if ZONE_POLAR_Z_GATE
    for (int i = 0; i < s_active_count; i++) {
        if (!zone_polar_z_bounds_valid(&s_active_zones[i])) {
            return false;
        }
    }
#endif
    return true;
}

static bool schema_version_current(void)
{
    nvs_handle_t handle;
    uint32_t schema = 0;
    if (nvs_open(NVS_NS_ZONES, NVS_READONLY, &handle) != ESP_OK) {
        return false;
    }
    esp_err_t err = nvs_get_u32(handle, NVS_KEY_SCHEMA, &schema);
    nvs_close(handle);
    return err == ESP_OK && schema == (uint32_t)CALIB_SCHEMA_VER;
}

bool zone_storage_calibration_complete(void)
{
    if (!zone_storage_wizard_done()) {
        return false;
    }
    if (!schema_version_current()) {
        return false;
    }
    if (!s_loaded_user_zones || !zones_valid(s_active_zones, s_active_count)) {
        return false;
    }
    if (!zones_z_bounds_ready()) {
        return false;
    }
    return true;
}

bool zone_storage_can_resume_after_sleep(void)
{
    return zone_storage_calibration_complete();
}

int zone_storage_nvs_schema(void)
{
    nvs_handle_t handle;
    uint32_t schema = 0;
    if (nvs_open(NVS_NS_ZONES, NVS_READONLY, &handle) != ESP_OK) {
        return -1;
    }
    esp_err_t err = nvs_get_u32(handle, NVS_KEY_SCHEMA, &schema);
    nvs_close(handle);
    return err == ESP_OK ? (int)schema : -1;
}

void zone_storage_init(void)
{
    if (load_from_nvs()) {
        ESP_LOGI(TAG, "Zones loaded from NVS (%s)", zones_match_embedded() ? "factory profile" : "user profile");
    } else {
        load_factory_defaults();
        if (save_zones_to_nvs(s_active_zones, s_active_count, false)) {
            ESP_LOGI(TAG, "First boot: seeded %d factory zones to NVS", s_active_count);
        } else {
            ESP_LOGW(TAG, "NVS seed failed; using embedded defaults in RAM only");
        }
    }
}

const polar_zone_t *zone_storage_get(void)
{
    return s_active_zones;
}

int zone_storage_count(void)
{
    return s_active_count;
}

bool zone_storage_has_custom(void)
{
    zone_nvs_blob_t blob = {0};
    nvs_handle_t handle;
    if (nvs_open(NVS_NS_ZONES, NVS_READONLY, &handle) != ESP_OK) {
        return false;
    }
    size_t len = sizeof(blob);
    esp_err_t err = nvs_get_blob(handle, NVS_KEY_BLOB, &blob, &len);
    nvs_close(handle);
    return err == ESP_OK && len == sizeof(blob) && blob.magic == ZONE_MAGIC &&
           zones_valid(blob.zones, (int)blob.count);
}

bool zone_storage_wizard_done(void)
{
    nvs_handle_t handle;
    uint8_t done = 0;
    if (nvs_open(NVS_NS_ZONES, NVS_READONLY, &handle) != ESP_OK) {
        return false;
    }
    esp_err_t err = nvs_get_u8(handle, NVS_KEY_WIZARD, &done);
    nvs_close(handle);
    return err == ESP_OK && done == 1;
}

bool zone_storage_mark_wizard_done(void)
{
    if (!s_loaded_user_zones || !zones_valid(s_active_zones, s_active_count) || !zones_z_bounds_ready() ||
            !calib_nvs_has_center()) {
        return false;
    }
    nvs_handle_t handle;
    if (nvs_open(NVS_NS_ZONES, NVS_READWRITE, &handle) != ESP_OK) {
        return false;
    }
    esp_err_t err = nvs_set_u32(handle, NVS_KEY_SCHEMA, (uint32_t)CALIB_SCHEMA_VER);
    if (err == ESP_OK) {
        err = nvs_set_u8(handle, NVS_KEY_WIZARD, 1);
    }
    if (err == ESP_OK) {
        err = nvs_commit(handle);
    }
    nvs_close(handle);
    if (err != ESP_OK) {
        return false;
    }
    ESP_LOGI(TAG, "Initial zone wizard marked done");
    return true;
}

void zone_storage_clear_wizard_done(void)
{
    nvs_handle_t handle;
    if (nvs_open(NVS_NS_ZONES, NVS_READWRITE, &handle) != ESP_OK) {
        return;
    }
    nvs_erase_key(handle, NVS_KEY_WIZARD);
    nvs_commit(handle);
    nvs_close(handle);
}

void zone_storage_clear_user_calibration(void)
{
    nvs_handle_t handle;
    if (nvs_open(NVS_NS_ZONES, NVS_READWRITE, &handle) == ESP_OK) {
        nvs_erase_key(handle, NVS_KEY_BLOB);
        nvs_erase_key(handle, NVS_KEY_WIZARD);
        nvs_erase_key(handle, NVS_KEY_SCHEMA);
        nvs_commit(handle);
        nvs_close(handle);
    }
    load_factory_defaults();
    ESP_LOGI(TAG, "User zone calibration cleared (RAM = factory defaults)");
}

bool zone_storage_save_zones(const polar_zone_t *zones, int count)
{
    if (!zones || !zones_valid(zones, count)) {
        return false;
    }
    if (!save_zones_to_nvs(zones, count, true)) {
        ESP_LOGE(TAG, "zone blob save failed");
        return false;
    }
    s_active_count = count;
    memcpy(s_active_zones, zones, sizeof(polar_zone_t) * (size_t)count);
    s_loaded_user_zones = true;
    ESP_LOGI(TAG, "Saved %d wizard zones to NVS", count);
    return true;
}

void zone_storage_reset_factory(void)
{
    nvs_handle_t handle;
    if (nvs_open(NVS_NS_ZONES, NVS_READWRITE, &handle) == ESP_OK) {
        nvs_erase_all(handle);
        nvs_commit(handle);
        nvs_close(handle);
    }
    load_factory_defaults();
    if (save_zones_to_nvs(s_active_zones, s_active_count, false)) {
        ESP_LOGI(TAG, "Factory zones restored to NVS");
    } else {
        ESP_LOGW(TAG, "Factory zones loaded in RAM only (NVS write failed)");
    }
}

bool zone_storage_is_factory_profile(void)
{
    return zone_storage_has_custom() && zones_match_embedded();
}

static float parse_json_float(const char *json, const char *key)
{
    char search[32];
    snprintf(search, sizeof(search), "\"%s\"", key);
    const char *p = strstr(json, search);
    if (!p) {
        return 0.0f;
    }
    p = strchr(p, ':');
    if (!p) {
        return 0.0f;
    }
    return strtof(p + 1, NULL);
}

bool zone_storage_save_from_json(const char *json, size_t len)
{
    if (!json || len < 32) {
        return false;
    }

    polar_zone_t tmp[9] = {0};
    int count = 0;

    for (int id = 1; id <= 9; id++) {
        char idpat[16];
        snprintf(idpat, sizeof(idpat), "\"%d\"", id);
        const char *block = strstr(json, idpat);
        if (!block) {
            continue;
        }
        const char *next = NULL;
        for (int nid = id + 1; nid <= 9; nid++) {
            snprintf(idpat, sizeof(idpat), "\"%d\"", nid);
            next = strstr(block + 1, idpat);
            if (next) {
                break;
            }
        }
        size_t blen = next ? (size_t)(next - block) : len - (size_t)(block - json);
        char chunk[256];
        if (blen >= sizeof(chunk)) {
            blen = sizeof(chunk) - 1;
        }
        memcpy(chunk, block, blen);
        chunk[blen] = '\0';

        tmp[count].id = id;
        tmp[count].r_min = parse_json_float(chunk, "r_min");
        tmp[count].r_max = parse_json_float(chunk, "r_max");
        tmp[count].th_min = parse_json_float(chunk, "th_min");
        tmp[count].th_max = parse_json_float(chunk, "th_max");
        tmp[count].quality_score = parse_json_float(chunk, "quality_score");
        if (tmp[count].quality_score <= 0.0f) {
            tmp[count].quality_score = 80.0f;
        }
        if (tmp[count].r_max > tmp[count].r_min) {
            count++;
        }
    }

    if (count < 9) {
        ESP_LOGW(TAG, "JSON parse: only %d/9 zones", count);
        return false;
    }

    if (!zones_valid(tmp, count) || !save_zones_to_nvs(tmp, count, true)) {
        ESP_LOGE(TAG, "NVS save failed");
        return false;
    }
    s_active_count = count;
    memcpy(s_active_zones, tmp, sizeof(tmp));
    s_loaded_user_zones = true;
    ESP_LOGI(TAG, "Saved %d zones to NVS", count);
    return true;
}
