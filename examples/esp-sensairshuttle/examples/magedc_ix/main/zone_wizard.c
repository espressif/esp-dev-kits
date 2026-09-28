/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "zone_wizard.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "esp_log.h"
#include "calib_nvs.h"
#include "mag_zones_config.h"
#include "zone_storage.h"
#include "zone_tone.h"

static const char *TAG = "ZONE_WIZ";

#define WIZARD_MAX_SAMPLES 192
#define WIZARD_SAMPLES_TARGET 96
#define WIZARD_STABLE_STREAK_NEED 3
#define WIZARD_MIN_QUALITY_SCORE 50.0f
#define WIZARD_PROMPT_MS 4000
#define WIZARD_POINT_OK_MS 600
#define WIZARD_R_MARGIN_BASE 300.0f
#define WIZARD_TH_MARGIN_BASE 24.0f
#define WIZARD_MIN_R_SPAN 450.0f
#define WIZARD_MIN_TH_SPAN 32.0f
#define WIZARD_X_MARGIN_BASE 300.0f
#define WIZARD_Z_MARGIN_BASE 200.0f
#define WIZARD_MIN_XYZ_SPAN 400.0f
#define WIZARD_MIN_Z_SPAN 220.0f
#define WIZARD_CENTER_XYZ_MIN 1500.0f
#define WIZARD_CENTER_R_MIN 1500.0f
#define WIZARD_MARGIN_SCALE 1.40f
#define WIZARD_PCT_LO 10.0f
#define WIZARD_PCT_HI 90.0f
/** 拟合完成后对 R/θ/Z 范围再外扩（相对半宽倍率 + 绝对余量） */
#define WIZARD_POST_EXPAND 1.35f
#define WIZARD_R_EXTRA 100.0f
#define WIZARD_TH_EXTRA 10.0f
#define WIZARD_Z_EXTRA 55.0f
#define CENTER_ZONE_ID 5

static const int8_t WIZARD_POINT_ORDER[] = {5, 1, 2, 3, 6, 9, 8, 7, 4};
#define WIZARD_POINT_COUNT ((int)(sizeof(WIZARD_POINT_ORDER) / sizeof(WIZARD_POINT_ORDER[0])))

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static zone_wizard_phase_t s_phase = ZONE_WIZARD_IDLE;
static int s_target_point = 0;
static int s_sample_count = 0;
static int s_stable_streak = 0;
static uint32_t s_prompt_start_ms = 0;
static uint32_t s_point_ok_until_ms = 0;
static bool s_zones_saved = false;

static float s_dx[WIZARD_MAX_SAMPLES];
static float s_dy[WIZARD_MAX_SAMPLES];
static float s_dz[WIZARD_MAX_SAMPLES];
static polar_zone_t s_wizard_zones[9];

static int compare_float(const void *a, const void *b)
{
    float fa = *(const float *)a;
    float fb = *(const float *)b;
    if (fa < fb) {
        return -1;
    }
    if (fa > fb) {
        return 1;
    }
    return 0;
}

static float percentile_sorted(const float *sorted, int n, float pct)
{
    if (n <= 0) {
        return 0.0f;
    }
    if (n == 1) {
        return sorted[0];
    }
    const float rank = (pct / 100.0f) * (float)(n - 1);
    const int lo = (int)rank;
    const int hi = lo + 1;
    if (hi >= n) {
        return sorted[n - 1];
    }
    const float frac = rank - (float)lo;
    return sorted[lo] * (1.0f - frac) + sorted[hi] * frac;
}

static void iqr_bounds(const float *data, int n, float mult, float *lo_out, float *hi_out)
{
    float tmp[WIZARD_MAX_SAMPLES];
    if (n > WIZARD_MAX_SAMPLES) {
        n = WIZARD_MAX_SAMPLES;
    }
    memcpy(tmp, data, (size_t)n * sizeof(float));
    qsort(tmp, (size_t)n, sizeof(float), compare_float);

    const float q1 = percentile_sorted(tmp, n, 25.0f);
    const float q3 = percentile_sorted(tmp, n, 75.0f);
    const float iqr = q3 - q1;
    *lo_out = q1 - mult * iqr;
    *hi_out = q3 + mult * iqr;
}

static int filter_outliers(float *dx, float *dy, int n)
{
    if (n <= 4) {
        return n;
    }

    float r[WIZARD_MAX_SAMPLES];
    float th[WIZARD_MAX_SAMPLES];
    for (int i = 0; i < n; i++) {
        r[i] = hypotf(dx[i], dy[i]);
        th[i] = atan2f(dy[i], dx[i]) * (180.0f / (float)M_PI);
    }

    float r_lo, r_hi, th_lo, th_hi;
    iqr_bounds(r, n, 1.35f, &r_lo, &r_hi);
    iqr_bounds(th, n, 1.35f, &th_lo, &th_hi);

    int w = 0;
    for (int i = 0; i < n; i++) {
        if (r[i] >= r_lo && r[i] <= r_hi && th[i] >= th_lo && th[i] <= th_hi) {
            dx[w] = dx[i];
            dy[w] = dy[i];
            w++;
        }
    }
    return (w >= 8) ? w : n;
}

#if ZONE_DETECT_USE_XYZ

static int filter_outliers_xyz(float *dx, float *dy, float *dz, int n)
{
    if (n <= 4) {
        return n;
    }

    float x_lo, x_hi, y_lo, y_hi, z_lo, z_hi;
    iqr_bounds(dx, n, 1.35f, &x_lo, &x_hi);
    iqr_bounds(dy, n, 1.35f, &y_lo, &y_hi);
    iqr_bounds(dz, n, 1.35f, &z_lo, &z_hi);

    int w = 0;
    for (int i = 0; i < n; i++) {
        if (dx[i] >= x_lo && dx[i] <= x_hi && dy[i] >= y_lo && dy[i] <= y_hi && dz[i] >= z_lo && dz[i] <= z_hi) {
            dx[w] = dx[i];
            dy[w] = dy[i];
            dz[w] = dz[i];
            w++;
        }
    }
    return (w >= 8) ? w : n;
}

#endif /* ZONE_DETECT_USE_XYZ */

#if ZONE_DETECT_USE_XYZ

static float calc_quality_score_xyz(int raw_n, int filtered_n, float x_std, float y_std, float z_std)
{
    float score = 100.0f;
    if (raw_n < 50) {
        score -= (50.0f - (float)raw_n) * 1.0f;
    }
    const float outlier_ratio = (raw_n > 0) ? (float)(raw_n - filtered_n) / (float)raw_n : 0.0f;
    if (outlier_ratio > 0.2f) {
        score -= (outlier_ratio - 0.2f) * 100.0f;
    }
    const float axis_std = fmaxf(x_std, fmaxf(y_std, z_std));
    if (axis_std > 120.0f) {
        score -= (axis_std - 120.0f) * 0.35f;
    }
    if (score < 0.0f) {
        score = 0.0f;
    }
    if (score > 100.0f) {
        score = 100.0f;
    }
    return score;
}

static void fit_axis_bounds(const float *axis, int n, float margin, float min_span_factor, float pct_lo, float pct_hi,
                            float *min_out, float *max_out)
{
    float sorted[WIZARD_MAX_SAMPLES];
    memcpy(sorted, axis, (size_t)n * sizeof(float));
    qsort(sorted, (size_t)n, sizeof(float), compare_float);

    const float p_lo = percentile_sorted(sorted, n, pct_lo);
    const float p_hi = percentile_sorted(sorted, n, pct_hi);
    float amin = p_lo - margin;
    float amax = p_hi + margin;
    const float span = p_hi - p_lo;
    if (span < WIZARD_MIN_XYZ_SPAN * min_span_factor) {
        const float mid = (p_lo + p_hi) * 0.5f;
        const float target_span = fmaxf(span + 2.0f * margin, WIZARD_MIN_XYZ_SPAN);
        amin = mid - target_span * 0.5f;
        amax = mid + target_span * 0.5f;
    }
    *min_out = rintf(amin);
    *max_out = rintf(amax);
}

static void fit_xyz_bounds(int zone_id, const float *dx_in, const float *dy_in, const float *dz_in, int n,
                           polar_zone_t *out)
{
    float dx[WIZARD_MAX_SAMPLES];
    float dy[WIZARD_MAX_SAMPLES];
    float dz[WIZARD_MAX_SAMPLES];
    if (n > WIZARD_MAX_SAMPLES) {
        n = WIZARD_MAX_SAMPLES;
    }
    memcpy(dx, dx_in, (size_t)n * sizeof(float));
    memcpy(dy, dy_in, (size_t)n * sizeof(float));
    memcpy(dz, dz_in, (size_t)n * sizeof(float));

    const int raw_n = n;
    n = filter_outliers_xyz(dx, dy, dz, n);
    if (n > 12) {
        n = filter_outliers_xyz(dx, dy, dz, n);
    }

    float x_mean = 0.0f;
    float y_mean = 0.0f;
    float z_mean = 0.0f;
    for (int i = 0; i < n; i++) {
        x_mean += dx[i];
        y_mean += dy[i];
        z_mean += dz[i];
    }
    if (n > 0) {
        x_mean /= (float)n;
        y_mean /= (float)n;
        z_mean /= (float)n;
    }

    float x_var = 0.0f;
    float y_var = 0.0f;
    float z_var = 0.0f;
    for (int i = 0; i < n; i++) {
        const float ddx = dx[i] - x_mean;
        const float ddy = dy[i] - y_mean;
        const float ddz = dz[i] - z_mean;
        x_var += ddx * ddx;
        y_var += ddy * ddy;
        z_var += ddz * ddz;
    }
    const float x_std = (n > 1) ? sqrtf(x_var / (float)n) : 0.0f;
    const float y_std = (n > 1) ? sqrtf(y_var / (float)n) : 0.0f;
    const float z_std = (n > 1) ? sqrtf(z_var / (float)n) : 0.0f;

    const float outlier_ratio = (raw_n > 0) ? (float)(raw_n - n) / (float)raw_n : 0.0f;
    float margin_scale = WIZARD_MARGIN_SCALE;
    float pct_lo = WIZARD_PCT_LO;
    float pct_hi = WIZARD_PCT_HI;
    float min_span_factor = 1.0f;

    float x_margin = WIZARD_X_MARGIN_BASE * (1.0f + outlier_ratio) * margin_scale;
    float y_margin = WIZARD_X_MARGIN_BASE * (1.0f + outlier_ratio) * margin_scale;
    float z_margin = WIZARD_Z_MARGIN_BASE * (1.0f + outlier_ratio) * margin_scale;
    x_margin = fmaxf(x_margin, (x_std * 3.5f + 140.0f) * margin_scale);
    y_margin = fmaxf(y_margin, (y_std * 3.5f + 140.0f) * margin_scale);
    z_margin = fmaxf(z_margin, (z_std * 3.5f + 90.0f) * margin_scale);

    memset(out, 0, sizeof(*out));
    out->id = zone_id;
    out->quality_score = calc_quality_score_xyz(raw_n, n, x_std, y_std, z_std);

    fit_axis_bounds(dx, n, x_margin, min_span_factor, pct_lo, pct_hi, &out->x_min, &out->x_max);
    fit_axis_bounds(dy, n, y_margin, min_span_factor, pct_lo, pct_hi, &out->y_min, &out->y_max);
    fit_axis_bounds(dz, n, z_margin, min_span_factor * 0.85f, pct_lo, pct_hi, &out->z_min, &out->z_max);

    if (zone_id == CENTER_ZONE_ID) {
        const float half = fmaxf(fmaxf(out->x_max - out->x_min, out->y_max - out->y_min) * 0.5f, WIZARD_CENTER_XYZ_MIN);
        out->x_min = -half;
        out->x_max = half;
        out->y_min = -half;
        out->y_max = half;
        const float z_half = fmaxf((out->z_max - out->z_min) * 0.5f, WIZARD_MIN_Z_SPAN);
        out->z_min = -z_half;
        out->z_max = z_half;
    }
}

#endif /* ZONE_DETECT_USE_XYZ */

static float circular_mean_deg(const float *angles, int n)
{
    if (n <= 0) {
        return 0.0f;
    }
    float sx = 0.0f;
    float sy = 0.0f;
    for (int i = 0; i < n; i++) {
        const float rad = angles[i] * ((float)M_PI / 180.0f);
        sx += cosf(rad);
        sy += sinf(rad);
    }
    return atan2f(sy, sx) * (180.0f / (float)M_PI);
}

static float calc_quality_score(int raw_n, int filtered_n, float r_std, float r_mean, float th_std)
{
    float score = 100.0f;
    if (raw_n < 50) {
        score -= (50.0f - (float)raw_n) * 1.0f;
    }
    const float outlier_ratio = (raw_n > 0) ? (float)(raw_n - filtered_n) / (float)raw_n : 0.0f;
    if (outlier_ratio > 0.2f) {
        score -= (outlier_ratio - 0.2f) * 100.0f;
    }
    const float cv = (r_mean > 1.0f) ? (r_std / r_mean) : r_std;
    if (cv > 0.05f) {
        score -= (cv - 0.05f) * 1000.0f;
    }
    if (th_std > 15.0f) {
        score -= (th_std - 15.0f) * 2.0f;
    }
    if (score < 0.0f) {
        score = 0.0f;
    }
    if (score > 100.0f) {
        score = 100.0f;
    }
    return score;
}

#if !ZONE_DETECT_USE_XYZ
static void fit_polar_bounds(int zone_id, const float *dx_in, const float *dy_in, int n, polar_zone_t *out)
{
    float dx[WIZARD_MAX_SAMPLES];
    float dy[WIZARD_MAX_SAMPLES];
    if (n > WIZARD_MAX_SAMPLES) {
        n = WIZARD_MAX_SAMPLES;
    }
    memcpy(dx, dx_in, (size_t)n * sizeof(float));
    memcpy(dy, dy_in, (size_t)n * sizeof(float));

    const int raw_n = n;
    n = filter_outliers(dx, dy, n);
    if (n > 12) {
        n = filter_outliers(dx, dy, n);
    }

    float r[WIZARD_MAX_SAMPLES];
    float angles[WIZARD_MAX_SAMPLES];
    float r_sum = 0.0f;
    float r_min = 1e9f;
    float r_max = 0.0f;

    for (int i = 0; i < n; i++) {
        r[i] = hypotf(dx[i], dy[i]);
        angles[i] = atan2f(dy[i], dx[i]) * (180.0f / (float)M_PI);
        r_sum += r[i];
        if (r[i] < r_min) {
            r_min = r[i];
        }
        if (r[i] > r_max) {
            r_max = r[i];
        }
    }

    const float r_mean = (n > 0) ? (r_sum / (float)n) : 0.0f;
    float r_var = 0.0f;
    for (int i = 0; i < n; i++) {
        const float dr = r[i] - r_mean;
        r_var += dr * dr;
    }
    const float r_std = (n > 1) ? sqrtf(r_var / (float)n) : 0.0f;
    const float th_mean = circular_mean_deg(angles, n);

    float th_var = 0.0f;
    for (int i = 0; i < n; i++) {
        float dth = angles[i] - th_mean;
        while (dth > 180.0f) {
            dth -= 360.0f;
        }
        while (dth < -180.0f) {
            dth += 360.0f;
        }
        th_var += dth * dth;
    }
    const float th_std = (n > 1) ? sqrtf(th_var / (float)n) : 0.0f;

    const float outlier_ratio = (raw_n > 0) ? (float)(raw_n - n) / (float)raw_n : 0.0f;
    float margin_scale = WIZARD_MARGIN_SCALE;
    float pct_lo = WIZARD_PCT_LO;
    float pct_hi = WIZARD_PCT_HI;
    float min_r_span_factor = 1.0f;
    float min_th_span_factor = 1.0f;

    float r_margin = WIZARD_R_MARGIN_BASE * (1.0f + outlier_ratio) * margin_scale;
    float th_margin = WIZARD_TH_MARGIN_BASE * (1.0f + outlier_ratio) * margin_scale;
    r_margin = fmaxf(r_margin, (r_std * 3.5f + 140.0f) * margin_scale);
    th_margin = fmaxf(th_margin, (th_std * 3.0f + 12.0f) * margin_scale);

    float r_sorted[WIZARD_MAX_SAMPLES];
    float th_aligned[WIZARD_MAX_SAMPLES];
    memcpy(r_sorted, r, (size_t)n * sizeof(float));
    qsort(r_sorted, (size_t)n, sizeof(float), compare_float);

    const float r_p10 = percentile_sorted(r_sorted, n, pct_lo);
    const float r_p90 = percentile_sorted(r_sorted, n, pct_hi);

    memset(out, 0, sizeof(*out));
    out->id = zone_id;
    out->quality_score = calc_quality_score(raw_n, n, r_std, r_mean, th_std);

    if (zone_id == CENTER_ZONE_ID) {
        out->r_min = 0.0f;
        out->r_max = r_p90 + r_margin;
        if (out->r_max < WIZARD_CENTER_R_MIN) {
            out->r_max = WIZARD_CENTER_R_MIN;
        }
        out->th_min = -180.0f;
        out->th_max = 180.0f;
        return;
    }

    r_min = fmaxf(0.0f, r_p10 - r_margin);
    r_max = r_p90 + r_margin;
    const float r_sample_span = r_p90 - r_p10;
    if (r_sample_span < WIZARD_MIN_R_SPAN * min_r_span_factor) {
        const float mid = (r_p10 + r_p90) * 0.5f;
        const float target_span = fmaxf(r_sample_span + 2.0f * r_margin, WIZARD_MIN_R_SPAN);
        r_min = fmaxf(0.0f, mid - target_span * 0.5f);
        r_max = mid + target_span * 0.5f;
    }

    for (int i = 0; i < n; i++) {
        float d = angles[i] - th_mean;
        while (d > 180.0f) {
            d -= 360.0f;
        }
        while (d < -180.0f) {
            d += 360.0f;
        }
        th_aligned[i] = th_mean + d;
    }
    qsort(th_aligned, (size_t)n, sizeof(float), compare_float);

    const float th_p10 = percentile_sorted(th_aligned, n, pct_lo);
    const float th_p90 = percentile_sorted(th_aligned, n, pct_hi);
    float th_min = fmodf(th_p10 - th_margin + 180.0f, 360.0f) - 180.0f;
    float th_max = fmodf(th_p90 + th_margin + 180.0f, 360.0f) - 180.0f;

    const float th_sample_span = th_p90 - th_p10;
    if (th_sample_span < WIZARD_MIN_TH_SPAN * min_th_span_factor) {
        const float mid = (th_p10 + th_p90) * 0.5f;
        const float target_span = fmaxf(th_sample_span + 2.0f * th_margin, WIZARD_MIN_TH_SPAN);
        th_min = fmodf(mid - target_span * 0.5f + 180.0f, 360.0f) - 180.0f;
        th_max = fmodf(mid + target_span * 0.5f + 180.0f, 360.0f) - 180.0f;
    }

    out->r_min = rintf(r_min);
    out->r_max = rintf(r_max);
    out->th_min = rintf(th_min * 10.0f) / 10.0f;
    out->th_max = rintf(th_max * 10.0f) / 10.0f;
}
#endif /* !ZONE_DETECT_USE_XYZ */

#if !ZONE_DETECT_USE_XYZ && ZONE_POLAR_Z_GATE

static void fit_z_bounds(int zone_id, const float *dz_in, int n, polar_zone_t *out)
{
    float dz[WIZARD_MAX_SAMPLES];
    if (n > WIZARD_MAX_SAMPLES) {
        n = WIZARD_MAX_SAMPLES;
    }
    memcpy(dz, dz_in, (size_t)n * sizeof(float));

    float z_lo, z_hi;
    iqr_bounds(dz, n, 1.35f, &z_lo, &z_hi);
    int w = 0;
    for (int i = 0; i < n; i++) {
        if (dz[i] >= z_lo && dz[i] <= z_hi) {
            dz[w++] = dz[i];
        }
    }
    if (w >= 8) {
        n = w;
    }

    float sorted[WIZARD_MAX_SAMPLES];
    memcpy(sorted, dz, (size_t)n * sizeof(float));
    qsort(sorted, (size_t)n, sizeof(float), compare_float);

    float pct_lo = WIZARD_PCT_LO;
    float pct_hi = WIZARD_PCT_HI;
    const float margin_scale = WIZARD_MARGIN_SCALE;

    float z_var = 0.0f;
    float z_mean = 0.0f;
    for (int i = 0; i < n; i++) {
        z_mean += dz[i];
    }
    if (n > 0) {
        z_mean /= (float)n;
    }
    for (int i = 0; i < n; i++) {
        const float d = dz[i] - z_mean;
        z_var += d * d;
    }
    const float z_std = (n > 1) ? sqrtf(z_var / (float)n) : 0.0f;
    float z_margin = WIZARD_Z_MARGIN_BASE * margin_scale;
    z_margin = fmaxf(z_margin, (z_std * 3.5f + 90.0f) * margin_scale);

    const float p_lo = percentile_sorted(sorted, n, pct_lo);
    const float p_hi = percentile_sorted(sorted, n, pct_hi);
    float zmin = p_lo - z_margin;
    float zmax = p_hi + z_margin;
    const float span = p_hi - p_lo;
    if (span < WIZARD_MIN_Z_SPAN) {
        const float mid = (p_lo + p_hi) * 0.5f;
        const float target_span = fmaxf(span + 2.0f * z_margin, WIZARD_MIN_Z_SPAN);
        zmin = mid - target_span * 0.5f;
        zmax = mid + target_span * 0.5f;
    }

    out->z_min = rintf(zmin);
    out->z_max = rintf(zmax);
}

#endif /* polar + Z gate calib */

#if !ZONE_DETECT_USE_XYZ
/** 拟合完成后统一外扩 R/θ/Z 判定范围 */
static void expand_calibrated_bounds(polar_zone_t *z)
{
    if (z->id == CENTER_ZONE_ID) {
        z->r_max = rintf(z->r_max * WIZARD_POST_EXPAND + WIZARD_R_EXTRA);
        if (z->r_max < WIZARD_CENTER_R_MIN) {
            z->r_max = WIZARD_CENTER_R_MIN;
        }
#if ZONE_POLAR_Z_GATE
        if (z->z_max > z->z_min) {
            const float z_mid = (z->z_min + z->z_max) * 0.5f;
            float z_half = (z->z_max - z->z_min) * 0.5f * WIZARD_POST_EXPAND + WIZARD_Z_EXTRA;
            z->z_min = rintf(z_mid - z_half);
            z->z_max = rintf(z_mid + z_half);
        }
#endif
        return;
    }

    const float r_mid = (z->r_min + z->r_max) * 0.5f;
    float r_half = (z->r_max - z->r_min) * 0.5f;
    r_half = r_half * WIZARD_POST_EXPAND + WIZARD_R_EXTRA;
    z->r_min = rintf(fmaxf(0.0f, r_mid - r_half));
    z->r_max = rintf(r_mid + r_half);

    z->th_min = fmodf(z->th_min - WIZARD_TH_EXTRA + 180.0f, 360.0f) - 180.0f;
    z->th_max = fmodf(z->th_max + WIZARD_TH_EXTRA + 180.0f, 360.0f) - 180.0f;

#if ZONE_POLAR_Z_GATE
    if (z->z_max > z->z_min) {
        const float z_mid = (z->z_min + z->z_max) * 0.5f;
        float z_half = (z->z_max - z->z_min) * 0.5f * WIZARD_POST_EXPAND + WIZARD_Z_EXTRA;
        z->z_min = rintf(z_mid - z_half);
        z->z_max = rintf(z_mid + z_half);
    }
#endif
}
#endif /* !ZONE_DETECT_USE_XYZ */

#if ZONE_DETECT_USE_XYZ
static void expand_xyz_bounds(polar_zone_t *z)
{
    const float x_mid = (z->x_min + z->x_max) * 0.5f;
    const float y_mid = (z->y_min + z->y_max) * 0.5f;
    const float z_mid = (z->z_min + z->z_max) * 0.5f;
    float x_half = (z->x_max - z->x_min) * 0.5f * WIZARD_POST_EXPAND + WIZARD_R_EXTRA;
    float y_half = (z->y_max - z->y_min) * 0.5f * WIZARD_POST_EXPAND + WIZARD_R_EXTRA;
    float z_half = (z->z_max - z->z_min) * 0.5f * WIZARD_POST_EXPAND + WIZARD_Z_EXTRA;
    z->x_min = rintf(x_mid - x_half);
    z->x_max = rintf(x_mid + x_half);
    z->y_min = rintf(y_mid - y_half);
    z->y_max = rintf(y_mid + y_half);
    z->z_min = rintf(z_mid - z_half);
    z->z_max = rintf(z_mid + z_half);
}
#endif

static void begin_point(int point)
{
    s_target_point = point;
    s_sample_count = 0;
    s_stable_streak = 0;
    s_phase = ZONE_WIZARD_COLLECT;
    if (point == CENTER_ZONE_ID) {
        ESP_LOGW(TAG, ">>> [5] 中心：放好磁珠并保持静止（可倾斜手持姿态，勿移动磁珠）<<<");
    } else {
        ESP_LOGW(TAG, ">>> [%d] 放好磁珠保持静止（可倾斜手持，勿移动磁珠）<<<", point);
    }
}

/** 5 号中心点：拟合前减去样本均值 */
static void demean_center_samples(float *rdx, float *rdy, float *rdz, int n)
{
    if (n <= 0) {
        return;
    }

    float mx = 0.0f;
    float my = 0.0f;
    float mz = 0.0f;
    for (int i = 0; i < n; i++) {
        mx += rdx[i];
        my += rdy[i];
        mz += rdz[i];
    }
    mx /= (float)n;
    my /= (float)n;
    mz /= (float)n;

    for (int i = 0; i < n; i++) {
        rdx[i] -= mx;
        rdy[i] -= my;
        rdz[i] -= mz;
    }

    ESP_LOGI(TAG, "点位 [5] 去均值后拟合: mean dX=%.0f dY=%.0f dZ=%.0f", mx, my, mz);
}

static bool apply_zone_fit(int zone_id, const float *dx_in, const float *dy_in, const float *dz_in, int sample_n)
{
    if (sample_n <= 0) {
        return false;
    }

    float work_dx[WIZARD_MAX_SAMPLES];
    float work_dy[WIZARD_MAX_SAMPLES];
    float work_dz[WIZARD_MAX_SAMPLES];
    if (sample_n > WIZARD_MAX_SAMPLES) {
        sample_n = WIZARD_MAX_SAMPLES;
    }
    memcpy(work_dx, dx_in, (size_t)sample_n * sizeof(float));
    memcpy(work_dy, dy_in, (size_t)sample_n * sizeof(float));
    memcpy(work_dz, dz_in, (size_t)sample_n * sizeof(float));

    if (zone_id == CENTER_ZONE_ID) {
        demean_center_samples(work_dx, work_dy, work_dz, sample_n);
    }

    polar_zone_t fitted = {0};
    const float *dx_ptr = work_dx;
    const float *dy_ptr = work_dy;
    const float *dz_ptr = work_dz;
    const int fit_n = sample_n;

#if ZONE_DETECT_USE_XYZ
    fit_xyz_bounds(zone_id, dx_ptr, dy_ptr, dz_ptr, fit_n, &fitted);
    expand_xyz_bounds(&fitted);
    ESP_LOGI(TAG, "点位 [%d] 完成: X[%.0f~%.0f] Y[%.0f~%.0f] Z[%.0f~%.0f] Q=%.0f (n=%d)", zone_id, fitted.x_min,
             fitted.x_max, fitted.y_min, fitted.y_max, fitted.z_min, fitted.z_max, fitted.quality_score, fit_n);
#else
    fit_polar_bounds(zone_id, dx_ptr, dy_ptr, fit_n, &fitted);
#if ZONE_POLAR_Z_GATE
    fit_z_bounds(zone_id, dz_ptr, fit_n, &fitted);
    expand_calibrated_bounds(&fitted);
    ESP_LOGI(TAG, "点位 [%d] 完成: R[%.0f~%.0f] TH[%.1f~%.1f] Z[%.0f~%.0f] Q=%.0f (n=%d)", zone_id, fitted.r_min,
             fitted.r_max, fitted.th_min, fitted.th_max, fitted.z_min, fitted.z_max, fitted.quality_score, fit_n);
#else
    expand_calibrated_bounds(&fitted);
    ESP_LOGI(TAG, "点位 [%d] 完成: R[%.0f~%.0f] TH[%.1f~%.1f] Q=%.0f (n=%d)", zone_id, fitted.r_min, fitted.r_max,
             fitted.th_min, fitted.th_max, fitted.quality_score, fit_n);
#endif
#endif
    if (zone_id == CENTER_ZONE_ID) {
        /* 中心附近的角度无定义，使用去均值后的三轴散布评估采样质量。 */
        float variance = 0.0f;
        for (int i = 0; i < fit_n; i++) {
            variance += dx_ptr[i] * dx_ptr[i] + dy_ptr[i] * dy_ptr[i] + dz_ptr[i] * dz_ptr[i];
        }
        const float stddev = sqrtf(variance / (float)fit_n);
        fitted.quality_score = fmaxf(0.0f, 100.0f - fmaxf(0.0f, stddev - 25.0f) * 2.0f);
    }
    if (!isfinite(fitted.quality_score) || fitted.quality_score < WIZARD_MIN_QUALITY_SCORE) {
        ESP_LOGW(TAG, "点位 [%d] 采样质量不足，重新采集", zone_id);
        return false;
    }
    s_wizard_zones[zone_id - 1] = fitted;
    return true;
}

static void finish_point(uint32_t now_ms)
{
    if (!apply_zone_fit(s_target_point, s_dx, s_dy, s_dz, s_sample_count)) {
        begin_point(s_target_point);
        return;
    }

    s_phase = ZONE_WIZARD_POINT_OK;
    s_point_ok_until_ms = now_ms + WIZARD_POINT_OK_MS;
    zone_tone_set_zone(s_target_point);
}

static int wizard_next_point(int current)
{
    for (int i = 0; i < WIZARD_POINT_COUNT - 1; i++) {
        if (WIZARD_POINT_ORDER[i] == current) {
            return WIZARD_POINT_ORDER[i + 1];
        }
    }
    return -1;
}

static bool finish_all(void)
{
    if (!s_zones_saved && !zone_storage_save_zones(s_wizard_zones, 9)) {
        ESP_LOGE(TAG, "保存 9 点标定到 NVS 失败");
        return false;
    }
    s_zones_saved = true;
    return true;
}

bool zone_wizard_complete(void)
{
    if (!s_zones_saved || !zone_storage_mark_wizard_done()) {
        return false;
    }
    calib_nvs_mark_firmware_bound();
    s_phase = ZONE_WIZARD_IDLE;
    s_target_point = 0;
    s_zones_saved = false;
    ESP_LOGI(TAG, "=== 9 点校准完成并已写入 NVS ===");
    return true;
}

void zone_wizard_start(void)
{
    memset(s_wizard_zones, 0, sizeof(s_wizard_zones));
    s_target_point = 0;
    s_sample_count = 0;
    s_stable_streak = 0;
    s_zones_saved = false;
    s_phase = ZONE_WIZARD_PROMPT;
    s_prompt_start_ms = 0;
    s_point_ok_until_ms = 0;
    ESP_LOGW(TAG, "=== 9 点校准：5→1→2→3→6→9→8→7→4（IMU 姿态补偿，磁珠保持静止）===");
}

void zone_wizard_abort(void)
{
    s_phase = ZONE_WIZARD_IDLE;
    s_target_point = 0;
    s_sample_count = 0;
    s_stable_streak = 0;
    s_zones_saved = false;
}

zone_wizard_phase_t zone_wizard_get_phase(void)
{
    return s_phase;
}

int zone_wizard_get_target_point(void)
{
    return s_target_point;
}

int zone_wizard_get_sample_count(void)
{
    return s_sample_count;
}

bool zone_wizard_feed_sample(float dx, float dy, float dz, bool window_stable, uint32_t now_ms)
{
    if (s_phase == ZONE_WIZARD_IDLE) {
        return false;
    }

    if (s_phase == ZONE_WIZARD_PROMPT) {
        if (s_prompt_start_ms == 0) {
            s_prompt_start_ms = now_ms;
        }
        if ((now_ms - s_prompt_start_ms) >= WIZARD_PROMPT_MS) {
            begin_point(WIZARD_POINT_ORDER[0]);
        }
        return false;
    }

    if (s_phase == ZONE_WIZARD_POINT_OK) {
        if (now_ms >= s_point_ok_until_ms) {
            const int next_pt = wizard_next_point(s_target_point);
            if (next_pt < 0) {
                return finish_all();
            }
            begin_point(next_pt);
        }
        return false;
    }

    if (s_phase != ZONE_WIZARD_COLLECT) {
        return false;
    }

    if (!window_stable || !isfinite(dx) || !isfinite(dy) || !isfinite(dz)) {
        s_sample_count = 0;
        s_stable_streak = 0;
        return false;
    }

    s_stable_streak++;
    if (s_stable_streak < WIZARD_STABLE_STREAK_NEED) {
        return false;
    }

    if (s_sample_count < WIZARD_SAMPLES_TARGET) {
        s_dx[s_sample_count] = dx;
        s_dy[s_sample_count] = dy;
        s_dz[s_sample_count] = dz;
        s_sample_count++;
        if (s_sample_count == 1) {
            ESP_LOGI(TAG, "点位 [%d] 开始采样...", s_target_point);
        } else if ((s_sample_count % 24) == 0) {
            ESP_LOGI(TAG, "点位 [%d] %d/%d", s_target_point, s_sample_count, WIZARD_SAMPLES_TARGET);
        }
    }

    if (s_sample_count >= WIZARD_SAMPLES_TARGET) {
        finish_point(now_ms);
    }

    return false;
}
