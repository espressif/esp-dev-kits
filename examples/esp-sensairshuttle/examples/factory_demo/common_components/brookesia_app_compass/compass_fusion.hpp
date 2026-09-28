/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include "compass_heading.hpp"
#include <algorithm>

namespace esp_brookesia::apps::compass_fusion {
using compass_heading::Vec3;
struct Quaternion {
    float w = 1, x = 0, y = 0, z = 0;
};
inline Quaternion multiply(Quaternion a, Quaternion b)
{
    return {a.w*b.w - a.x*b.x - a.y*b.y - a.z * b.z,
            a.w*b.x + a.x*b.w + a.y*b.z - a.z * b.y,
            a.w*b.y - a.x*b.z + a.y*b.w + a.z * b.x,
            a.w*b.z + a.x*b.y - a.y*b.x + a.z * b.w};
}
inline Quaternion conjugate(Quaternion q)
{
    return {q.w, -q.x, -q.y, -q.z};
}
inline Quaternion normalized(Quaternion q)
{
    const float n = std::sqrt(q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z);
    return n > 1e-9f && std::isfinite(n) ? Quaternion{q.w / n, q.x / n, q.y / n, q.z / n}: Quaternion{};
}
inline Vec3 rotate(Quaternion q, Vec3 v)
{
    const auto r = multiply(multiply(q, {0, v.x, v.y, v.z}), conjugate(q));
    return {r.x, r.y, r.z};
}
inline Quaternion delta(Vec3 radians)
{
    const float angle = std::sqrt(compass_heading::dot(radians, radians));
    const float k = angle > 1e-7f ? std::sin(angle * .5f) / angle : .5f;
    return {std::cos(angle * .5f), radians.x * k, radians.y * k, radians.z * k};
}
inline Quaternion fromUp(Vec3 up)
{
    Vec3 u;
    if (!compass_heading::normalize(up, u)) {
        return {};
    }
    if (u.z < -.9999f) {
        return {0, 1, 0, 0};
    }
    return normalized({1 + u.z, u.y, -u.x, 0});
}

// Quaternion complementary AHRS, body -> ENU. Gyro propagates every IMU frame;
// accelerometer corrects inclination, validated magnetometer corrects yaw only.
// This is an independent implementation, not a copy of the x-io Fusion library.
class Filter {
public:
    void reset()
    {
        q_ = {};
        initialized_ = false;
        north_ = false;
        rejected_ = false;
        reject_s_ = 0;
    }
    bool update(Vec3 gyro_dps, Vec3 acc_g, float dt)
    {
        if (!std::isfinite(dt) || dt <= 0 || dt > .1f ||
                !std::isfinite(gyro_dps.x) || !std::isfinite(gyro_dps.y) || !std::isfinite(gyro_dps.z) ||
                std::max({std::fabs(gyro_dps.x), std::fabs(gyro_dps.y), std::fabs(gyro_dps.z)}) > 1960) {
            reset(); return false;
        }
        const float norm = std::sqrt(compass_heading::dot(acc_g, acc_g));
        const bool acc_ok = std::isfinite(norm) && norm > .85f && norm < 1.15f;
        if (!initialized_) {
            if (!acc_ok) {
                return false;
            }
            q_ = fromUp(acc_g); initialized_ = true;
        }
        constexpr float rad = .017453292519943295f;
        q_ = normalized(multiply(q_, delta({gyro_dps.x*dt * rad, gyro_dps.y*dt * rad, gyro_dps.z*dt * rad})));
        if (acc_ok) {
            Vec3 measured{acc_g.x / norm, acc_g.y / norm, acc_g.z / norm};
            const Vec3 world_up = rotate(q_, measured);
            // Reject large inconsistent acceleration; gyro continues through it.
            if (world_up.z > .94f) {
                const float gain = -std::expm1(-dt / .35f);
                q_ = normalized(multiply(delta({world_up.y * gain, -world_up.x * gain, 0}), q_));
            }
        }
        return true;
    }
    bool correctMagnetic(Vec3 magnetic, float dt)
    {
        if (!initialized_ || !std::isfinite(dt) || dt <= 0 || dt > .5f) {
            return false;
        }
        Vec3 unit;
        if (!compass_heading::normalize(magnetic, unit)) {
            return false;
        }
        const Vec3 world = rotate(q_, unit);
        if (world.x * world.x + world.y * world.y < .01f) {
            rejected_ = true;
            return false;
        }
        const float correction = std::atan2(world.x, world.y);
        // Brief >30° disagreements are treated as disturbance. A lock that stays
        // more than 30° off for 2 s is dropped; the next sample re-locks.
        constexpr float kMaxYawCorrectionRad = .5235988f;
        constexpr float kYawRejectRecoverS = 2.0f;
        if (north_ && std::fabs(correction) > kMaxYawCorrectionRad) {
            reject_s_ += dt;
            if (reject_s_ >= kYawRejectRecoverS) {
                north_ = false;
                reject_s_ = 0;
            }
            rejected_ = true;
            return false;
        }
        const float gain = north_ ? -std::expm1(-dt / .6f) : 1.0f;
        q_ = normalized(multiply(delta({0, 0, correction * gain}), q_));
        north_ = true; rejected_ = false; reject_s_ = 0; return true;
    }
    bool heading(float &degrees) const
    {
        if (!initialized_) {
            return false;
        }
        const Vec3 forward = rotate(q_, {0, 1, 0});
        if (forward.x * forward.x + forward.y * forward.y < .01f) {
            return false;
        }
        degrees = compass_heading::wrap_degrees(std::atan2(forward.x, forward.y) * 57.29577951308232f);
        return std::isfinite(degrees);
    }
    Vec3 up() const
    {
        return rotate(conjugate(q_), {0, 0, 1});
    }
    Quaternion quaternion() const
    {
        return q_;
    }
    bool northReferenced() const
    {
        return north_;
    }
    bool magneticRejected() const
    {
        return rejected_;
    }
private:
    Quaternion q_{};
    bool initialized_ = false, north_ = false, rejected_ = false;
    float reject_s_ = 0;
};
} // namespace esp_brookesia::apps::compass_fusion
