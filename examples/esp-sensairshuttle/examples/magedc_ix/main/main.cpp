/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* 开发期临时开关：1=烧录后跳过校准向导；0=恢复烧录后校准流程 */
#define TEMP_SKIP_REFLASH_WIZARD 0

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "driver/i2c_master.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "bmm350.h"
#include "bmm350_common.h"
#include <math.h>
#include <stdbool.h>
#include "driver/gpio.h"
#include "esp_board_manager.h"
#include "esp_board_manager_includes.h"
#include "led_strip.h"
#include "esp_sleep.h"
#include "bmi270_api.h"
#include "bmi2.h"
#include "mag_zones_config.h"
#include "wifi_stream.h"
#include "web_portal.h"
#include "zone_storage.h"
#include "zone_tone.h"
#include "speaker_gesture.h"
#include "zone_wizard.h"
#include "zone_polar_detect.h"
#include "calib_nvs.h"
#include "audio_tuning.h"

#define I2C_MASTER_SDO_IO 9       /*!< GPIO for I2C address selection */
#define BMM350_I2C_ADDR 0x14
#define IDLE_SLEEP_MS (60 * 1000)

static const char *TAG = "MAG_DEMO";
static i2c_master_bus_handle_t s_sensor_i2c_bus = NULL;

#define PI 3.14159265358979323846f

/* --------------------------------------------------------------------------
 * 5号点（中心原点）校准数据存储与管理
 * -------------------------------------------------------------------------- */

int32_t center_x = 0;
int32_t center_y = 0;
int32_t center_z = 0;

static bool save_center_to_nvs(void)
{
    nvs_handle_t handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        return false;
    }
    err = nvs_set_i32(handle, "center_x", center_x);
    if (err == ESP_OK) {
        err = nvs_set_i32(handle, "center_y", center_y);
    }
    if (err == ESP_OK) {
        err = nvs_set_i32(handle, "center_z", center_z);
    }
    if (err == ESP_OK) {
        err = nvs_set_u8(handle, "has_center", 1);
    }
    if (err == ESP_OK) {
        err = nvs_commit(handle);
    }
    nvs_close(handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Save magnetic center failed: %s", esp_err_to_name(err));
        return false;
    }
    ESP_LOGI(TAG, "Center calibrated coordinates saved to NVS: X:%d Y:%d Z:%d", (int)center_x, (int)center_y,
             (int)center_z);
    return true;
}

static bool load_center_from_nvs(void)
{
    nvs_handle_t handle;
    if (nvs_open("storage", NVS_READONLY, &handle) != ESP_OK) {
        return false;
    }
    uint8_t has_center = 0;
    int32_t x = 0, y = 0, z = 0;
    const bool valid = nvs_get_u8(handle, "has_center", &has_center) == ESP_OK && has_center == 1 &&
                       nvs_get_i32(handle, "center_x", &x) == ESP_OK &&
                       nvs_get_i32(handle, "center_y", &y) == ESP_OK &&
                       nvs_get_i32(handle, "center_z", &z) == ESP_OK;
    nvs_close(handle);
    if (valid) {
        center_x = x;
        center_y = y;
        center_z = z;
        ESP_LOGI(TAG, "Center loaded from NVS: X:%d Y:%d Z:%d", (int)x, (int)y, (int)z);
    }
    return valid;
}

/* --------------------------------------------------------------------------
 * 校准态稳定性检测窗口
 * -------------------------------------------------------------------------- */

#define WIZARD_WINDOW_SIZE 20
#define WIZARD_STABLE_VARIANCE_SQ 520

static struct {
    float x[WIZARD_WINDOW_SIZE];
    float y[WIZARD_WINDOW_SIZE];
    float z[WIZARD_WINDOW_SIZE];
    int index;
    bool is_full;
} wizard_window = {{0}, {0}, {0}, 0, false};

static void reset_wizard_window(void)
{
    wizard_window.index = 0;
    wizard_window.is_full = false;
}

static void add_to_wizard_window(float x, float y, float z)
{
    wizard_window.x[wizard_window.index] = x;
    wizard_window.y[wizard_window.index] = y;
    wizard_window.z[wizard_window.index] = z;
    wizard_window.index++;
    if (wizard_window.index >= WIZARD_WINDOW_SIZE) {
        wizard_window.index = 0;
        wizard_window.is_full = true;
    }
}

static bool is_wizard_window_stable(int max_variance)
{
    if (!wizard_window.is_full) {
        return false;
    }

    double sum_x = 0.0, sum_y = 0.0, sum_z = 0.0;
    for (int i = 0; i < WIZARD_WINDOW_SIZE; i++) {
        sum_x += wizard_window.x[i];
        sum_y += wizard_window.y[i];
        sum_z += wizard_window.z[i];
    }
    const float avg_x = (float)(sum_x / WIZARD_WINDOW_SIZE);
    const float avg_y = (float)(sum_y / WIZARD_WINDOW_SIZE);
    const float avg_z = (float)(sum_z / WIZARD_WINDOW_SIZE);

    double variance = 0.0;
    for (int i = 0; i < WIZARD_WINDOW_SIZE; i++) {
        const float dx = wizard_window.x[i] - avg_x;
        const float dy = wizard_window.y[i] - avg_y;
        const float dz = wizard_window.z[i] - avg_z;
        variance += (double)dx * dx + (double)dy * dy + (double)dz * dz;
    }
    variance /= WIZARD_WINDOW_SIZE;

    return variance < (double)max_variance;
}

static void wizard_window_mean(float *mx, float *my, float *mz)
{
    double sum_x = 0.0, sum_y = 0.0, sum_z = 0.0;
    const int count = wizard_window.is_full ? WIZARD_WINDOW_SIZE : wizard_window.index;
    if (count <= 0) {
        *mx = *my = *mz = 0.0f;
        return;
    }
    for (int i = 0; i < count; i++) {
        sum_x += wizard_window.x[i];
        sum_y += wizard_window.y[i];
        sum_z += wizard_window.z[i];
    }
    *mx = (float)(sum_x / count);
    *my = (float)(sum_y / count);
    *mz = (float)(sum_z / count);
}

/* --------------------------------------------------------------------------
 * System state machine
 * -------------------------------------------------------------------------- */

enum SystemState {
    STATE_ZONE_WIZARD, /* 9 点极坐标校准（含 5 号磁中心） */
    STATE_RUNNING
};

SystemState current_state = STATE_RUNNING;

#define POSITION_INVALID (-1) /* 不在任何标定点内 / 远离所有区域 */
int last_stable_id = POSITION_INVALID;
static volatile int s_action_zone_id = POSITION_INVALID;
static volatile float s_action_zone_confidence = 0.0f;
static volatile int s_visual_zone_id = POSITION_INVALID;
static volatile int s_visual_from_zone_id = POSITION_INVALID;
static volatile int s_visual_to_zone_id = POSITION_INVALID;
static volatile float s_visual_blend_t = 1.0f;
static volatile float s_visual_zone_confidence = 0.0f;

static void reset_zone_runtime_channels(bool clear_action);

static bool idle_sleep_due(bool running, bool activity, uint32_t now_ms, uint32_t *last_activity_ms)
{
    if (!running || activity) {
        *last_activity_ms = now_ms;
    }
    return running && (now_ms - *last_activity_ms) >= IDLE_SLEEP_MS;
}

static void start_zone_wizard_calibration(void)
{
    current_state = STATE_ZONE_WIZARD;
    reset_wizard_window();
    center_x = center_y = center_z = 0;
    last_stable_id = POSITION_INVALID;
    reset_zone_runtime_channels(true);
#if ENABLE_WIFI
    /* 校准期间停掉 Wi-Fi：Wi-Fi 关联会破坏 9 点向导采样导致卡红灯不推进，
     * 此处复刻“无网首次烧录校准”这一已知可用条件；完成后自动重连。 */
    wifi_stream_set_calibrating(true);
#endif
    zone_wizard_start();
    ESP_LOGW(TAG, "--- 9-point zone calibration started (center from zone 5) ---");
}

static void request_full_recalibration(void)
{
    calib_nvs_clear_center();
    zone_storage_clear_user_calibration();
    start_zone_wizard_calibration();
    ESP_LOGW(TAG, "BOOT: full 9-point recalibration started");
}

/* --------------------------------------------------------------------------
 * 穴位判定：极坐标 r/θ + Z 门控
 * -------------------------------------------------------------------------- */

#define ZONE_ACTIVE_THRESHOLD 0.05f
#define HYSTERESIS_MARGIN_BASE 0.035f
#define HYSTERESIS_ADJ_EXTRA 0.06f
#define HYSTERESIS_MIN_HOLD 0.12f
#define ADJ_CONF_TIE_GAP 0.07f
#define FAR_ZONE_WIN_GAP 0.09f
#define CONFUSION_PAIR_WIN_GAP 0.16f
#define STALE_LOCK_SWITCH_FRAMES 2

#define CONF_EMA_ALPHA 0.48f

#define DEBOUNCE_STABLE_FRAMES 2
#define DEBOUNCE_ADJ_STABLE 3
#define DEBOUNCE_ADJ_FRAMES 3
#define DEBOUNCE_ADJ_BUDGET 0.70f
#define DEBOUNCE_ADJ_CONF_GAP 0.08f
#define DEBOUNCE_ADJ_FAST_GAP 0.14f
#define DEBOUNCE_NORM_FRAMES 2
#define DEBOUNCE_NORM_BUDGET 0.85f
#define DEBOUNCE_FAR_FRAMES 3
#define DEBOUNCE_FAR_BUDGET 1.35f
#define DEBOUNCE_FAR_CONF_GAP 0.06f
#define DEBOUNCE_CONF_HIGH 0.35f
#define DEBOUNCE_CONF_LOW 0.10f
#define DEBOUNCE_MIN_STREAK 0.04f
#define DEBOUNCE_PINGPONG_EXTRA 2
#define NEAREST_ADJ_MARGIN 0.88f

#define ACTION_MIN_HOLD_MS 70U
#define ACTION_MIN_CONF 0.12f
#define ACTION_CONFIRM_FRAMES 2
#define ACTION_RELEASE_MS 110U

#define VISUAL_CONF_WEAK 0.05f
#define VISUAL_CONF_SOLID 0.14f
#define VISUAL_EASE_MS_BASE 150U
#define VISUAL_EASE_MS_FAST 120U
#define VISUAL_EASE_MS_SLOW 180U
/* LED 速度判定：action 切换驱动的漏桶积分（leaky integrator）。
 * 每次 action 切换 +VISUAL_RATE_IMPULSE，桶值随时间按 VISUAL_RATE_LEAK_MS 泄漏。
 * 切换越密集 -> 桶值越高 -> 颜色越偏红；停手后缓慢回落到蓝，颜色不会一闪即逝。 */
#define VISUAL_RATE_IMPULSE 1.0f
#define VISUAL_RATE_MAX 3.0f
#define VISUAL_RATE_LEAK_MS 300.0f
#define VISUAL_RATE_BLUE 0.8f
#define VISUAL_RATE_RED 3.7f

#define CORNER_ACTION_CONF_FLOOR_SCALE 0.65f
#define CORNER_ACTION_CONF_BOOST 0.03f
#define CORNER_ACTION_HOLD_REDUCE_MS 12
#define CORNER_ACTION_BUDGET_SCALE 0.78f
#define CORNER_SWITCH_BUDGET_SCALE 0.82f

#define SPEAKER_TOGGLE_STEP_TIMEOUT_MS 2200U
#define SPEAKER_TOGGLE_STEP_HOLD_MS 140U
#define SPEAKER_TOGGLE_MIN_CONF 0.28f
#define IDLE_ACTIVITY_CONF_MIN 0.22f
#define IMU_WAKE_PIN_SETTLE_TIMEOUT_MS 220U

#define AUDIO_SETTLE_HOLD_MS 170U
#define AUDIO_MIN_CONF 0.22f
#define AUDIO_COOLDOWN_MS 20U

#define MOTION_THETA_WEIGHT 4.5f
#define MOTION_DZ_WEIGHT 0.35f
#define MOTION_EMA_ALPHA 0.35f
#define MOTION_SLIDE_ENTER 72.0f
#define MOTION_SLIDE_EXIT 45.0f

#define ZONE_SOFT_EDGE 1.30f
#define ZONE_LOCK_RELEASE_CONF 0.03f
#define CENTER_ZONE_ID 5
#define CENTER_ZONE_RUNTIME_SCALE 0.72f

typedef enum {
    DEBOUNCE_PROFILE_STILL = 0,
    DEBOUNCE_PROFILE_SLIDING = 1,
} debounce_profile_t;

typedef enum {
    AUDIO_GATE_IDLE = 0,
    AUDIO_GATE_SLIDING_SUPPRESSED,
    AUDIO_GATE_PENDING_SETTLE,
    AUDIO_GATE_PLAYED,
} audio_gate_state_t;

static volatile debounce_profile_t s_motion_profile = DEBOUNCE_PROFILE_STILL;
static float s_motion_metric_ema = 0.0f;
static float s_motion_prev_r = 0.0f;
static float s_motion_prev_theta = 0.0f;
static float s_motion_prev_dz = 0.0f;
static uint32_t s_motion_prev_ts = 0;
static bool s_motion_has_prev = false;

static int s_action_candidate_id = POSITION_INVALID;
static uint32_t s_action_candidate_since_ms = 0;
static int s_action_candidate_frames = 0;
static float s_action_candidate_budget = 0.0f;
static float s_action_candidate_peak = 0.0f;
static uint32_t s_action_last_valid_ms = 0;

static int s_visual_target_id = POSITION_INVALID;
static int s_visual_origin_id = POSITION_INVALID;
static uint32_t s_visual_blend_start_ms = 0;
static uint32_t s_visual_blend_duration_ms = VISUAL_EASE_MS_BASE;
static volatile float s_action_rate = 0.0f;    /* 漏桶积分：action 切换驱动，随时间泄漏 */
static volatile uint32_t s_action_rate_ts = 0; /* 上次更新 s_action_rate 的时间 */

static audio_gate_state_t s_audio_gate_state = AUDIO_GATE_IDLE;
static int s_audio_pending_zone = POSITION_INVALID;
static uint32_t s_audio_pending_since_ms = 0;
static float s_audio_pending_peak_conf = 0.0f;
static int s_audio_last_played_zone = POSITION_INVALID;
static uint32_t s_audio_last_played_ms = 0;
static audio_gate_config_t s_audio_gate_cfg = {
    AUDIO_SETTLE_HOLD_MS,
    AUDIO_MIN_CONF,
    AUDIO_COOLDOWN_MS,
};

static const polar_zone_t *zone_by_id(int zone_id)
{
    for (int i = 0; i < zone_storage_count(); i++) {
        const polar_zone_t *zone = &zone_storage_get()[i];
        if (zone->id == zone_id) {
            return zone;
        }
    }
    return nullptr;
}

static int zone_grid_distance(int a, int b)
{
    if (a < 1 || a > 9 || b < 1 || b > 9) {
        return 99;
    }
    if (a == b) {
        return 0;
    }
    int ra = (a - 1) / 3, ca = (a - 1) % 3;
    int rb = (b - 1) / 3, cb = (b - 1) % 3;
    int dr = ra - rb;
    int dc = ca - cb;
    if (dr < 0) {
        dr = -dr;
    }
    if (dc < 0) {
        dc = -dc;
    }
    return dr + dc;
}

static bool zones_are_adjacent(int a, int b)
{
    return zone_grid_distance(a, b) == 1;
}

static const int CONFUSION_PAIRS[][2] = {{1, 8}, {3, 6}, {6, 9}};
#define CONFUSION_PAIR_COUNT 3

static bool zones_are_confusion_pair(int a, int b)
{
    if (a < 1 || b < 1) {
        return false;
    }
    for (int i = 0; i < CONFUSION_PAIR_COUNT; i++) {
        if ((CONFUSION_PAIRS[i][0] == a && CONFUSION_PAIRS[i][1] == b) ||
                (CONFUSION_PAIRS[i][0] == b && CONFUSION_PAIRS[i][1] == a)) {
            return true;
        }
    }
    return false;
}

static volatile bool s_position_debounce_reset = false;

static void position_debounce_reset(void)
{
    s_position_debounce_reset = true;
}

static float clamp01f(float v)
{
    if (v < 0.0f) {
        return 0.0f;
    }
    if (v > 1.0f) {
        return 1.0f;
    }
    return v;
}

static uint32_t clamp_u32_range(uint32_t v, uint32_t min_v, uint32_t max_v)
{
    if (v < min_v) {
        return min_v;
    }
    if (v > max_v) {
        return max_v;
    }
    return v;
}

static float clamp_float_range(float v, float min_v, float max_v)
{
    if (v < min_v) {
        return min_v;
    }
    if (v > max_v) {
        return max_v;
    }
    return v;
}

static float smoothstepf(float t)
{
    t = clamp01f(t);
    return t * t * (3.0f - 2.0f * t);
}

static float wrap_angle_delta_deg(float deg)
{
    while (deg > 180.0f) {
        deg -= 360.0f;
    }
    while (deg < -180.0f) {
        deg += 360.0f;
    }
    return deg;
}

static const char *debounce_profile_name(debounce_profile_t profile)
{
    return (profile == DEBOUNCE_PROFILE_SLIDING) ? "slide" : "still";
}

static bool audio_zone_is_playable(int zone_id)
{
    return zone_id >= 1 && zone_id <= 9 && zone_id != CENTER_ZONE_ID;
}

static void audio_gate_reset(void)
{
    s_audio_gate_state = AUDIO_GATE_IDLE;
    s_audio_pending_zone = POSITION_INVALID;
    s_audio_pending_since_ms = 0;
    s_audio_pending_peak_conf = 0.0f;
}

void audio_gate_get_config(audio_gate_config_t *cfg)
{
    if (!cfg) {
        return;
    }
    *cfg = s_audio_gate_cfg;
}

void audio_gate_set_config(const audio_gate_config_t *cfg)
{
    if (!cfg) {
        return;
    }
    s_audio_gate_cfg.settle_hold_ms = clamp_u32_range(cfg->settle_hold_ms, 40, 800);
    s_audio_gate_cfg.min_conf = clamp_float_range(cfg->min_conf, 0.02f, 0.80f);
    s_audio_gate_cfg.cooldown_ms = clamp_u32_range(cfg->cooldown_ms, 0, 1500);
    ESP_LOGI(TAG, "Audio gate cfg: settle=%u min_conf=%.3f cooldown=%u", (unsigned)s_audio_gate_cfg.settle_hold_ms,
             (double)s_audio_gate_cfg.min_conf, (unsigned)s_audio_gate_cfg.cooldown_ms);
}

void audio_gate_reset_config(void)
{
    const audio_gate_config_t defaults = {
        AUDIO_SETTLE_HOLD_MS,
        AUDIO_MIN_CONF,
        AUDIO_COOLDOWN_MS,
    };
    audio_gate_set_config(&defaults);
}

static bool is_corner_zone(int zone_id)
{
    return (zone_id == 1 || zone_id == 3 || zone_id == 7 || zone_id == 9);
}

static void reset_zone_runtime_channels(bool clear_action)
{
    s_motion_metric_ema = 0.0f;
    s_motion_prev_r = 0.0f;
    s_motion_prev_theta = 0.0f;
    s_motion_prev_dz = 0.0f;
    s_motion_prev_ts = 0;
    s_motion_has_prev = false;
    s_motion_profile = DEBOUNCE_PROFILE_STILL;

    s_action_candidate_id = POSITION_INVALID;
    s_action_candidate_since_ms = 0;
    s_action_candidate_frames = 0;
    s_action_candidate_budget = 0.0f;
    s_action_candidate_peak = 0.0f;
    s_action_last_valid_ms = 0;
    if (clear_action) {
        s_action_zone_id = POSITION_INVALID;
        s_action_zone_confidence = 0.0f;
    }

    const int anchor = clear_action
                       ? POSITION_INVALID
                       : ((s_action_zone_id >= 1 && s_action_zone_id <= 9) ? s_action_zone_id : POSITION_INVALID);
    s_visual_target_id = anchor;
    s_visual_origin_id = anchor;
    s_visual_blend_start_ms = 0;
    s_visual_blend_duration_ms = VISUAL_EASE_MS_BASE;
    s_visual_zone_id = anchor;
    s_visual_from_zone_id = anchor;
    s_visual_to_zone_id = anchor;
    s_visual_blend_t = 1.0f;
    s_visual_zone_confidence = 0.0f;
    s_action_rate = 0.0f;
    s_action_rate_ts = 0;
    audio_gate_reset();
    if (clear_action) {
        s_audio_last_played_zone = POSITION_INVALID;
        s_audio_last_played_ms = 0;
    }
}

static float action_rate_decayed(uint32_t now_ms)
{
    float rate = s_action_rate;
    if (rate <= 0.0f || s_action_rate_ts == 0) {
        return 0.0f;
    }
    const uint32_t dt = now_ms - s_action_rate_ts;
    rate -= (float)dt / VISUAL_RATE_LEAK_MS;
    if (rate < 0.0f) {
        rate = 0.0f;
    }
    return rate;
}

static float action_rate_to_speed(float rate)
{
    const float span = fmaxf(0.001f, VISUAL_RATE_RED - VISUAL_RATE_BLUE);
    return clamp01f((rate - VISUAL_RATE_BLUE) / span);
}

static void update_visual_speed_by_action_change(bool action_changed, uint32_t now_ms)
{
    if (!action_changed) {
        return;
    }
    if (s_action_zone_id < 1 || s_action_zone_id > 9) {
        return;
    }

    // 每次 action 切换给“速度漏桶”加一个脉冲，桶值随时间泄漏。
    // 切换越密集 -> 桶值越高 -> 颜色越偏红，并自然撑住一段时间再退回蓝。
    float rate = action_rate_decayed(now_ms) + VISUAL_RATE_IMPULSE;
    if (rate > VISUAL_RATE_MAX) {
        rate = VISUAL_RATE_MAX;
    }
    s_action_rate = rate;
    s_action_rate_ts = now_ms;

    ESP_LOGD(TAG, "Visual speed by action: rate=%.2f speed=%.3f action=%d", (double)rate,
             (double)action_rate_to_speed(rate), s_action_zone_id);
}

static void update_motion_profile(float r_xy, float theta_deg, float dz, uint32_t now_ms)
{
    if (!s_motion_has_prev) {
        s_motion_prev_r = r_xy;
        s_motion_prev_theta = theta_deg;
        s_motion_prev_dz = dz;
        s_motion_prev_ts = now_ms;
        s_motion_has_prev = true;
        return;
    }

    uint32_t dt_ms = now_ms - s_motion_prev_ts;
    if (dt_ms == 0) {
        dt_ms = 1;
    }

    const float dr = fabsf(r_xy - s_motion_prev_r);
    const float dth = fabsf(wrap_angle_delta_deg(theta_deg - s_motion_prev_theta));
    const float ddz = fabsf(dz - s_motion_prev_dz);
    const float raw_speed = dr + MOTION_THETA_WEIGHT * dth + MOTION_DZ_WEIGHT * ddz;
    const float dt_norm = 20.0f / fmaxf((float)dt_ms, 5.0f);
    const float motion_metric = raw_speed * dt_norm;
    if (s_motion_metric_ema <= 0.0f) {
        s_motion_metric_ema = motion_metric;
    } else {
        s_motion_metric_ema = MOTION_EMA_ALPHA * motion_metric + (1.0f - MOTION_EMA_ALPHA) * s_motion_metric_ema;
    }

    debounce_profile_t next = s_motion_profile;
    if (s_motion_profile == DEBOUNCE_PROFILE_SLIDING) {
        if (s_motion_metric_ema <= MOTION_SLIDE_EXIT) {
            next = DEBOUNCE_PROFILE_STILL;
        }
    } else if (s_motion_metric_ema >= MOTION_SLIDE_ENTER) {
        next = DEBOUNCE_PROFILE_SLIDING;
    }
    if (next != s_motion_profile) {
        s_motion_profile = next;
        ESP_LOGI(TAG, "Debounce profile -> %s (motion=%.1f)", debounce_profile_name(next), s_motion_metric_ema);
    }

    s_motion_prev_r = r_xy;
    s_motion_prev_theta = theta_deg;
    s_motion_prev_dz = dz;
    s_motion_prev_ts = now_ms;
}

static void apply_motion_profile_to_switch_requirements(int *frames, float *budget)
{
    if (s_motion_profile == DEBOUNCE_PROFILE_SLIDING) {
        *budget *= 0.95f;
        if (*budget < 0.35f) {
            *budget = 0.35f;
        }
        return;
    }

    (*frames)++;
    *budget += 0.20f;
}

static float infer_locked_zone_confidence(int best_grid_id, float best_confidence, int locked_id,
                                          float locked_confidence)
{
    if (locked_id < 1 || locked_id > 9) {
        return 0.0f;
    }
    if (best_grid_id == locked_id) {
        return fmaxf(best_confidence, 0.0f);
    }
    return fmaxf(locked_confidence, 0.0f);
}

static void select_action_candidate(int best_grid_id, float best_confidence, float locked_confidence,
                                    int *candidate_zone_id_out, float *candidate_conf_out)
{
    int candidate_zone_id = POSITION_INVALID;
    float candidate_conf = 0.0f;

    const bool corner_best = is_corner_zone(best_grid_id);
    const float best_conf_floor =
        corner_best ? (ZONE_ACTIVE_THRESHOLD * CORNER_ACTION_CONF_FLOOR_SCALE) : ZONE_ACTIVE_THRESHOLD;

    if (best_grid_id >= 1 && best_grid_id <= 9 && best_confidence >= best_conf_floor) {
        candidate_zone_id = best_grid_id;
        candidate_conf = fmaxf(best_confidence, 0.0f);
        if (corner_best) {
            candidate_conf += CORNER_ACTION_CONF_BOOST;
        }
    } else if (last_stable_id >= 1 && last_stable_id <= 9 && locked_confidence > ZONE_LOCK_RELEASE_CONF) {
        /* best 候选短暂掉帧时，允许上一稳定点位兜底，避免动作态抖成 invalid */
        candidate_zone_id = last_stable_id;
        candidate_conf = fmaxf(locked_confidence * 0.90f, 0.0f);
    }

    *candidate_zone_id_out = candidate_zone_id;
    *candidate_conf_out = candidate_conf;
}

static bool update_action_zone(int candidate_zone_id, float candidate_confidence, uint32_t now_ms)
{
    if (candidate_zone_id < 1 || candidate_zone_id > 9) {
        s_action_candidate_id = POSITION_INVALID;
        s_action_candidate_frames = 0;
        s_action_candidate_budget = 0.0f;
        s_action_candidate_peak = 0.0f;

        if (s_action_zone_id >= 1 && s_action_last_valid_ms > 0 &&
                (now_ms - s_action_last_valid_ms) >= ACTION_RELEASE_MS) {
            s_action_zone_id = POSITION_INVALID;
            s_action_zone_confidence = 0.0f;
            return true;
        }
        return false;
    }

    const float conf = fmaxf(candidate_confidence, 0.0f);
    s_action_last_valid_ms = now_ms;

    if (candidate_zone_id == s_action_zone_id) {
        s_action_zone_confidence = conf;
        s_action_candidate_id = candidate_zone_id;
        s_action_candidate_since_ms = now_ms;
        s_action_candidate_frames = 0;
        s_action_candidate_budget = 0.0f;
        s_action_candidate_peak = 0.0f;
        return false;
    }

    if (candidate_zone_id != s_action_candidate_id) {
        s_action_candidate_id = candidate_zone_id;
        s_action_candidate_since_ms = now_ms;
        s_action_candidate_frames = 1;
        s_action_candidate_budget = conf;
        s_action_candidate_peak = conf;
    } else {
        s_action_candidate_frames++;
        s_action_candidate_budget += conf;
        s_action_candidate_peak = fmaxf(s_action_candidate_peak, conf);
    }

    int required_frames = ACTION_CONFIRM_FRAMES;
    int required_hold_ms = (int)ACTION_MIN_HOLD_MS;
    float required_conf = ACTION_MIN_CONF;
    const bool corner_candidate = is_corner_zone(candidate_zone_id);
    if (s_motion_profile == DEBOUNCE_PROFILE_SLIDING) {
        if (required_frames > 1) {
            required_frames--;
        }
        required_hold_ms = 18;
        required_conf = fmaxf(0.10f, required_conf - 0.02f);
    } else {
        required_hold_ms += 15;
        required_conf += 0.01f;
    }
    float required_budget = required_conf * (float)required_frames;
    if (corner_candidate) {
        required_conf = fmaxf(0.085f, required_conf - 0.015f);
        required_hold_ms -= CORNER_ACTION_HOLD_REDUCE_MS;
        if (required_hold_ms < 10) {
            required_hold_ms = 10;
        }
        required_budget = fmaxf(0.08f, required_budget * CORNER_ACTION_BUDGET_SCALE);
        if (s_motion_profile == DEBOUNCE_PROFILE_SLIDING && required_frames > 1) {
            required_frames--;
        }
    }
    if (s_action_zone_id >= 1 && candidate_zone_id != s_action_zone_id &&
            zone_grid_distance(s_action_zone_id, candidate_zone_id) >= 2 && s_motion_profile == DEBOUNCE_PROFILE_SLIDING &&
            conf >= (required_conf + 0.08f)) {
        if (required_frames < 2) {
            required_frames = 2;
        }
        if (required_hold_ms < 55) {
            required_hold_ms = 55;
        }
        required_budget = fmaxf(0.28f, required_conf * (float)required_frames);
    }
    const uint32_t held_ms = now_ms - s_action_candidate_since_ms;

    if ((int)held_ms >= required_hold_ms && s_action_candidate_frames >= required_frames &&
            s_action_candidate_peak >= required_conf && s_action_candidate_budget >= required_budget) {
        s_action_zone_id = s_action_candidate_id;
        s_action_zone_confidence = s_action_candidate_peak;
        return true;
    }
    return false;
}

static void start_visual_transition(int target_zone_id, uint32_t now_ms)
{
    int origin_zone = s_visual_zone_id;
    if (origin_zone < 1 || origin_zone > 9) {
        origin_zone = (s_action_zone_id >= 1 && s_action_zone_id <= 9) ? s_action_zone_id : POSITION_INVALID;
    }

    s_visual_origin_id = origin_zone;
    s_visual_target_id = target_zone_id;
    s_visual_blend_start_ms = now_ms;
    s_visual_blend_duration_ms =
        (s_motion_profile == DEBOUNCE_PROFILE_SLIDING) ? VISUAL_EASE_MS_FAST : VISUAL_EASE_MS_SLOW;

    s_visual_from_zone_id = origin_zone;
    s_visual_to_zone_id = target_zone_id;
    s_visual_blend_t = (origin_zone == target_zone_id) ? 1.0f : 0.0f;
}

static void update_visual_zone(int best_zone_id, float best_confidence, uint32_t now_ms)
{
    int target_zone_id = POSITION_INVALID;
    if (best_zone_id >= 1 && best_zone_id <= 9 && best_confidence >= VISUAL_CONF_WEAK) {
        target_zone_id = best_zone_id;
    } else if (s_action_zone_id >= 1 && s_action_zone_id <= 9) {
        target_zone_id = s_action_zone_id;
    }

    if (target_zone_id != s_visual_target_id) {
        start_visual_transition(target_zone_id, now_ms);
    }

    float target_conf = (target_zone_id == s_action_zone_id) ? s_action_zone_confidence : best_confidence;
    if (target_conf < 0.0f) {
        target_conf = 0.0f;
    }
    s_visual_zone_confidence = target_conf;

    if (s_visual_origin_id == s_visual_target_id) {
        s_visual_zone_id = s_visual_target_id;
        s_visual_blend_t = 1.0f;
        return;
    }

    const uint32_t elapsed_ms = now_ms - s_visual_blend_start_ms;
    const float t =
        (s_visual_blend_duration_ms > 0) ? fminf(1.0f, (float)elapsed_ms / (float)s_visual_blend_duration_ms) : 1.0f;
    s_visual_blend_t = smoothstepf(t);
    if (t >= 1.0f) {
        s_visual_origin_id = s_visual_target_id;
        s_visual_zone_id = s_visual_target_id;
        s_visual_from_zone_id = s_visual_target_id;
        s_visual_to_zone_id = s_visual_target_id;
        s_visual_blend_t = 1.0f;
    } else {
        s_visual_zone_id = (s_visual_blend_t >= 0.5f) ? s_visual_target_id : s_visual_origin_id;
    }
}

static void process_speaker_toggle_sequence(int zone_id, float zone_conf, uint32_t now_ms)
{
    if (speaker_gesture_match_step(zone_id, zone_conf, now_ms, SPEAKER_TOGGLE_STEP_TIMEOUT_MS,
                                   SPEAKER_TOGGLE_STEP_HOLD_MS, SPEAKER_TOGGLE_MIN_CONF)) {
        const bool next_enabled = !zone_tone_is_enabled();
        zone_tone_set_enabled(next_enabled);
        int seq[SPEAKER_GESTURE_MAX_LEN] = {0};
        const size_t seq_len = speaker_gesture_get_sequence(seq, SPEAKER_GESTURE_MAX_LEN);
        char seq_buf[48] = {0};
        size_t pos = 0;
        for (size_t i = 0; i < seq_len && pos < (sizeof(seq_buf) - 4); i++) {
            const int n = snprintf(seq_buf + pos, sizeof(seq_buf) - pos, "%s%d", (i == 0) ? "" : "-", seq[i]);
            if (n <= 0) {
                break;
            }
            pos += (size_t)n;
        }
        ESP_LOGW(TAG, "Speaker toggled by sequence %s => %s", seq_buf, next_enabled ? "ON" : "OFF");
    }
}

static void update_audio_zone_gate(int zone_id, float zone_conf, bool action_changed, uint32_t now_ms)
{
    const float conf = fmaxf(zone_conf, 0.0f);

    if (s_motion_profile == DEBOUNCE_PROFILE_SLIDING) {
        zone_tone_set_zone(POSITION_INVALID);
        if (audio_zone_is_playable(zone_id)) {
            if (s_audio_gate_state != AUDIO_GATE_SLIDING_SUPPRESSED || s_audio_pending_zone != zone_id) {
                ESP_LOGD(TAG, "Audio gate: sliding suppress zone=%d conf=%.3f", zone_id, conf);
            }
            s_audio_gate_state = AUDIO_GATE_SLIDING_SUPPRESSED;
            if (s_audio_pending_zone != zone_id) {
                s_audio_pending_peak_conf = conf;
            } else {
                s_audio_pending_peak_conf = fmaxf(s_audio_pending_peak_conf, conf);
            }
            s_audio_pending_zone = zone_id;
            s_audio_pending_since_ms = 0;
        } else if (s_audio_gate_state != AUDIO_GATE_IDLE) {
            ESP_LOGD(TAG, "Audio gate: reject zone=%d reason=sliding_invalid", zone_id);
            audio_gate_reset();
        }
        return;
    }

    if (!audio_zone_is_playable(zone_id)) {
        zone_tone_set_zone(POSITION_INVALID);
        if (s_audio_gate_state != AUDIO_GATE_IDLE) {
            ESP_LOGD(TAG, "Audio gate: reject zone=%d reason=invalid_or_center", zone_id);
        }
        audio_gate_reset();
        return;
    }

    if (conf < s_audio_gate_cfg.min_conf) {
        zone_tone_set_zone(POSITION_INVALID);
        if (s_audio_gate_state == AUDIO_GATE_PENDING_SETTLE && s_audio_pending_zone == zone_id) {
            ESP_LOGD(TAG, "Audio gate: reject zone=%d reason=conf conf=%.3f", zone_id, conf);
            audio_gate_reset();
        }
        return;
    }

    if (action_changed || s_audio_pending_zone != zone_id || s_audio_gate_state == AUDIO_GATE_SLIDING_SUPPRESSED ||
            s_audio_gate_state == AUDIO_GATE_IDLE) {
        zone_tone_set_zone(POSITION_INVALID);
        s_audio_gate_state = AUDIO_GATE_PENDING_SETTLE;
        s_audio_pending_zone = zone_id;
        s_audio_pending_since_ms = now_ms;
        s_audio_pending_peak_conf = conf;
        ESP_LOGD(TAG, "Audio gate: pending settle zone=%d conf=%.3f", zone_id, conf);
        return;
    }

    if (s_audio_gate_state == AUDIO_GATE_PLAYED && s_audio_last_played_zone == zone_id) {
        return;
    }

    if (s_audio_gate_state != AUDIO_GATE_PENDING_SETTLE || s_audio_pending_zone != zone_id) {
        return;
    }

    s_audio_pending_peak_conf = fmaxf(s_audio_pending_peak_conf, conf);
    const uint32_t held_ms = now_ms - s_audio_pending_since_ms;
    if (held_ms < s_audio_gate_cfg.settle_hold_ms) {
        return;
    }

    if (s_audio_last_played_zone == zone_id && s_audio_last_played_ms > 0 &&
            (now_ms - s_audio_last_played_ms) < s_audio_gate_cfg.cooldown_ms) {
        ESP_LOGD(TAG, "Audio gate: reject zone=%d reason=cooldown", zone_id);
        s_audio_gate_state = AUDIO_GATE_PLAYED;
        return;
    }

    zone_tone_set_zone(zone_id);
    s_audio_last_played_zone = zone_id;
    s_audio_last_played_ms = now_ms;
    s_audio_gate_state = AUDIO_GATE_PLAYED;
    ESP_LOGD(TAG, "Audio gate: play final zone=%d conf=%.3f hold=%u", zone_id, (double)s_audio_pending_peak_conf,
             (unsigned)held_ms);
}

static void update_zone_output_channels(int best_grid_id, float best_confidence, float locked_confidence, float dx,
                                        float dy, float dz, float r_xy, float theta_deg)
{
    const uint32_t now_ms = esp_log_timestamp();
    update_motion_profile(r_xy, theta_deg, dz, now_ms);

    const float stable_confidence =
        infer_locked_zone_confidence(best_grid_id, best_confidence, last_stable_id, locked_confidence);
    int action_candidate_id = POSITION_INVALID;
    float action_candidate_conf = 0.0f;
    select_action_candidate(best_grid_id, best_confidence, stable_confidence, &action_candidate_id,
                            &action_candidate_conf);
    const bool action_changed = update_action_zone(action_candidate_id, action_candidate_conf, now_ms);
    update_visual_speed_by_action_change(action_changed, now_ms);
    update_visual_zone(best_grid_id, best_confidence, now_ms);
    process_speaker_toggle_sequence(s_action_zone_id, s_action_zone_confidence, now_ms);
    update_audio_zone_gate(s_action_zone_id, s_action_zone_confidence, action_changed, now_ms);

    if (action_changed) {
        ESP_LOGD(TAG, "Action zone -> [%d] (conf=%.3f, visual=[%d] blend=%.2f)", s_action_zone_id,
                 s_action_zone_confidence, s_visual_zone_id, s_visual_blend_t);
        web_portal_push_zone_change(s_action_zone_id, s_action_zone_confidence);
    }

    web_portal_set_action_snapshot(s_action_zone_id, s_action_zone_confidence);
    web_portal_push_mag(dx, dy, dz, s_visual_zone_id, s_visual_zone_confidence, r_xy, theta_deg);
}

static void get_switch_thresholds(int from_id, int to_id, float peak_conf, float conf_gap, int *frames_out,
                                  float *budget_out)
{
    int frames = DEBOUNCE_NORM_FRAMES;
    float budget = DEBOUNCE_NORM_BUDGET;

    if (from_id >= 1 && zones_are_adjacent(from_id, to_id)) {
        frames = DEBOUNCE_ADJ_FRAMES;
        budget = DEBOUNCE_ADJ_BUDGET;
        if (conf_gap < DEBOUNCE_ADJ_CONF_GAP) {
            frames += 2;
            budget += 0.45f;
        } else if (conf_gap >= DEBOUNCE_ADJ_FAST_GAP) {
            frames = 2;
            budget = 0.55f;
        }
    } else if (zone_grid_distance(from_id, to_id) >= 3 || peak_conf < DEBOUNCE_CONF_LOW ||
               conf_gap < DEBOUNCE_FAR_CONF_GAP) {
        frames = DEBOUNCE_FAR_FRAMES;
        budget = DEBOUNCE_FAR_BUDGET;
    } else if (peak_conf >= DEBOUNCE_CONF_HIGH && conf_gap >= DEBOUNCE_FAR_CONF_GAP) {
        frames = DEBOUNCE_NORM_FRAMES;
        budget = DEBOUNCE_NORM_BUDGET * 0.85f;
    }

    apply_motion_profile_to_switch_requirements(&frames, &budget);
    if (is_corner_zone(to_id)) {
        if (frames > 1) {
            frames--;
        }
        budget = fmaxf(0.20f, budget * CORNER_SWITCH_BUDGET_SCALE);
    }

    *frames_out = frames;
    *budget_out = budget;
}

#if !ZONE_DETECT_USE_XYZ /* -------- 极坐标 r/θ 判定 -------- */

static float get_center_r_max(void)
{
    for (int i = 0; i < zone_storage_count(); i++) {
        const polar_zone_t *zone = &zone_storage_get()[i];
        if (zone->id == CENTER_ZONE_ID) {
            return zone->r_max * CENTER_ZONE_RUNTIME_SCALE;
        }
    }
    return 2500.0f * CENTER_ZONE_RUNTIME_SCALE;
}

static void confusion_pair_weights(int id_a, int id_b, float *r_w, float *th_w)
{
    *r_w = 1.0f;
    *th_w = 1.0f;
    if ((id_a == 1 && id_b == 8) || (id_a == 8 && id_b == 1)) {
        *th_w = 6.0f;
    } else if ((id_a == 3 && id_b == 6) || (id_a == 6 && id_b == 3)) {
        *r_w = 3.0f;
        *th_w = 3.0f;
    } else if ((id_a == 6 && id_b == 9) || (id_a == 9 && id_b == 6)) {
        *r_w = 3.5f;
        *th_w = 3.5f;
    }
}

static void promote_confusion_partner_scores(float r_xy, float theta_deg, float dz, int best_id, int *second_id,
                                             float *second_confidence)
{
    for (int i = 0; i < CONFUSION_PAIR_COUNT; i++) {
        int partner = -1;
        if (CONFUSION_PAIRS[i][0] == best_id) {
            partner = CONFUSION_PAIRS[i][1];
        } else if (CONFUSION_PAIRS[i][1] == best_id) {
            partner = CONFUSION_PAIRS[i][0];
        }
        if (partner < 0) {
            continue;
        }
        const polar_zone_t *pz = zone_by_id(partner);
        if (!pz) {
            continue;
        }
        const float partner_conf = zone_polar_confidence_gated(r_xy, theta_deg, dz, pz);
        if (partner_conf > *second_confidence) {
            *second_confidence = partner_conf;
            *second_id = partner;
        }
    }
}

static float zone_weighted_distance_norm(float r_xy, float theta_deg, const polar_zone_t *zone, float r_weight,
                                         float th_weight)
{
    const float r_mid = (zone->r_min + zone->r_max) * 0.5f;
    const float r_half = fmaxf((zone->r_max - zone->r_min) * 0.5f, 1.0f);

    float th_mid;
    float th_half;
    if (zone->th_min <= zone->th_max) {
        th_mid = (zone->th_min + zone->th_max) * 0.5f;
        th_half = fmaxf((zone->th_max - zone->th_min) * 0.5f, 0.5f);
    } else {
        th_half = (360.0f - (zone->th_min - zone->th_max)) * 0.5f;
        th_mid = zone->th_min + th_half;
        if (th_mid > 180.0f) {
            th_mid -= 360.0f;
        }
    }

    const float r_norm = fabsf(r_xy - r_mid) / r_half;
    float th_diff = theta_deg - th_mid;
    if (th_diff > 180.0f) {
        th_diff -= 360.0f;
    }
    if (th_diff < -180.0f) {
        th_diff += 360.0f;
    }
    const float th_norm = fabsf(th_diff) / th_half;
    return sqrtf(r_weight * r_norm * r_norm + th_weight * th_norm * th_norm);
}

static void resolve_confusion_pair_winner(float r_xy, float theta_deg, int *best_id, float *best_confidence,
                                          int *second_id, float *second_confidence)
{
    if (!zones_are_confusion_pair(*best_id, *second_id)) {
        return;
    }
    if ((*best_confidence - *second_confidence) >= CONFUSION_PAIR_WIN_GAP) {
        return;
    }

    const polar_zone_t *zb = zone_by_id(*best_id);
    const polar_zone_t *zs = zone_by_id(*second_id);
    if (!zb || !zs) {
        return;
    }

    float r_w = 1.0f;
    float th_w = 1.0f;
    confusion_pair_weights(*best_id, *second_id, &r_w, &th_w);

    const float dist_b = zone_weighted_distance_norm(r_xy, theta_deg, zb, r_w, th_w);
    const float dist_s = zone_weighted_distance_norm(r_xy, theta_deg, zs, r_w, th_w);
    if (dist_s + 0.08f < dist_b) {
        const int tmp_id = *best_id;
        const float tmp_conf = *best_confidence;
        *best_id = *second_id;
        *best_confidence = *second_confidence;
        *second_id = tmp_id;
        *second_confidence = tmp_conf;
    }
}

static float zone_center_distance_norm(float r_xy, float theta_deg, const polar_zone_t *zone)
{
    const float r_mid = (zone->r_min + zone->r_max) * 0.5f;
    const float r_half = fmaxf((zone->r_max - zone->r_min) * 0.5f, 1.0f);

    float th_mid;
    float th_half;
    if (zone->th_min <= zone->th_max) {
        th_mid = (zone->th_min + zone->th_max) * 0.5f;
        th_half = fmaxf((zone->th_max - zone->th_min) * 0.5f, 0.5f);
    } else {
        th_half = (360.0f - (zone->th_min - zone->th_max)) * 0.5f;
        th_mid = zone->th_min + th_half;
        if (th_mid > 180.0f) {
            th_mid -= 360.0f;
        }
    }

    const float r_norm = fabsf(r_xy - r_mid) / r_half;
    float th_diff = theta_deg - th_mid;
    if (th_diff > 180.0f) {
        th_diff -= 360.0f;
    }
    if (th_diff < -180.0f) {
        th_diff += 360.0f;
    }
    const float th_norm = fabsf(th_diff) / th_half;
    return sqrtf(r_norm * r_norm + th_norm * th_norm);
}

static int find_nearest_zone(float r_xy, float theta_deg)
{
    int nearest_id = POSITION_INVALID;
    float nearest_dist = 1e9f;

    for (int i = 0; i < zone_storage_count(); i++) {
        const polar_zone_t *zone = &zone_storage_get()[i];
        if (zone->id == CENTER_ZONE_ID && r_xy > get_center_r_max()) {
            continue;
        }
        const float dist = zone_center_distance_norm(r_xy, theta_deg, zone);
        if (dist < nearest_dist) {
            nearest_dist = dist;
            nearest_id = zone->id;
        }
    }
    return nearest_id;
}

static int get_position_polar_with_confidence(float r_xy, float theta_deg, float dz, int last_id,
                                              float *best_confidence_out, float *locked_confidence_out)
{
    float best_confidence = 0.0f;
    int best_id = POSITION_INVALID;
    float second_confidence = 0.0f;
    int second_id = POSITION_INVALID;
    float current_zone_confidence = 0.0f;

    for (int i = 0; i < zone_storage_count(); i++) {
        const polar_zone_t *zone = &zone_storage_get()[i];
        float confidence = zone_polar_confidence_gated(r_xy, theta_deg, dz, zone);

        if (confidence > best_confidence) {
            second_confidence = best_confidence;
            second_id = best_id;
            best_confidence = confidence;
            best_id = zone->id;
        } else if (confidence > second_confidence) {
            second_confidence = confidence;
            second_id = zone->id;
        }
        if (zone->id == last_id) {
            current_zone_confidence = confidence;
        }
    }

    /* 易混区对 (1↔8, 3↔6, 6↔9)：强制纳入所有配对对手比较 */
    if (best_id >= 1) {
        promote_confusion_partner_scores(r_xy, theta_deg, dz, best_id, &second_id, &second_confidence);
    }

    resolve_confusion_pair_winner(r_xy, theta_deg, &best_id, &best_confidence, &second_id, &second_confidence);

    /* 仅相距 ≥3 格且 top-2 极接近时，按几何距离重判 */
    if (best_id >= 1 && second_id >= 1 && best_id != second_id && !zones_are_confusion_pair(best_id, second_id) &&
            zone_grid_distance(best_id, second_id) >= 3 && (best_confidence - second_confidence) < FAR_ZONE_WIN_GAP) {
        const polar_zone_t *zb = nullptr;
        const polar_zone_t *zs = nullptr;
        for (int i = 0; i < zone_storage_count(); i++) {
            const polar_zone_t *zone = &zone_storage_get()[i];
            if (zone->id == best_id) {
                zb = zone;
            } else if (zone->id == second_id) {
                zs = zone;
            }
        }
        if (zb && zs) {
            const float dist_b = zone_center_distance_norm(r_xy, theta_deg, zb);
            const float dist_s = zone_center_distance_norm(r_xy, theta_deg, zs);
            if (dist_s + 0.10f < dist_b) {
                const int tmp_id = best_id;
                const float tmp_conf = best_confidence;
                best_id = second_id;
                best_confidence = second_confidence;
                second_id = tmp_id;
                second_confidence = tmp_conf;
            }
        }
    }

    /* 5 号位：低半径区域，外围区未命中时回落到中心区 */
    const float center_r_max = get_center_r_max();
    if (r_xy <= center_r_max && (best_id == POSITION_INVALID || best_confidence <= ZONE_ACTIVE_THRESHOLD)) {
        for (int i = 0; i < zone_storage_count(); i++) {
            const polar_zone_t *zone = &zone_storage_get()[i];
            if (zone->id != CENTER_ZONE_ID) {
                continue;
            }
            float z5_conf = zone_polar_confidence_gated(r_xy, theta_deg, dz, zone);
            best_id = CENTER_ZONE_ID;
            best_confidence = fmaxf(z5_conf, ZONE_ACTIVE_THRESHOLD + 0.05f);
            break;
        }
    }

    /* 动态滞后：相邻区加大门槛，避免边界来回跳变 */
    if (last_id >= 1 && best_id != last_id && current_zone_confidence >= ZONE_ACTIVE_THRESHOLD) {
        float margin = HYSTERESIS_MARGIN_BASE;
        const bool adjacent_switch = zones_are_adjacent(last_id, best_id);
        if (current_zone_confidence > 0.35f) {
            margin += 0.02f;
        } else if (current_zone_confidence < 0.10f && !adjacent_switch) {
            margin = 0.0f;
        }
        if (adjacent_switch) {
            margin += HYSTERESIS_ADJ_EXTRA;
        }
        if (current_zone_confidence > HYSTERESIS_MIN_HOLD && best_confidence < current_zone_confidence + margin) {
            best_id = last_id;
            best_confidence = current_zone_confidence;
        }
    }

    /* 相邻区 top-2 接近时保持当前锁定 */
    if (last_id >= 1 && best_id != last_id && zones_are_adjacent(last_id, best_id) &&
            (best_confidence - current_zone_confidence) < ADJ_CONF_TIE_GAP) {
        best_id = last_id;
        best_confidence = current_zone_confidence;
    } else if (last_id >= 1 && best_id != last_id && second_id == last_id && zones_are_adjacent(last_id, best_id) &&
               (best_confidence - second_confidence) < ADJ_CONF_TIE_GAP) {
        best_id = last_id;
        best_confidence = second_confidence;
    }

    /* 易混区对保持锁定，除非新候选明显更强 */
    if (last_id >= 1 && best_id != last_id && zones_are_confusion_pair(last_id, best_id) &&
            current_zone_confidence >= ZONE_ACTIVE_THRESHOLD &&
            best_confidence < current_zone_confidence + HYSTERESIS_MARGIN_BASE + 0.10f) {
        best_id = last_id;
        best_confidence = current_zone_confidence;
    }

    /* 无候选或新候选不如旧区时，才保持上一锁定穴位 */
    if (last_id >= 1 && current_zone_confidence > ZONE_ACTIVE_THRESHOLD &&
            current_zone_confidence > best_confidence + 0.005f) {
        best_id = last_id;
        best_confidence = current_zone_confidence;
    }

    /* 全区置信度均失效：按几何距离选最近点位；远距跳变需明显更近 */
    if (best_confidence <= ZONE_ACTIVE_THRESHOLD) {
        int near_id = find_nearest_zone(r_xy, theta_deg);
        if (near_id >= 1 && last_id >= 1 && near_id != last_id) {
            const polar_zone_t *last_zone = nullptr;
            const polar_zone_t *near_zone = nullptr;
            for (int i = 0; i < zone_storage_count(); i++) {
                const polar_zone_t *zone = &zone_storage_get()[i];
                if (zone->id == last_id) {
                    last_zone = zone;
                } else if (zone->id == near_id) {
                    near_zone = zone;
                }
            }
            if (last_zone && near_zone) {
                const float dist_last = zone_center_distance_norm(r_xy, theta_deg, last_zone);
                const float dist_near = zone_center_distance_norm(r_xy, theta_deg, near_zone);
                const float margin = (zone_grid_distance(last_id, near_id) >= 3) ? 0.82f : NEAREST_ADJ_MARGIN;
                if (dist_near >= dist_last * margin) {
                    near_id = last_id;
                }
            }
        }
        if (near_id >= 1 && near_id != best_id) {
            best_id = near_id;
            best_confidence = ZONE_ACTIVE_THRESHOLD + 0.02f;
        } else if (near_id >= 1 && best_id == POSITION_INVALID) {
            best_id = near_id;
            best_confidence = ZONE_ACTIVE_THRESHOLD + 0.02f;
        }
    }

    if (best_confidence_out) {
        *best_confidence_out = best_confidence;
    }
    if (locked_confidence_out) {
        *locked_confidence_out = (last_id >= 1) ? current_zone_confidence : 0.0f;
    }

    return best_id;
}

#endif /* polar */

#if ZONE_DETECT_USE_XYZ

static void confusion_pair_weights_xyz(int id_a, int id_b, float *xw, float *yw, float *zw)
{
    *xw = 1.0f;
    *yw = 1.0f;
    *zw = 1.0f;
    if ((id_a == 1 && id_b == 8) || (id_a == 8 && id_b == 1)) {
        *yw = 4.0f;
    } else if ((id_a == 3 && id_b == 6) || (id_a == 6 && id_b == 3)) {
        *xw = 2.5f;
        *zw = 2.5f;
    } else if ((id_a == 6 && id_b == 9) || (id_a == 9 && id_b == 6)) {
        *xw = 3.0f;
        *yw = 3.0f;
        *zw = 3.0f;
    }
}

static float axis_norm(float v, float vmin, float vmax)
{
    const float mid = (vmin + vmax) * 0.5f;
    const float half = fmaxf((vmax - vmin) * 0.5f, 1.0f);
    return fabsf(v - mid) / half;
}

static float calculate_zone_confidence_xyz(float dx, float dy, float dz, const polar_zone_t *zone)
{
    const float x_norm = axis_norm(dx, zone->x_min, zone->x_max);
    const float y_norm = axis_norm(dy, zone->y_min, zone->y_max);
    const float z_norm = axis_norm(dz, zone->z_min, zone->z_max);

    if (x_norm > ZONE_SOFT_EDGE || y_norm > ZONE_SOFT_EDGE || z_norm > ZONE_SOFT_EDGE) {
        return 0.0f;
    }

    const float GAUSSIAN_K = 3.5f;
    const float x_score = expf(-GAUSSIAN_K * x_norm * x_norm);
    const float y_score = expf(-GAUSSIAN_K * y_norm * y_norm);
    const float z_score = expf(-GAUSSIAN_K * z_norm * z_norm);
    float combined_score = cbrtf(x_score * y_score * z_score);
    if (zone->id == 1 || zone->id == 8) {
        combined_score = x_score * y_score * y_score * z_score;
    } else if (zone->id == 3 || zone->id == 6 || zone->id == 9) {
        combined_score = x_score * x_score * y_score * z_score;
    }
    const float weight = 0.95f + 0.05f * (zone->quality_score / 100.0f);
    return combined_score * weight;
}

static void promote_confusion_partner_scores_xyz(float dx, float dy, float dz, int best_id, int *second_id,
                                                 float *second_confidence)
{
    for (int i = 0; i < CONFUSION_PAIR_COUNT; i++) {
        int partner = -1;
        if (CONFUSION_PAIRS[i][0] == best_id) {
            partner = CONFUSION_PAIRS[i][1];
        } else if (CONFUSION_PAIRS[i][1] == best_id) {
            partner = CONFUSION_PAIRS[i][0];
        }
        if (partner < 0) {
            continue;
        }
        const polar_zone_t *pz = zone_by_id(partner);
        if (!pz) {
            continue;
        }
        const float partner_conf = calculate_zone_confidence_xyz(dx, dy, dz, pz);
        if (partner_conf > *second_confidence) {
            *second_confidence = partner_conf;
            *second_id = partner;
        }
    }
}

static float zone_weighted_distance_norm_xyz(float dx, float dy, float dz, const polar_zone_t *zone, float x_weight,
                                             float y_weight, float z_weight)
{
    const float x_norm = axis_norm(dx, zone->x_min, zone->x_max);
    const float y_norm = axis_norm(dy, zone->y_min, zone->y_max);
    const float z_norm = axis_norm(dz, zone->z_min, zone->z_max);
    return sqrtf(x_weight * x_norm * x_norm + y_weight * y_norm * y_norm + z_weight * z_norm * z_norm);
}

static void resolve_confusion_pair_winner_xyz(float dx, float dy, float dz, int *best_id, float *best_confidence,
                                              int *second_id, float *second_confidence)
{
    if (!zones_are_confusion_pair(*best_id, *second_id)) {
        return;
    }
    if ((*best_confidence - *second_confidence) >= CONFUSION_PAIR_WIN_GAP) {
        return;
    }

    const polar_zone_t *zb = zone_by_id(*best_id);
    const polar_zone_t *zs = zone_by_id(*second_id);
    if (!zb || !zs) {
        return;
    }

    float xw = 1.0f;
    float yw = 1.0f;
    float zw = 1.0f;
    confusion_pair_weights_xyz(*best_id, *second_id, &xw, &yw, &zw);

    const float dist_b = zone_weighted_distance_norm_xyz(dx, dy, dz, zb, xw, yw, zw);
    const float dist_s = zone_weighted_distance_norm_xyz(dx, dy, dz, zs, xw, yw, zw);
    if (dist_s + 0.08f < dist_b) {
        const int tmp_id = *best_id;
        const float tmp_conf = *best_confidence;
        *best_id = *second_id;
        *best_confidence = *second_confidence;
        *second_id = tmp_id;
        *second_confidence = tmp_conf;
    }
}

static float zone_center_distance_norm_xyz(float dx, float dy, float dz, const polar_zone_t *zone)
{
    const float x_norm = axis_norm(dx, zone->x_min, zone->x_max);
    const float y_norm = axis_norm(dy, zone->y_min, zone->y_max);
    const float z_norm = axis_norm(dz, zone->z_min, zone->z_max);
    return sqrtf(x_norm * x_norm + y_norm * y_norm + z_norm * z_norm);
}

static int find_nearest_zone_xyz(float dx, float dy, float dz)
{
    int nearest_id = POSITION_INVALID;
    float nearest_dist = 1e9f;

    for (int i = 0; i < zone_storage_count(); i++) {
        const polar_zone_t *zone = &zone_storage_get()[i];
        const float dist = zone_center_distance_norm_xyz(dx, dy, dz, zone);
        if (dist < nearest_dist) {
            nearest_dist = dist;
            nearest_id = zone->id;
        }
    }
    return nearest_id;
}

static bool point_in_center_xyz(float dx, float dy, float dz)
{
    const polar_zone_t *z5 = zone_by_id(CENTER_ZONE_ID);
    if (!z5) {
        return false;
    }
    return dx >= z5->x_min && dx <= z5->x_max && dy >= z5->y_min && dy <= z5->y_max && dz >= z5->z_min &&
           dz <= z5->z_max;
}

static int get_position_xyz_with_confidence(float dx, float dy, float dz, int last_id, float *best_confidence_out,
                                            float *locked_confidence_out)
{
    float best_confidence = 0.0f;
    int best_id = POSITION_INVALID;
    float second_confidence = 0.0f;
    int second_id = POSITION_INVALID;
    float current_zone_confidence = 0.0f;

    for (int i = 0; i < zone_storage_count(); i++) {
        const polar_zone_t *zone = &zone_storage_get()[i];
        const float confidence = calculate_zone_confidence_xyz(dx, dy, dz, zone);

        if (confidence > best_confidence) {
            second_confidence = best_confidence;
            second_id = best_id;
            best_confidence = confidence;
            best_id = zone->id;
        } else if (confidence > second_confidence) {
            second_confidence = confidence;
            second_id = zone->id;
        }
        if (zone->id == last_id) {
            current_zone_confidence = confidence;
        }
    }

    if (best_id >= 1) {
        promote_confusion_partner_scores_xyz(dx, dy, dz, best_id, &second_id, &second_confidence);
    }

    resolve_confusion_pair_winner_xyz(dx, dy, dz, &best_id, &best_confidence, &second_id, &second_confidence);

    if (best_id >= 1 && second_id >= 1 && best_id != second_id && !zones_are_confusion_pair(best_id, second_id) &&
            zone_grid_distance(best_id, second_id) >= 3 && (best_confidence - second_confidence) < FAR_ZONE_WIN_GAP) {
        const polar_zone_t *zb = zone_by_id(best_id);
        const polar_zone_t *zs = zone_by_id(second_id);
        if (zb && zs) {
            const float dist_b = zone_center_distance_norm_xyz(dx, dy, dz, zb);
            const float dist_s = zone_center_distance_norm_xyz(dx, dy, dz, zs);
            if (dist_s + 0.10f < dist_b) {
                const int tmp_id = best_id;
                const float tmp_conf = best_confidence;
                best_id = second_id;
                best_confidence = second_confidence;
                second_id = tmp_id;
                second_confidence = tmp_conf;
            }
        }
    }

    if (point_in_center_xyz(dx, dy, dz) && (best_id == POSITION_INVALID || best_confidence <= ZONE_ACTIVE_THRESHOLD)) {
        const polar_zone_t *z5 = zone_by_id(CENTER_ZONE_ID);
        if (z5) {
            const float z5_conf = calculate_zone_confidence_xyz(dx, dy, dz, z5);
            best_id = CENTER_ZONE_ID;
            best_confidence = fmaxf(z5_conf, ZONE_ACTIVE_THRESHOLD + 0.05f);
        }
    }

    if (last_id >= 1 && best_id != last_id && current_zone_confidence >= ZONE_ACTIVE_THRESHOLD) {
        float margin = HYSTERESIS_MARGIN_BASE;
        const bool adjacent_switch = zones_are_adjacent(last_id, best_id);
        if (current_zone_confidence > 0.35f) {
            margin += 0.02f;
        } else if (current_zone_confidence < 0.10f && !adjacent_switch) {
            margin = 0.0f;
        }
        if (adjacent_switch) {
            margin += HYSTERESIS_ADJ_EXTRA;
        }
        if (current_zone_confidence > HYSTERESIS_MIN_HOLD && best_confidence < current_zone_confidence + margin) {
            best_id = last_id;
            best_confidence = current_zone_confidence;
        }
    }

    if (last_id >= 1 && best_id != last_id && zones_are_adjacent(last_id, best_id) &&
            (best_confidence - current_zone_confidence) < ADJ_CONF_TIE_GAP) {
        best_id = last_id;
        best_confidence = current_zone_confidence;
    } else if (last_id >= 1 && best_id != last_id && second_id == last_id && zones_are_adjacent(last_id, best_id) &&
               (best_confidence - second_confidence) < ADJ_CONF_TIE_GAP) {
        best_id = last_id;
        best_confidence = second_confidence;
    }

    if (last_id >= 1 && best_id != last_id && zones_are_confusion_pair(last_id, best_id) &&
            current_zone_confidence >= ZONE_ACTIVE_THRESHOLD &&
            best_confidence < current_zone_confidence + HYSTERESIS_MARGIN_BASE + 0.10f) {
        best_id = last_id;
        best_confidence = current_zone_confidence;
    }

    if (last_id >= 1 && current_zone_confidence > ZONE_ACTIVE_THRESHOLD &&
            current_zone_confidence > best_confidence + 0.005f) {
        best_id = last_id;
        best_confidence = current_zone_confidence;
    }

    if (best_confidence <= ZONE_ACTIVE_THRESHOLD) {
        int near_id = find_nearest_zone_xyz(dx, dy, dz);
        if (near_id >= 1 && last_id >= 1 && near_id != last_id) {
            const polar_zone_t *last_zone = zone_by_id(last_id);
            const polar_zone_t *near_zone = zone_by_id(near_id);
            if (last_zone && near_zone) {
                const float dist_last = zone_center_distance_norm_xyz(dx, dy, dz, last_zone);
                const float dist_near = zone_center_distance_norm_xyz(dx, dy, dz, near_zone);
                const float margin = (zone_grid_distance(last_id, near_id) >= 3) ? 0.82f : NEAREST_ADJ_MARGIN;
                if (dist_near >= dist_last * margin) {
                    near_id = last_id;
                }
            }
        }
        if (near_id >= 1 && near_id != best_id) {
            best_id = near_id;
            best_confidence = ZONE_ACTIVE_THRESHOLD + 0.02f;
        } else if (near_id >= 1 && best_id == POSITION_INVALID) {
            best_id = near_id;
            best_confidence = ZONE_ACTIVE_THRESHOLD + 0.02f;
        }
    }

    if (best_confidence_out) {
        *best_confidence_out = best_confidence;
    }
    if (locked_confidence_out) {
        *locked_confidence_out = (last_id >= 1) ? current_zone_confidence : 0.0f;
    }

    return best_id;
}

#endif /* ZONE_DETECT_USE_XYZ */

/* 置信度预算消抖：候选稳定 → 相邻快切 / 远距慢切；间隙保持上一穴 */

/** 旧锁定区置信≈0 且新候选明显更强时，强制释放 stale lock */
static bool try_force_stale_lock_switch(int best_id, float best_conf, float locked_conf, int *last_stable_id)
{
    static int streak = 0;

    if (*last_stable_id < 1 || best_id < 1 || best_id == *last_stable_id || locked_conf > ZONE_LOCK_RELEASE_CONF ||
            best_conf < DEBOUNCE_CONF_HIGH) {
        streak = 0;
        return false;
    }

    streak++;
    if (streak < STALE_LOCK_SWITCH_FRAMES) {
        return false;
    }
    streak = 0;

    ESP_LOGI(TAG, "Stale lock switch [%d]->[%d] (lock_conf=%.3f, new_conf=%.3f)", *last_stable_id, best_id, locked_conf,
             best_conf);
    position_debounce_reset();
    *last_stable_id = best_id;
    return true;
}

static bool try_confirm_position_change(int best_grid_id, float confidence, float locked_zone_confidence,
                                        int *last_stable_id)
{
    static int target_candidate = POSITION_INVALID;
    static int stable_candidate = POSITION_INVALID;
    static int stable_streak = 0;
    static int confirm_frames = 0;
    static float confidence_budget = 0.0f;
    static float confidence_min = 0.0f;
    static float confidence_peak = 0.0f;
    static float confidence_ema = 0.0f;
    static int prev_locked_id = POSITION_INVALID;

    if (s_position_debounce_reset) {
        target_candidate = POSITION_INVALID;
        stable_candidate = POSITION_INVALID;
        stable_streak = 0;
        confirm_frames = 0;
        confidence_budget = 0.0f;
        confidence_min = 0.0f;
        confidence_peak = 0.0f;
        confidence_ema = 0.0f;
        s_position_debounce_reset = false;
    }

    if (best_grid_id < 1 || best_grid_id > 9) {
        return false;
    }

    /* 置信度 EMA，滤除单帧噪声 */
    if (*last_stable_id == best_grid_id && best_grid_id == target_candidate) {
        confidence_ema = confidence;
    } else {
        confidence_ema = CONF_EMA_ALPHA * confidence + (1.0f - CONF_EMA_ALPHA) * confidence_ema;
    }
    const float conf = confidence_ema;

    if (best_grid_id == *last_stable_id) {
        if (conf > ZONE_LOCK_RELEASE_CONF && locked_zone_confidence > ZONE_LOCK_RELEASE_CONF) {
            target_candidate = best_grid_id;
            stable_candidate = best_grid_id;
            stable_streak = DEBOUNCE_STABLE_FRAMES;
            confirm_frames = 0;
            confidence_budget = 0.0f;
            confidence_min = 0.0f;
            confidence_peak = 0.0f;
        }
        return false;
    }

    /* 候选需连续稳定若干帧，才进入确认流程（抗单帧误触） */
    const bool stable_adjacent = (*last_stable_id >= 1 && zones_are_adjacent(*last_stable_id, best_grid_id));
    const int stable_required = stable_adjacent ? DEBOUNCE_ADJ_STABLE : DEBOUNCE_STABLE_FRAMES;
    int stable_required_dyn = stable_required;
    if (s_motion_profile == DEBOUNCE_PROFILE_SLIDING && stable_adjacent) {
        if (stable_required_dyn > 1) {
            stable_required_dyn--;
        }
    } else {
        stable_required_dyn++;
    }

    if (best_grid_id != stable_candidate) {
        stable_candidate = best_grid_id;
        stable_streak = 1;
    } else if (stable_streak < stable_required_dyn) {
        stable_streak++;
    }

    if (stable_streak < stable_required_dyn) {
        return false;
    }

    if (*last_stable_id >= 1 && best_grid_id != *last_stable_id && locked_zone_confidence <= ZONE_LOCK_RELEASE_CONF &&
            confidence >= DEBOUNCE_CONF_HIGH && s_motion_profile != DEBOUNCE_PROFILE_SLIDING) {
        prev_locked_id = *last_stable_id;
        *last_stable_id = best_grid_id;
        target_candidate = best_grid_id;
        stable_candidate = best_grid_id;
        stable_streak = stable_required_dyn;
        confirm_frames = 0;
        confidence_budget = 0.0f;
        confidence_min = 0.0f;
        confidence_peak = confidence;
        confidence_ema = confidence;
        return true;
    }

    if (best_grid_id != target_candidate) {
        /*  leaky：短暂抖动不完全清零，减少来回卡顿 */
        if (target_candidate != POSITION_INVALID && confirm_frames > 0) {
            confirm_frames = confirm_frames / 2;
            confidence_budget *= 0.45f;
        } else {
            confirm_frames = 0;
            confidence_budget = 0.0f;
        }
        target_candidate = best_grid_id;
        confidence_min = conf;
        confidence_peak = conf;
    }

    confirm_frames++;
    confidence_budget += conf;
    confidence_min = fminf(confidence_min, conf);
    confidence_peak = fmaxf(confidence_peak, conf);

    const float conf_gap = confidence_peak - fmaxf(locked_zone_confidence, 0.0f);

    int required_frames = DEBOUNCE_NORM_FRAMES;
    float required_budget = DEBOUNCE_NORM_BUDGET;
    get_switch_thresholds(*last_stable_id, target_candidate, confidence_peak, conf_gap, &required_frames,
                          &required_budget);

    /* 刚离开的穴位又回来：加长确认，防 ping-pong */
    if (target_candidate == prev_locked_id && target_candidate != *last_stable_id) {
        required_frames += DEBOUNCE_PINGPONG_EXTRA;
        required_budget += 0.55f;
    }

    /* 易混区对 (1↔8, 3↔6, 6↔9) 切换加长确认 */
    if (*last_stable_id >= 1 && zones_are_confusion_pair(*last_stable_id, target_candidate)) {
        required_frames += 2;
        required_budget += 0.55f;
        if (conf_gap < 0.12f) {
            return false;
        }
    }

    const bool adjacent_switch = (*last_stable_id >= 1 && zones_are_adjacent(*last_stable_id, target_candidate));
    const int switch_grid_dist = (*last_stable_id >= 1) ? zone_grid_distance(*last_stable_id, target_candidate) : 0;

    /* 远距切换略加长确认，但不硬阻断 */
    if (switch_grid_dist >= 3) {
        required_frames += 2;
        required_budget += 0.55f;
    } else if (switch_grid_dist >= 2) {
        required_frames += 1;
        required_budget += 0.30f;
    }

    /* 旧区置信已很低：非相邻切换可加速（相邻仍完整消抖） */
    if (locked_zone_confidence < ZONE_ACTIVE_THRESHOLD && !adjacent_switch) {
        if (s_motion_profile == DEBOUNCE_PROFILE_SLIDING) {
            if (required_frames < 2) {
                required_frames = 2;
            }
            required_budget = fmaxf(required_budget, 0.60f);
        } else {
            required_frames = 1;
            required_budget *= 0.55f;
            if (required_budget < 0.25f) {
                required_budget = 0.25f;
            }
        }
    }

    if (locked_zone_confidence <= ZONE_LOCK_RELEASE_CONF && confidence_min <= ZONE_LOCK_RELEASE_CONF) {
        if (adjacent_switch) {
            required_frames = fmaxf(required_frames, 2);
            required_budget = fmaxf(required_budget, 0.55f);
        } else if (switch_grid_dist >= 3) {
            required_frames = fmaxf(required_frames, 2);
            required_budget = fmaxf(required_budget, 0.60f);
        } else {
            required_frames = 1;
            required_budget = 0.20f;
        }
        confidence_min = fmaxf(confidence_min, 0.01f);
    }

    if (confirm_frames >= required_frames && confidence_budget >= required_budget &&
            confidence_min >= DEBOUNCE_MIN_STREAK) {
        prev_locked_id = *last_stable_id;
        *last_stable_id = target_candidate;
        confirm_frames = 0;
        confidence_budget = 0.0f;
        confidence_min = 0.0f;
        confidence_peak = 0.0f;
        confidence_ema = conf;
        return true;
    }

    return false;
}

/* --------------------------------------------------------------------------
 * LED strip
 * -------------------------------------------------------------------------- */

#define LED_STRIP_MAX_LEDS 28
#define LED_DIM_LEVEL 4
#define LED_SPIN_LEVEL 28
#define LED_AMBER_DIM_R 8
#define LED_AMBER_DIM_G 3
#define LED_AMBER_DIM_B 0
#define LED_RING_BRIGHT 255
#define LED_RING_OUTER_MAX 10
#define LED_RING_FALLOFF (LED_STRIP_MAX_LEDS / 2)
#define LED_ZONE_CYAN_R 0
#define LED_ZONE_CYAN_G 170
#define LED_ZONE_CYAN_B 255
#define LED_ZONE_SKY_R 0
#define LED_ZONE_SKY_G 235
#define LED_ZONE_SKY_B 210
#define LED_ZONE_ORANGE_R 255
#define LED_ZONE_ORANGE_G 120
#define LED_ZONE_ORANGE_B 0
#define LED_ZONE_DEEP_ORANGE_R 255
#define LED_ZONE_DEEP_ORANGE_G 55
#define LED_ZONE_DEEP_ORANGE_B 0
#define LED_ZONE_RED_R 255
#define LED_ZONE_RED_G 0
#define LED_ZONE_RED_B 0

#define IMU_INT_PIN GPIO_NUM_0

static bmi270_handle_t s_bmi_handle = NULL;
static volatile bool s_recalib_requested = false;

/** 外部（串口 RECALIB 命令）请求重新进行 9 点校准；主循环在下一拍消费 */
extern "C" void app_request_recalibration(void)
{
    s_recalib_requested = true;
}

// LED index for each grid position (index = position id, value = LED index)
static const int grid_to_led_center[10] = {
    -1, // 0: None
    2,  // 1: 对应 Top Left (左上)
    24, // 2: 对应 Top Center (正上，高亮区见 zone2_bright_leds)
    23, // 3: 对应 Top Right (右上)
    5,  // 4: 对应 Middle Left (正左)
    -1, // 5: Center (中心不变)
    20, // 6: 对应 Middle Right (正右)
    9,  // 7: 对应 Bottom Left (左下)
    13, // 8: 对应 Bottom Center (正下)
    16, // 9: 对应 Bottom Right (右下)
};

static led_strip_handle_t s_led_strip;

static int wrap_led_idx(int idx)
{
    return (idx % LED_STRIP_MAX_LEDS + LED_STRIP_MAX_LEDS) % LED_STRIP_MAX_LEDS;
}

static int zone_get_led_offsets(int pt, int8_t *offsets, int max_count)
{
    static const int8_t corner[] = {-1, 0, 1};
    static const int8_t edge_fwd[] = {-1, 0, 1, 2};  /* 4: 正左 */
    static const int8_t edge_bwd[] = {-2, -1, 0, 1}; /* 6/8: 向 - 侧多 1 颗 */

    const int8_t *src = corner;
    int count = 3;

    if (pt == 4) {
        src = edge_fwd;
        count = 4;
    } else if (pt == 6 || pt == 8) {
        src = edge_bwd;
        count = 4;
    }

    if (count > max_count) {
        count = max_count;
    }
    for (int i = 0; i < count; i++) {
        offsets[i] = src[i];
    }
    return count;
}

static int zone_get_bright_leds(int pt, int *leds, int max_count)
{
    static const int zone2_bright_leds[] = {0, 27, 26, 25};

    if (pt == 2) {
        const int count = 4;
        for (int i = 0; i < count && i < max_count; i++) {
            leds[i] = zone2_bright_leds[i];
        }
        return count;
    }

    const int origin = grid_to_led_center[pt];
    if (origin < 0) {
        return 0;
    }

    int8_t offsets[6];
    const int count = zone_get_led_offsets(pt, offsets, 6);
    for (int i = 0; i < count && i < max_count; i++) {
        leds[i] = wrap_led_idx(origin + offsets[i]);
    }
    return count;
}

static int ring_step_dist(int a, int b)
{
    const int d = abs(a - b);
    return (d < LED_STRIP_MAX_LEDS - d) ? d : (LED_STRIP_MAX_LEDS - d);
}

static int ring_dist_to_bright_zone(int led_idx, int pt)
{
    int bright_leds[6];
    const int count = zone_get_bright_leds(pt, bright_leds, 6);
    int min_dist = LED_STRIP_MAX_LEDS;

    for (int i = 0; i < count; i++) {
        const int d = ring_step_dist(led_idx, bright_leds[i]);
        if (d < min_dist) {
            min_dist = d;
        }
    }
    return min_dist;
}

/* 高亮区全亮，区外第一颗即大幅压暗，再沿环二次方渐灭至对侧 */
static uint8_t led_ring_brightness_scale(int dist)
{
    if (dist == 0) {
        return LED_RING_BRIGHT;
    }
    if (dist >= LED_RING_FALLOFF) {
        return 0;
    }

    const int fall_steps = LED_RING_FALLOFF - 1;
    const int d = dist - 1;
    const int remain = fall_steps - d;
    if (remain <= 0) {
        return 0;
    }

    const uint32_t t = (uint32_t)remain * (uint32_t)remain;
    const uint32_t denom = (uint32_t)fall_steps * (uint32_t)fall_steps;
    return (uint8_t)((LED_RING_OUTER_MAX * t) / denom);
}

static void led_set_pixel_scaled(int idx, uint8_t r, uint8_t g, uint8_t b, uint8_t scale)
{
    idx = wrap_led_idx(idx);
    led_strip_set_pixel(s_led_strip, idx, (uint8_t)((r * scale) / 255), (uint8_t)((g * scale) / 255),
                        (uint8_t)((b * scale) / 255));
}

static void led_fill_all(uint8_t r, uint8_t g, uint8_t b)
{
    for (int i = 0; i < LED_STRIP_MAX_LEDS; i++) {
        led_strip_set_pixel(s_led_strip, i, r, g, b);
    }
}

static void led_render_zone_ring(int pt, uint8_t r, uint8_t g, uint8_t b, bool calib_amber_mode, uint8_t dim_r,
                                 uint8_t dim_g, uint8_t dim_b)
{
    if (pt < 1 || pt > 9) {
        return;
    }

    if (pt == 5) {
        led_fill_all(r, g, b);
        return;
    }

    const int origin = grid_to_led_center[pt];
    if (origin < 0 && pt != 2) {
        return;
    }

    for (int i = 0; i < LED_STRIP_MAX_LEDS; i++) {
        const int dist = ring_dist_to_bright_zone(i, pt);
        if (calib_amber_mode) {
            if (dist == 0) {
                led_strip_set_pixel(s_led_strip, i, r, g, b);
            } else {
                led_strip_set_pixel(s_led_strip, i, dim_r, dim_g, dim_b);
            }
            continue;
        }

        const uint8_t scale = led_ring_brightness_scale(dist);
        led_set_pixel_scaled(i, r, g, b, scale);
    }
}

volatile bool is_going_to_sleep = false;

static void hsv_to_rgb(uint8_t h, uint8_t s, uint8_t v, uint8_t *r, uint8_t *g, uint8_t *b)
{
    if (s == 0) {
        *r = *g = *b = v;
        return;
    }
    uint8_t region = h / 43;
    uint8_t remainder = (h - (region * 43)) * 6;
    uint8_t p = (v * (255 - s)) >> 8;
    uint8_t q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    uint8_t t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

    switch (region) {
    case 0:
        *r = v;
        *g = t;
        *b = p;
        break;
    case 1:
        *r = q;
        *g = v;
        *b = p;
        break;
    case 2:
        *r = p;
        *g = v;
        *b = t;
        break;
    case 3:
        *r = p;
        *g = q;
        *b = v;
        break;
    case 4:
        *r = t;
        *g = p;
        *b = v;
        break;
    default:
        *r = v;
        *g = p;
        *b = q;
        break;
    }
}

static void led_add_zone_ring_to_accum(int pt, uint8_t r, uint8_t g, uint8_t b, float gain, uint16_t *acc_r,
                                       uint16_t *acc_g, uint16_t *acc_b)
{
    if (pt < 1 || pt > 9 || gain <= 0.001f) {
        return;
    }
    gain = clamp01f(gain);
    if (pt == 5) {
        for (int i = 0; i < LED_STRIP_MAX_LEDS; i++) {
            acc_r[i] += (uint16_t)(r * gain);
            acc_g[i] += (uint16_t)(g * gain);
            acc_b[i] += (uint16_t)(b * gain);
        }
        return;
    }

    for (int i = 0; i < LED_STRIP_MAX_LEDS; i++) {
        const int dist = ring_dist_to_bright_zone(i, pt);
        const float scale = ((float)led_ring_brightness_scale(dist) / 255.0f) * gain;
        if (scale <= 0.0001f) {
            continue;
        }
        acc_r[i] += (uint16_t)(r * scale);
        acc_g[i] += (uint16_t)(g * scale);
        acc_b[i] += (uint16_t)(b * scale);
    }
}

static void led_commit_accum(const uint16_t *acc_r, const uint16_t *acc_g, const uint16_t *acc_b)
{
    for (int i = 0; i < LED_STRIP_MAX_LEDS; i++) {
        const uint8_t r = (acc_r[i] > 255U) ? 255U : (uint8_t)acc_r[i];
        const uint8_t g = (acc_g[i] > 255U) ? 255U : (uint8_t)acc_g[i];
        const uint8_t b = (acc_b[i] > 255U) ? 255U : (uint8_t)acc_b[i];
        led_strip_set_pixel(s_led_strip, i, r, g, b);
    }
}

static uint8_t led_lerp_u8(uint8_t a, uint8_t b, float t)
{
    t = clamp01f(t);
    return (uint8_t)((float)a + ((float)b - (float)a) * t);
}

static float visual_speed_level_at(uint32_t now_ms)
{
    return action_rate_to_speed(action_rate_decayed(now_ms));
}

static void led_zone_color(uint32_t frame_count, int zone_id, float speed_level, uint8_t *r, uint8_t *g, uint8_t *b)
{
    if (zone_id == 5) {
        const uint8_t hue = (frame_count * 2) % 255;
        const uint8_t v = 10 + (abs((int)(frame_count % 100) - 50) * 200 / 50);
        hsv_to_rgb(hue, 150, v, r, g, b);
        return;
    }

    const float t = clamp01f(speed_level);
    if (t < 0.35f) {
        const float k = smoothstepf(t / 0.35f);
        *r = led_lerp_u8(LED_ZONE_CYAN_R, LED_ZONE_SKY_R, k);
        *g = led_lerp_u8(LED_ZONE_CYAN_G, LED_ZONE_SKY_G, k);
        *b = led_lerp_u8(LED_ZONE_CYAN_B, LED_ZONE_SKY_B, k);
    } else if (t < 0.72f) {
        const float k = smoothstepf((t - 0.35f) / 0.37f);
        *r = led_lerp_u8(LED_ZONE_SKY_R, LED_ZONE_ORANGE_R, k);
        *g = led_lerp_u8(LED_ZONE_SKY_G, LED_ZONE_ORANGE_G, k);
        *b = led_lerp_u8(LED_ZONE_SKY_B, LED_ZONE_ORANGE_B, k);
    } else if (t < 0.92f) {
        const float k = smoothstepf((t - 0.72f) / 0.20f);
        *r = led_lerp_u8(LED_ZONE_ORANGE_R, LED_ZONE_DEEP_ORANGE_R, k);
        *g = led_lerp_u8(LED_ZONE_ORANGE_G, LED_ZONE_DEEP_ORANGE_G, k);
        *b = led_lerp_u8(LED_ZONE_ORANGE_B, LED_ZONE_DEEP_ORANGE_B, k);
    } else {
        const float k = smoothstepf((t - 0.92f) / 0.08f);
        *r = led_lerp_u8(LED_ZONE_DEEP_ORANGE_R, LED_ZONE_RED_R, k);
        *g = led_lerp_u8(LED_ZONE_DEEP_ORANGE_G, LED_ZONE_RED_G, k);
        *b = led_lerp_u8(LED_ZONE_DEEP_ORANGE_B, LED_ZONE_RED_B, k);
    }
}

static void led_render_running_visual(uint32_t frame_count)
{
    const int from_zone = s_visual_from_zone_id;
    const int to_zone = s_visual_to_zone_id;
    const int current_zone = s_visual_zone_id;
    const float mix = clamp01f(s_visual_blend_t);
    const float conf = fmaxf(s_visual_zone_confidence, 0.0f);
    const float conf_gain = 0.35f + 0.65f * clamp01f(conf / VISUAL_CONF_SOLID);
    /* 必须与判速写入端 (update_visual_speed_by_action_change 使用 esp_log_timestamp) 用同一时钟，
     * 否则 dt 会因常量偏移而 uint32 下溢，漏桶恒为 0，颜色永远偏蓝。 */
    const uint32_t now_ms = esp_log_timestamp();
    const float speed_level = visual_speed_level_at(now_ms);

    if (current_zone < 1 || current_zone > 9) {
        led_fill_all(LED_DIM_LEVEL, LED_DIM_LEVEL, LED_DIM_LEVEL);
        const int spin_idx = (frame_count / 3) % LED_STRIP_MAX_LEDS;
        led_strip_set_pixel(s_led_strip, spin_idx, LED_SPIN_LEVEL, LED_SPIN_LEVEL, LED_SPIN_LEVEL);
        return;
    }

    if (from_zone < 1 || to_zone < 1 || from_zone == to_zone) {
        uint8_t r = 0, g = 0, b = 0;
        led_zone_color(frame_count, current_zone, speed_level, &r, &g, &b);
        if (current_zone == 5) {
            led_fill_all((uint8_t)(r * conf_gain), (uint8_t)(g * conf_gain), (uint8_t)(b * conf_gain));
        } else {
            led_render_zone_ring(current_zone, (uint8_t)(r * conf_gain), (uint8_t)(g * conf_gain),
                                 (uint8_t)(b * conf_gain), false, 0, 0, 0);
        }
        return;
    }

    uint16_t acc_r[LED_STRIP_MAX_LEDS] = {0};
    uint16_t acc_g[LED_STRIP_MAX_LEDS] = {0};
    uint16_t acc_b[LED_STRIP_MAX_LEDS] = {0};

    uint8_t from_r = 0, from_g = 0, from_b = 0;
    uint8_t to_r = 0, to_g = 0, to_b = 0;
    led_zone_color(frame_count, from_zone, speed_level, &from_r, &from_g, &from_b);
    led_zone_color(frame_count, to_zone, speed_level, &to_r, &to_g, &to_b);

    led_add_zone_ring_to_accum(from_zone, from_r, from_g, from_b, conf_gain * (1.0f - mix), acc_r, acc_g, acc_b);
    led_add_zone_ring_to_accum(to_zone, to_r, to_g, to_b, conf_gain * mix, acc_r, acc_g, acc_b);
    led_commit_accum(acc_r, acc_g, acc_b);
}

static void led_strip_task(void *arg)
{
    uint32_t frame_count = 0;
    while (1) {
        if (is_going_to_sleep) {
            led_strip_clear(s_led_strip);
            led_strip_refresh(s_led_strip);
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        led_strip_clear(s_led_strip);

        if (current_state == STATE_ZONE_WIZARD) {
            zone_wizard_phase_t wz = zone_wizard_get_phase();
            int pt = zone_wizard_get_target_point();

            if (wz == ZONE_WIZARD_PROMPT) {
                const uint8_t v = 15 + (abs((int)(frame_count % 40) - 20) * 220 / 20);
                led_fill_all(v, 0, 0);
            } else if (pt >= 1 && pt <= 9) {
                if (pt == 5 && wz == ZONE_WIZARD_POINT_OK) {
                    led_fill_all(0, 220, 0);
                } else if (wz == ZONE_WIZARD_POINT_OK) {
                    led_render_zone_ring(pt, 0, 220, 0, false, 0, 0, 0);
                } else {
                    /* 校准：目标点位明亮琥珀，其余全圈低亮琥珀（对比拉大） */
                    led_render_zone_ring(pt, 255, 175, 0, true, LED_AMBER_DIM_R, LED_AMBER_DIM_G, LED_AMBER_DIM_B);
                }
            }
        } else {
            led_render_running_visual(frame_count);
        }

        led_strip_refresh(s_led_strip);
        frame_count++;
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

static bool configure_led_strip(void)
{
    void *handle = nullptr;
    esp_err_t ret = esp_board_manager_get_device_handle("led_strip", &handle);
    if (ret != ESP_OK || handle == nullptr) {
        ESP_LOGE(TAG, "Failed to get LED strip from Board Manager: %s", esp_err_to_name(ret));
        return false;
    }

    auto *led = static_cast<dev_led_strip_handles_t *>(handle);
    if (led->strip_handle == nullptr) {
        ESP_LOGE(TAG, "Board Manager LED strip handle is null");
        return false;
    }

    dev_led_strip_config_t *cfg = nullptr;
    ret = esp_board_manager_get_device_config("led_strip", (void **)&cfg);
    if (ret == ESP_OK && cfg != nullptr && cfg->strip_config.max_leds != LED_STRIP_MAX_LEDS) {
        ESP_LOGW(TAG, "LED strip max_leds=%d, MagEDC expects %d",
                 cfg->strip_config.max_leds, LED_STRIP_MAX_LEDS);
    }

    s_led_strip = led->strip_handle;
    xTaskCreate(led_strip_task, "led_strip_task", 2048, NULL, 5, NULL);
    return true;
}

/* --------------------------------------------------------------------------
 * BMI270 anymotion wakeup helpers (仅用于休眠唤醒配置)
 * -------------------------------------------------------------------------- */

static bool sensors_i2c_bus_init(void)
{
    if (s_sensor_i2c_bus) {
        return true;
    }

    esp_err_t ret = esp_board_manager_get_periph_handle("i2c_master", (void **)&s_sensor_i2c_bus);
    if (ret != ESP_OK || s_sensor_i2c_bus == nullptr) {
        ESP_LOGE(TAG, "Failed to get I2C master bus from Board Manager: %s", esp_err_to_name(ret));
        return false;
    }
    return true;
}

static bool bmi270_init_sensor(void)
{
    if (!s_sensor_i2c_bus) {
        return false;
    }

    esp_err_t ret = bmi270_sensor_create_from_master_bus(s_sensor_i2c_bus, &s_bmi_handle, bmi270_toy_config_file, 0);
    return (ret == ESP_OK && s_bmi_handle != NULL);
}

static bool bmm350_bind_interface(struct bmm350_dev *dev)
{
    if (!s_sensor_i2c_bus) {
        ESP_LOGE(TAG, "BMM350 bind failed: sensor I2C bus not ready");
        return false;
    }
    if (bmm350_set_i2c_master_bus_handle(s_sensor_i2c_bus) != ESP_OK) {
        ESP_LOGE(TAG, "BMM350 set native I2C master bus failed");
        return false;
    }
    bmm350_set_i2c_address(BMM350_I2C_ADDR);
    const int8_t rslt = bmm350_interface_init(dev);
    if (rslt != BMM350_OK) {
        ESP_LOGE(TAG, "BMM350 interface_init failed (%d)", (int)rslt);
        return false;
    }
    return true;
}

static bool bmm350_configure_runtime(struct bmm350_dev *dev)
{
    int8_t rslt = bmm350_init(dev);
    if (rslt != BMM350_OK) {
        ESP_LOGW(TAG, "BMM350 init failed (%d), trying soft reset", (int)rslt);
        bmm350_soft_reset(dev);
        bmm350_delay_us(BMM350_SOFT_RESET_DELAY + 10000, dev);
        rslt = bmm350_init(dev);
    }
    if (rslt != BMM350_OK) {
        ESP_LOGE(TAG, "BMM350 init failed after retry (%d)", (int)rslt);
        return false;
    }

    rslt = bmm350_set_odr_performance(BMM350_DATA_RATE_100HZ, BMM350_AVERAGING_4, dev);
    if (rslt != BMM350_OK) {
        ESP_LOGE(TAG, "BMM350 set_odr_performance failed (%d)", (int)rslt);
        return false;
    }
    rslt = bmm350_enable_axes(BMM350_X_EN, BMM350_Y_EN, BMM350_Z_EN, dev);
    if (rslt != BMM350_OK) {
        ESP_LOGE(TAG, "BMM350 enable_axes failed (%d)", (int)rslt);
        return false;
    }
    rslt = bmm350_set_powermode(BMM350_NORMAL_MODE, dev);
    if (rslt != BMM350_OK) {
        ESP_LOGE(TAG, "BMM350 set_powermode(NORMAL) failed (%d)", (int)rslt);
        return false;
    }
    return true;
}

static bool bmm350_recover_runtime(struct bmm350_dev *dev)
{
    if (!bmm350_bind_interface(dev)) {
        return false;
    }
    if (!bmm350_configure_runtime(dev)) {
        return false;
    }
    ESP_LOGW(TAG, "BMM350 recovered, sampling resumed");
    return true;
}

static bool bmi270_configure_any_motion_for_wakeup(void)
{
    if (s_bmi_handle == NULL) {
        return false;
    }
    int8_t rslt;
    uint8_t sens_list[2] = {BMI2_ACCEL, BMI2_GYRO};

    rslt = bmi2_set_adv_power_save(BMI2_DISABLE, s_bmi_handle);
    if (rslt != BMI2_OK) {
        return false;
    }
    struct bmi2_sens_config config[2] = {};
    struct bmi2_int_pin_config pin_config = {};
    config[BMI2_ACCEL].type = BMI2_ACCEL;
    config[BMI2_GYRO].type = BMI2_GYRO;
    rslt = bmi2_get_sensor_config(config, 2, s_bmi_handle);
    if (rslt != BMI2_OK) {
        return false;
    }

    config[BMI2_ACCEL].cfg.acc.odr = BMI2_ACC_ODR_200HZ;
    config[BMI2_ACCEL].cfg.acc.range = BMI2_ACC_RANGE_16G;
    config[BMI2_ACCEL].cfg.acc.bwp = BMI2_ACC_NORMAL_AVG4;
    config[BMI2_ACCEL].cfg.acc.filter_perf = BMI2_PERF_OPT_MODE;
    config[BMI2_GYRO].cfg.gyr.odr = BMI2_GYR_ODR_200HZ;
    config[BMI2_GYRO].cfg.gyr.range = BMI2_GYR_RANGE_2000;
    config[BMI2_GYRO].cfg.gyr.bwp = BMI2_GYR_NORMAL_MODE;
    config[BMI2_GYRO].cfg.gyr.noise_perf = BMI2_PERF_OPT_MODE;
    config[BMI2_GYRO].cfg.gyr.filter_perf = BMI2_PERF_OPT_MODE;
    rslt = bmi2_set_sensor_config(config, 2, s_bmi_handle);
    if (rslt != BMI2_OK) {
        return false;
    }

    pin_config.pin_type = BMI2_INT1;
    pin_config.pin_cfg[0].input_en = BMI2_INT_INPUT_DISABLE;
    pin_config.pin_cfg[0].lvl = BMI2_INT_ACTIVE_HIGH;
    pin_config.pin_cfg[0].od = BMI2_INT_PUSH_PULL;
    pin_config.pin_cfg[0].output_en = BMI2_INT_OUTPUT_ENABLE;
    pin_config.int_latch = BMI2_INT_NON_LATCH;
    rslt = bmi2_set_int_pin_config(&pin_config, s_bmi_handle);
    if (rslt != BMI2_OK) {
        return false;
    }

    uint8_t map_data = BMI270_TOY_INT_ANY_MOT_MASK;
    rslt = bmi2_set_regs(BMI2_INT1_MAP_FEAT_ADDR, &map_data, 1, s_bmi_handle);
    if (rslt != BMI2_OK) {
        return false;
    }

    rslt = bmi2_sensor_enable(sens_list, 2, s_bmi_handle);
    if (rslt != BMI2_OK) {
        return false;
    }
    rslt = bmi270_enable_toy_any_motion(s_bmi_handle, BMI2_ENABLE);
    if (rslt != BMI2_OK) {
        return false;
    }
    return true;
}

static bool configure_imu_wakeup_gpio(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << IMU_INT_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    if (gpio_config(&io_conf) != ESP_OK) {
        return false;
    }

    const uint32_t t0 = esp_log_timestamp();
    int pin_level = gpio_get_level(IMU_INT_PIN);
    while (pin_level != 0 && (esp_log_timestamp() - t0) < IMU_WAKE_PIN_SETTLE_TIMEOUT_MS) {
        if (s_bmi_handle != NULL) {
            uint8_t int_status = 0;
            if (bmi2_get_regs(BMI2_INT_STATUS_0_ADDR, &int_status, 1, s_bmi_handle) != BMI2_OK) {
                return false;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
        pin_level = gpio_get_level(IMU_INT_PIN);
    }

    if (pin_level != 0) {
        return false;
    }
    return gpio_hold_en(IMU_INT_PIN) == ESP_OK;
}

/* --------------------------------------------------------------------------
 * app_main
 * -------------------------------------------------------------------------- */

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "Starting (Geometrical Polar Partition Mode)...");

    esp_board_manager_print_board_info();

    esp_sleep_wakeup_cause_t wakeup_cause = esp_sleep_get_wakeup_cause();
    const esp_reset_reason_t reset_reason = esp_reset_reason();
    /* 仅以芯片复位原因为准；RTC 标记/ wakeup_cause 在烧录、开串口时会误判 */
    const bool from_deep_sleep = (reset_reason == ESP_RST_DEEPSLEEP);

    if (from_deep_sleep) {
        gpio_hold_dis(IMU_INT_PIN);
        ESP_LOGI(TAG, "Boot: deep-sleep resume (reset_reason=DEEPSLEEP wakeup=%d)", (int)wakeup_cause);
    } else {
        ESP_LOGI(TAG, "Boot: cold start (reset_reason=%d, wakeup=%d)", (int)reset_reason, (int)wakeup_cause);
    }

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    calib_nvs_sync_firmware(from_deep_sleep || TEMP_SKIP_REFLASH_WIZARD);

    // 统一初始化I2C总线管理
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << I2C_MASTER_SDO_IO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    gpio_set_level((gpio_num_t)I2C_MASTER_SDO_IO, 0);
    vTaskDelay(pdMS_TO_TICKS(10));

    ESP_ERROR_CHECK(esp_board_manager_init());
    if (!configure_led_strip()) {
        ESP_LOGE(TAG, "LED strip init failed");
        return;
    }

    if (!sensors_i2c_bus_init()) {
        ESP_LOGE(TAG, "Shared I2C bus init failed");
        return;
    }

    if (!bmi270_init_sensor()) {
        ESP_LOGW(TAG, "BMI270 init failed (deep-sleep wakeup may be unavailable)");
    } else {
        ESP_LOGI(TAG, "BMI270 ready for motion wakeup");
    }

    int8_t rslt = BMM350_OK;

    struct bmm350_dev dev = {0};
    if (!bmm350_bind_interface(&dev) || !bmm350_configure_runtime(&dev)) {
        ESP_LOGE(TAG, "BMM350 runtime configuration failed");
    } else {
        ESP_LOGI(TAG, "BMM350 ready (iot-solution component)");
    }

    zone_storage_init();

    const bool wizard_done = zone_storage_wizard_done();
    const bool calib_ok = zone_storage_calibration_complete();
    const bool center_ok = load_center_from_nvs();

    /* Restore only a complete calibration; valid data survives normal firmware updates. */
    const bool calibrated_once = (wizard_done && calib_ok && center_ok) || (bool)TEMP_SKIP_REFLASH_WIZARD;
    if (calibrated_once) {
        current_state = STATE_RUNNING;
        ESP_LOGI(TAG,
                 "Restoring calibration (wizard_done=%d calib_ok=%d center_ok=%d schema=%d). Entering running mode.",
                 wizard_done ? 1 : 0, calib_ok ? 1 : 0, center_ok ? 1 : 0, zone_storage_nvs_schema());
    } else {
        ESP_LOGW(TAG, "No complete calibration; entering 9-point wizard.");
        start_zone_wizard_calibration();
    }

    wifi_stream_init();
    zone_tone_init();
    speaker_gesture_init();
    reset_zone_runtime_channels(true);

    struct bmm350_mag_temp_data mag_temp_data = {0};
    uint32_t last_activity_ms = esp_log_timestamp();
    int prev_action_id = -2;
    int prev_wizard_point = 0;
    zone_wizard_phase_t prev_wizard_phase = ZONE_WIZARD_IDLE;
    struct {
        double sx;
        double sy;
        double sz;
        int n;
    } wizard_center_accum = {0};

    // ======= EMA 软件低通滤波器变量初始化 =======
    float smooth_x = 0.0f, smooth_y = 0.0f, smooth_z = 0.0f;
    bool is_filter_initialized = false;
    const float alpha_run = 0.46f;
    const float alpha_calib = 0.58f;
    TickType_t last_sample_tick = xTaskGetTickCount();

    if (current_state == STATE_RUNNING && from_deep_sleep) {
        position_debounce_reset();
        reset_zone_runtime_channels(true);
    }

    /* ==================== 主循环 ==================== */
    while (1) {
        if (s_recalib_requested) {
            s_recalib_requested = false;
            request_full_recalibration();
        }

        bool mag_ok = false;
        float mag_x = 0.0f, mag_y = 0.0f, mag_z = 0.0f;

        rslt = bmm350_get_compensated_mag_xyz_temp_data(&mag_temp_data, &dev);

        if (rslt == BMM350_OK) {
            mag_x = (float)mag_temp_data.x;
            mag_y = (float)mag_temp_data.y;
            mag_z = (float)mag_temp_data.z;
            mag_ok = true;
        }

        static int bmm_fail_streak = 0;
        static uint32_t bmm_last_fail_log_ms = 0;
        if (!mag_ok) {
            bmm_fail_streak++;
            const uint32_t now_ms = esp_log_timestamp();
            if (now_ms - bmm_last_fail_log_ms >= 1000) {
                bmm_last_fail_log_ms = now_ms;
                ESP_LOGW(TAG, "BMM350 sample failed rslt=%d streak=%d", (int)rslt, bmm_fail_streak);
            }
            if (bmm_fail_streak == 400 || (bmm_fail_streak > 400 && (bmm_fail_streak % 800) == 0)) {
                ESP_LOGW(TAG, "Attempting BMM350 recovery (streak=%d)", bmm_fail_streak);
                if (bmm350_recover_runtime(&dev)) {
                    bmm_fail_streak = 0;
                    bmm_last_fail_log_ms = now_ms;
                }
            }
        } else {
            bmm_fail_streak = 0;
        }

        if (mag_ok) {
            const float alpha = (current_state == STATE_RUNNING) ? alpha_run : alpha_calib;

            // EMA 一阶低通滤波（校准/运行共用；向导与运行检测均基于 smooth）
            if (!is_filter_initialized) {
                smooth_x = mag_x;
                smooth_y = mag_y;
                smooth_z = mag_z;
                is_filter_initialized = true;
            } else {
                smooth_x = alpha * mag_x + (1.0f - alpha) * smooth_x;
                smooth_y = alpha * mag_y + (1.0f - alpha) * smooth_y;
                smooth_z = alpha * mag_z + (1.0f - alpha) * smooth_z;
            }

            if (current_state == STATE_ZONE_WIZARD) {
                add_to_wizard_window(smooth_x, smooth_y, smooth_z);
            }

            static uint32_t log_timestamp = 0;
            if (esp_log_timestamp() - log_timestamp > 2000) {
                ESP_LOGI(TAG, "Mag uT x=%.2f y=%.2f z=%.2f temp=%.2f", mag_x, mag_y, mag_z,
                         mag_temp_data.temperature);
                log_timestamp = esp_log_timestamp();
            }

            /* ------------------ 状态机核心处理 ------------------ */
            if (current_state == STATE_ZONE_WIZARD) {
                const zone_wizard_phase_t wz_phase = zone_wizard_get_phase();
                int wz_pt = zone_wizard_get_target_point();
                if (wz_pt != prev_wizard_point) {
                    prev_wizard_point = wz_pt;
                    reset_wizard_window();
                    if (wz_pt == 5) {
                        wizard_center_accum.sx = 0.0;
                        wizard_center_accum.sy = 0.0;
                        wizard_center_accum.sz = 0.0;
                        wizard_center_accum.n = 0;
                    }
                }

                float wx = 0.0f;
                float wy = 0.0f;
                float wz = 0.0f;
                wizard_window_mean(&wx, &wy, &wz);
                const bool stable = is_wizard_window_stable(WIZARD_STABLE_VARIANCE_SQ);
                (void)wx;
                (void)wy;
                (void)wz;

                if (wz_pt == 5 && wz_phase == ZONE_WIZARD_COLLECT &&
                        (!stable || zone_wizard_get_sample_count() == 0)) {
                    wizard_center_accum = {};
                }
                if (wz_pt == 5 && wz_phase == ZONE_WIZARD_COLLECT && stable) {
                    wizard_center_accum.sx += (double)smooth_x;
                    wizard_center_accum.sy += (double)smooth_y;
                    wizard_center_accum.sz += (double)smooth_z;
                    wizard_center_accum.n++;
                    if (wizard_center_accum.n > 0) {
                        center_x = (int32_t)lrintf((float)(wizard_center_accum.sx / wizard_center_accum.n));
                        center_y = (int32_t)lrintf((float)(wizard_center_accum.sy / wizard_center_accum.n));
                        center_z = (int32_t)lrintf((float)(wizard_center_accum.sz / wizard_center_accum.n));
                    }
                }

                if (prev_wizard_phase != ZONE_WIZARD_POINT_OK && wz_phase == ZONE_WIZARD_POINT_OK && wz_pt == 5 &&
                        wizard_center_accum.n > 0) {
                    center_x = (int32_t)lrintf((float)(wizard_center_accum.sx / wizard_center_accum.n));
                    center_y = (int32_t)lrintf((float)(wizard_center_accum.sy / wizard_center_accum.n));
                    center_z = (int32_t)lrintf((float)(wizard_center_accum.sz / wizard_center_accum.n));
                    /* 中心仅在整个向导完成后写入 NVS，避免误进校准污染旧数据 */
                    ESP_LOGI(TAG, "Mag center from zone 5 (RAM): X=%d Y=%d Z=%d", (int)center_x, (int)center_y,
                             (int)center_z);
                }
                prev_wizard_phase = wz_phase;

                const float dx = smooth_x - (float)center_x;
                const float dy = smooth_y - (float)center_y;
                const float dz = smooth_z - (float)center_z;
                if (zone_wizard_feed_sample(dx, dy, dz, stable, esp_log_timestamp()) &&
                        save_center_to_nvs() && zone_wizard_complete()) {
                    current_state = STATE_RUNNING;
                    last_activity_ms = esp_log_timestamp();
                    prev_action_id = POSITION_INVALID;
                    prev_wizard_point = 0;
                    prev_wizard_phase = ZONE_WIZARD_IDLE;
                    reset_wizard_window();
                    is_filter_initialized = false;
                    position_debounce_reset();
                    reset_zone_runtime_channels(true);
#if ENABLE_WIFI
                    /* 校准结束：自动重连 Wi-Fi 并恢复省电 */
                    wifi_stream_set_calibrating(false);
#endif
                    ESP_LOGI(TAG, "Wizard complete — normal detection started.");
                }
            } else {
                float dx = (float)(smooth_x - center_x);
                float dy = (float)(smooth_y - center_y);
#if ZONE_DETECT_USE_XYZ
                float dz = (float)(smooth_z - center_z);
                float r_xy = sqrtf(dx * dx + dy * dy);
                float theta_deg = atan2f(dy, dx) * (180.0f / PI);

                float confidence, locked_confidence;
                int best_grid_id =
                    get_position_xyz_with_confidence(dx, dy, dz, last_stable_id, &confidence, &locked_confidence);

                static int dead_lock_streak = 0;
                if (last_stable_id >= 1 && locked_confidence <= ZONE_LOCK_RELEASE_CONF &&
                        confidence <= ZONE_LOCK_RELEASE_CONF && best_grid_id == last_stable_id) {
                    dead_lock_streak++;
                    if (dead_lock_streak > 8) {
                        last_stable_id = POSITION_INVALID;
                        position_debounce_reset();
                        dead_lock_streak = 0;
                        best_grid_id = get_position_xyz_with_confidence(dx, dy, dz, last_stable_id, &confidence,
                                                                        &locked_confidence);
                    }
                } else {
                    dead_lock_streak = 0;
                }

                if (try_force_stale_lock_switch(best_grid_id, confidence, locked_confidence, &last_stable_id)) {
                } else if (try_confirm_position_change(best_grid_id, confidence, locked_confidence, &last_stable_id)) {
                    ESP_LOGI(TAG, "Position changed to [%d] (conf=%.3f)", last_stable_id, confidence);
                }

                static uint32_t dbg_conf_ts = 0;
                const uint32_t now_dbg = esp_log_timestamp();
                if (now_dbg - dbg_conf_ts > 2000) {
                    const int nearest_id = find_nearest_zone_xyz(dx, dy, dz);
                    ESP_LOGD(TAG,
                             "Position candidate: [%d] conf=%.3f, locked=[%d] lock_conf=%.3f, nearest=[%d], dX=%.0f "
                             "dY=%.0f dZ=%.0f",
                             best_grid_id, confidence, last_stable_id, locked_confidence, nearest_id, dx, dy, dz);
                    dbg_conf_ts = now_dbg;
                }

                update_zone_output_channels(best_grid_id, confidence, locked_confidence, dx, dy, dz, r_xy, theta_deg);
#else
                float dz = (float)(smooth_z - center_z);
                float r_xy = sqrtf(dx * dx + dy * dy);
                float theta_deg = atan2f(dy, dx) * (180.0f / PI);

                float confidence, locked_confidence;
                int best_grid_id = get_position_polar_with_confidence(r_xy, theta_deg, dz, last_stable_id, &confidence,
                                                                      &locked_confidence);

                static int dead_lock_streak = 0;
                if (last_stable_id >= 1 && locked_confidence <= ZONE_LOCK_RELEASE_CONF &&
                        confidence <= ZONE_LOCK_RELEASE_CONF && best_grid_id == last_stable_id) {
                    dead_lock_streak++;
                    if (dead_lock_streak > 8) {
                        last_stable_id = POSITION_INVALID;
                        position_debounce_reset();
                        dead_lock_streak = 0;
                        best_grid_id = get_position_polar_with_confidence(r_xy, theta_deg, dz, last_stable_id,
                                                                          &confidence, &locked_confidence);
                    }
                } else {
                    dead_lock_streak = 0;
                }

                if (try_force_stale_lock_switch(best_grid_id, confidence, locked_confidence, &last_stable_id)) {
                } else if (try_confirm_position_change(best_grid_id, confidence, locked_confidence, &last_stable_id)) {
                    ESP_LOGI(TAG, "Position changed to [%d] (conf=%.3f)", last_stable_id, confidence);
                }

                static uint32_t dbg_conf_ts = 0;
                const uint32_t now_dbg = esp_log_timestamp();
                if (now_dbg - dbg_conf_ts > 2000) {
                    const int nearest_id = find_nearest_zone(r_xy, theta_deg);
                    const polar_zone_t *bz = zone_by_id(best_grid_id);
                    const float z_gate = bz ? zone_polar_z_gate_score(dz, bz) : 0.0f;
                    ESP_LOGD(TAG,
                             "Position candidate: [%d] conf=%.3f, locked=[%d] lock_conf=%.3f, nearest=[%d], R=%.0f "
                             "Ang=%.0f dZ=%.0f z_gate=%.2f",
                             best_grid_id, confidence, last_stable_id, locked_confidence, nearest_id, r_xy, theta_deg,
                             dz, z_gate);
                    dbg_conf_ts = now_dbg;
                }

                update_zone_output_channels(best_grid_id, confidence, locked_confidence, dx, dy, dz, r_xy, theta_deg);
#endif
            }
        }

        // 休眠定时器逻辑：仅“可信且非中心点”的动作切换视为有效操作
        bool idle_activity = false;
        if (s_action_zone_id != prev_action_id) {
            const int new_action_id = s_action_zone_id;
            const bool valid_zone = (new_action_id >= 1 && new_action_id <= 9);
            const bool center_zone = (new_action_id == CENTER_ZONE_ID);
            if (valid_zone && !center_zone && s_action_zone_confidence >= IDLE_ACTIVITY_CONF_MIN) {
                idle_activity = true;
            }
            prev_action_id = new_action_id;
        }

        // 1分钟无操作进入深度休眠（WiFi 连接/重试阶段会暂缓）
        if (idle_sleep_due(current_state == STATE_RUNNING, idle_activity, esp_log_timestamp(), &last_activity_ms)) {
            static uint32_t wifi_block_log_ts = 0;
            static uint32_t imu_block_log_ts = 0;
            if (wifi_stream_inhibits_sleep()) {
                uint32_t now = esp_log_timestamp();
                if (now - wifi_block_log_ts > 2000) {
                    ESP_LOGI(TAG, "Sleep deferred: Wi-Fi connecting or web pairing active");
                    wifi_block_log_ts = now;
                }
                vTaskDelay(pdMS_TO_TICKS(20));
                continue;
            }

            if (!bmi270_configure_any_motion_for_wakeup()) {
                ESP_LOGW(TAG, "Sleep deferred: IMU motion wakeup configuration failed");
                last_activity_ms = esp_log_timestamp();
                continue;
            }
            if (!configure_imu_wakeup_gpio()) {
                uint32_t now = esp_log_timestamp();
                if (now - imu_block_log_ts > 2000) {
                    ESP_LOGW(TAG, "Sleep deferred: IMU wake pin is not ready");
                    imu_block_log_ts = now;
                }
                vTaskDelay(pdMS_TO_TICKS(20));
                continue;
            }

            ESP_LOGW(TAG, "Idle 1 min, entering deep sleep...");
            zone_tone_set_zone(-1);
            is_going_to_sleep = true;
            vTaskDelay(pdMS_TO_TICKS(50));
            led_strip_clear(s_led_strip);
            led_strip_refresh(s_led_strip);

            wifi_stream_prepare_sleep();

            bmm350_set_powermode(BMM350_SUSPEND_MODE, &dev);

            /* Ensure no stale wakeup source (e.g., timer) survives into deep sleep */
            ESP_ERROR_CHECK(esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL));
            ESP_ERROR_CHECK(esp_sleep_enable_ext1_wakeup(1ULL << IMU_INT_PIN, ESP_EXT1_WAKEUP_ANY_HIGH));

            esp_deep_sleep_start();
        }

        xTaskDelayUntil(&last_sample_tick, pdMS_TO_TICKS(10));
    }
}
