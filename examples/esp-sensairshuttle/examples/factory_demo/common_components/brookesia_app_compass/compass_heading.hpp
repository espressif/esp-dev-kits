/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <cmath>

namespace esp_brookesia::apps::compass_heading {

struct Vec3 {
    float x;
    float y;
    float z;
};

inline float dot(Vec3 a, Vec3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline bool normalize(Vec3 input, Vec3 &output)
{
    if (!std::isfinite(input.x) || !std::isfinite(input.y) || !std::isfinite(input.z)) {
        return false;
    }
    const double length = std::sqrt(static_cast<double>(input.x) * input.x +
                                    static_cast<double>(input.y) * input.y +
                                    static_cast<double>(input.z) * input.z);
    if (length < 1e-6) {
        return false;
    }
    output = {static_cast<float>(input.x / length), static_cast<float>(input.y / length),
              static_cast<float>(input.z / length)
             };
    return true;
}

inline float wrap_degrees(float angle)
{
    angle = std::fmod(angle, 360.0f);
    if (angle < 0.0f) {
        angle += 360.0f;
    }
    // A tiny negative angle can round up to exactly 360 in float arithmetic.
    return angle >= 360.0f ? 0.0f : angle;
}

// LVGL rotates positive angles clockwise. A north-pointing icon initially
// facing screen-up must rotate opposite to the device heading.
inline float pointer_angle_deg(float heading_deg)
{
    return -wrap_degrees(heading_deg);
}

// Follow relative motion until north is available, then remove the temporary
// display offset smoothly. The final output is magnetic heading, never a
// permanently arbitrary reference. Gyro responsiveness itself is unchanged.
class NorthReference {
public:
    void reset()
    {
        initialized_ = false;
        offset_ = 0;
    }
    void preserveCorrection(float before, float after)
    {
        if (initialized_ && std::isfinite(before) && std::isfinite(after)) {
            offset_ = wrap_degrees(offset_ + after - before);
        }
    }
    bool update(float heading, float dt, bool magnetic_reliable, float &output)
    {
        if (!std::isfinite(heading) || !std::isfinite(dt) || dt <= 0 || dt > .1f) {
            return false;
        }
        if (!initialized_) {
            offset_ = magnetic_reliable ? 0 : heading;
            initialized_ = true;
        }
        if (magnetic_reliable) {
            float offset = std::remainder(offset_, 360.0f);
            offset *= std::exp(-dt / 1.0f);
            if (std::fabs(offset) < .1f) {
                offset = 0;
            }
            offset_ = wrap_degrees(offset);
        }
        output = wrap_degrees(heading - offset_);
        return true;
    }
    bool aligned() const
    {
        return initialized_ && std::fabs(std::remainder(offset_, 360.0f)) < .5f;
    }
private:
    float offset_ = 0;
    bool initialized_ = false;
};

} // namespace esp_brookesia::apps::compass_heading
