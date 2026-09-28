/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "speaker_gesture.h"

#include <string.h>

#include "esp_log.h"
#include "nvs.h"

static const char *TAG = "SPEAKER_GESTURE";
static const char *NVS_NS = "spk_gesture";
static const char *NVS_KEY_SEQ = "seq";

static int s_seq[SPEAKER_GESTURE_MAX_LEN] = {1, 9};
static size_t s_seq_len = 2;

static int s_match_idx = 0;
static uint32_t s_last_step_ms = 0;
static int s_hold_zone_id = -1;
static uint32_t s_hold_since_ms = 0;
static bool s_hold_consumed = false;

static void speaker_gesture_set_default(void)
{
    s_seq[0] = 1;
    s_seq[1] = 9;
    s_seq_len = 2;
}

static bool speaker_gesture_validate(const int *seq, size_t len)
{
    if (!seq || len < 2 || len > SPEAKER_GESTURE_MAX_LEN) {
        return false;
    }
    for (size_t i = 0; i < len; i++) {
        if (seq[i] < 1 || seq[i] > 9) {
            return false;
        }
    }
    return true;
}

void speaker_gesture_reset_matcher(void)
{
    s_match_idx = 0;
    s_last_step_ms = 0;
    s_hold_zone_id = -1;
    s_hold_since_ms = 0;
    s_hold_consumed = false;
}

static bool speaker_gesture_store_to_nvs(const int *seq, size_t len)
{
    nvs_handle_t handle = 0;
    esp_err_t err = nvs_open(NVS_NS, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Open NVS failed: 0x%x", err);
        return false;
    }
    uint8_t blob[SPEAKER_GESTURE_MAX_LEN + 1] = {0};
    blob[0] = (uint8_t)len;
    for (size_t i = 0; i < len; i++) {
        blob[i + 1] = (uint8_t)seq[i];
    }
    err = nvs_set_blob(handle, NVS_KEY_SEQ, blob, len + 1);
    if (err == ESP_OK) {
        err = nvs_commit(handle);
    }
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Store NVS failed: 0x%x", err);
    }
    nvs_close(handle);
    return err == ESP_OK;
}

static void speaker_gesture_load_from_nvs(void)
{
    nvs_handle_t handle = 0;
    esp_err_t err = nvs_open(NVS_NS, NVS_READONLY, &handle);
    if (err != ESP_OK) {
        speaker_gesture_set_default();
        return;
    }
    uint8_t blob[SPEAKER_GESTURE_MAX_LEN + 1] = {0};
    size_t len = sizeof(blob);
    err = nvs_get_blob(handle, NVS_KEY_SEQ, blob, &len);
    nvs_close(handle);
    if (err != ESP_OK || len < 3) {
        speaker_gesture_set_default();
        return;
    }
    const size_t seq_len = blob[0];
    if (seq_len < 2 || seq_len > SPEAKER_GESTURE_MAX_LEN || (seq_len + 1) > len) {
        speaker_gesture_set_default();
        return;
    }
    int seq[SPEAKER_GESTURE_MAX_LEN] = {0};
    for (size_t i = 0; i < seq_len; i++) {
        seq[i] = (int)blob[i + 1];
    }
    if (!speaker_gesture_validate(seq, seq_len)) {
        speaker_gesture_set_default();
        return;
    }
    memcpy(s_seq, seq, seq_len * sizeof(int));
    s_seq_len = seq_len;
}

void speaker_gesture_init(void)
{
    speaker_gesture_load_from_nvs();
    speaker_gesture_reset_matcher();
    ESP_LOGI(TAG, "Loaded gesture len=%u", (unsigned)s_seq_len);
}

size_t speaker_gesture_get_sequence(int *out_seq, size_t max_len)
{
    if (!out_seq || max_len == 0) {
        return 0;
    }
    const size_t n = (s_seq_len < max_len) ? s_seq_len : max_len;
    memcpy(out_seq, s_seq, n * sizeof(int));
    return n;
}

bool speaker_gesture_set_sequence(const int *seq, size_t len)
{
    if (!speaker_gesture_validate(seq, len)) {
        return false;
    }
    if (!speaker_gesture_store_to_nvs(seq, len)) {
        return false;
    }
    memcpy(s_seq, seq, len * sizeof(int));
    s_seq_len = len;
    speaker_gesture_reset_matcher();
    ESP_LOGI(TAG, "Gesture updated len=%u", (unsigned)s_seq_len);
    return true;
}

bool speaker_gesture_match_step(int zone_id, float zone_conf, uint32_t now_ms, uint32_t step_timeout_ms,
                                uint32_t hold_ms, float min_conf)
{
    if (s_seq_len < 2) {
        return false;
    }

    /* 仅当相邻两步间隔超时才清零，兜底防止长时间残留进度造成误触。 */
    if (s_match_idx > 0 && (now_ms - s_last_step_ms) > step_timeout_ms) {
        s_match_idx = 0;
        s_last_step_ms = 0;
    }

    /* 无效点位或置信度不足：只结束当前停留，不清空已匹配进度，
     * 使多点手势中途短暂抬起/丢失不会前功尽弃（连续滑动容错的核心）。 */
    if (zone_id < 1 || zone_id > 9 || zone_conf < min_conf) {
        s_hold_zone_id = -1;
        s_hold_consumed = false;
        return false;
    }

    if (zone_id != s_hold_zone_id) {
        s_hold_zone_id = zone_id;
        s_hold_since_ms = now_ms;
        s_hold_consumed = false;
    }

    if (s_hold_consumed) {
        return false;
    }
    if ((now_ms - s_hold_since_ms) < hold_ms) {
        return false;
    }
    s_hold_consumed = true;

    /* 依次停留到目标点位才推进一步。 */
    if (zone_id == s_seq[s_match_idx]) {
        s_match_idx++;
        s_last_step_ms = now_ms;
        if ((size_t)s_match_idx >= s_seq_len) {
            speaker_gesture_reset_matcher();
            return true;
        }
        return false;
    }

    /* 停到序列首点则重新起步；其余“路过”的无关点位一律忽略、不清零，
     * 这是 3 点及以上能稳定识别的关键。 */
    if (zone_id == s_seq[0]) {
        s_match_idx = 1;
        s_last_step_ms = now_ms;
    }
    return false;
}
