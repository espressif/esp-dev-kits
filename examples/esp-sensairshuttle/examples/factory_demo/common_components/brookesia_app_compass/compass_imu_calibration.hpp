/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include "compass_heading.hpp"
#include <array>
#include <algorithm>

namespace esp_brookesia::apps::compass_imu_calibration {
using compass_heading::Vec3;

struct Model {
    Vec3 gyro_bias{}; // degrees/second, in BMI270 native axes
    Vec3 accel_bias{}; // g, in BMI270 native axes
    Vec3 accel_scale{1, 1, 1};
    bool gyro_valid = false;
    bool accel_valid = false;
};

inline bool finite(Vec3 v)
{
    return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
}

inline Vec3 subtract(Vec3 a, Vec3 b)
{
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

inline bool valid(const Model &m)
{
    return finite(m.gyro_bias) && finite(m.accel_bias) && finite(m.accel_scale) &&
           std::max({std::fabs(m.gyro_bias.x), std::fabs(m.gyro_bias.y), std::fabs(m.gyro_bias.z)}) <= 5 &&
           std::max({std::fabs(m.accel_bias.x), std::fabs(m.accel_bias.y), std::fabs(m.accel_bias.z)}) <= .2f &&
           std::min({m.accel_scale.x, m.accel_scale.y, m.accel_scale.z}) >= .8f &&
           std::max({m.accel_scale.x, m.accel_scale.y, m.accel_scale.z}) <= 1.25f;
}

inline Vec3 acceleration(const Model &m, Vec3 g)
{
    if (!m.accel_valid) {
        return g;
    }
    return {(g.x - m.accel_bias.x)*m.accel_scale.x, (g.y - m.accel_bias.y)*m.accel_scale.y,
            (g.z - m.accel_bias.z)*m.accel_scale.z};
}

inline Vec3 gyroscope(const Model &m, Vec3 dps)
{
    return m.gyro_valid ? subtract(dps, m.gyro_bias) : dps;
}

// Explicit user-held-still window. Constant slow rotation cannot be distinguished
// from bias by a gyro alone, so the UI must ask the user to keep the board still.
class StationaryWindow {
public:
    explicit StationaryWindow(float minimum_acceleration = .8f, float maximum_acceleration = 1.2f)
        : minimum_norm2_(minimum_acceleration * minimum_acceleration),
          maximum_norm2_(maximum_acceleration * maximum_acceleration) {}
    void reset()
    {
        count_ = 0;
        elapsed_ = 0;
        sum_a_ = {};
        sum_g_ = {};
        square_a_ = {};
        square_g_ = {};
    }
    bool add(Vec3 acc, Vec3 gyro, float dt)
    {
        const float norm2 = compass_heading::dot(acc, acc);
        if (!finite(acc) || !finite(gyro) || !std::isfinite(dt) || dt <= 0) {
            return reject();
        }
        if (dt > .05f) {
            return reject();
        }
        if (norm2 < minimum_norm2_ || norm2 > maximum_norm2_) {
            return reject();
        }
        if (compass_heading::dot(gyro, gyro) > 25) {
            return reject();
        }
        const float a[] = {acc.x, acc.y, acc.z}, g[] = {gyro.x, gyro.y, gyro.z};
        for (int i = 0; i < 3; ++i) {
            sum_a_[i] += a[i]; sum_g_[i] += g[i];
            square_a_[i] += a[i] * a[i]; square_g_[i] += g[i] * g[i];
        }
        ++count_; elapsed_ += dt;
        if (count_ >= 20) {
            for (int i = 0; i < 3; ++i) {
                const double av = sum_a_[i] / count_, gv = sum_g_[i] / count_;
                if (square_a_[i] / count_ - av * av > .0004) {
                    return reject();
                }
                if (square_g_[i] / count_ - gv * gv > .25) {
                    return reject();
                }
            }
        }
        return true;
    }
    bool ready(float seconds) const
    {
        return count_ >= 80 && elapsed_ >= seconds;
    }
    Vec3 gyroMean() const
    {
        return mean(sum_g_);
    }
private:
    bool reject()
    {
        reset();
        return false;
    }
    Vec3 mean(const std::array<double, 3> &s) const
    {
        return count_ ? Vec3{float(s[0] / count_), float(s[1] / count_), float(s[2] / count_)} : Vec3{};
    }
    unsigned count_ = 0;
    float elapsed_ = 0;
    std::array<double, 3> sum_a_{}, sum_g_{}, square_a_{}, square_g_{};
    float minimum_norm2_, maximum_norm2_;
};

} // namespace esp_brookesia::apps::compass_imu_calibration
