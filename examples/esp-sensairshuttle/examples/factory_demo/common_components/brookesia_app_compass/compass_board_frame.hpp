/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include "compass_heading.hpp"

namespace esp_brookesia::apps::compass_board_frame {

using compass_heading::Vec3;

// Main-board frame: X right, Y towards the LCD top edge, Z out of the screen.
// The ShuttleBoard-BMI270&BMM350 daughterboard must stay plugged in a fixed
// orientation relative to the LCD for a screen heading.
// Each signed index selects +/- sensor X=1, Y=2 or Z=3. Both sensor
// coordinate systems must be mapped into this SAME right-handed frame.
struct AxisMap {
    int x, y, z;

    static constexpr int absolute(int axis)
    {
        return axis < 0 ? -axis : axis;
    }
    constexpr int determinant() const
    {
        if (x < -3 || x > 3 || y < -3 || y > 3 || z < -3 || z > 3) {
            return 0;
        }
        const int a = absolute(x), b = absolute(y), c = absolute(z);
        if (a < 1 || a > 3 || b < 1 || b > 3 || c < 1 || c > 3 ||
                a == b || b == c || a == c) {
            return 0;
        }
        const int parity = ((a > b) + (a > c) + (b > c)) % 2 ? -1 : 1;
        return parity * (x < 0 ? -1 : 1) * (y < 0 ? -1 : 1) * (z < 0 ? -1 : 1);
    }

    Vec3 apply(Vec3 value) const
    {
        const float axes[] = {value.x, value.y, value.z};
        const auto select = [&axes](int axis) {
            return axes[absolute(axis) - 1] * (axis < 0 ? -1.0f : 1.0f);
        };
        return {select(x), select(y), select(z)};
    }
};

inline constexpr AxisMap compose(AxisMap outer, AxisMap inner)
{
    if (outer.determinant() != 1 || inner.determinant() != 1) {
        return {0, 0, 0};
    }
    const int axes[] = {inner.x, inner.y, inner.z};
    return {axes[AxisMap::absolute(outer.x) - 1] * (outer.x < 0 ? -1 : 1),
            axes[AxisMap::absolute(outer.y) - 1] * (outer.y < 0 ? -1 : 1),
            axes[AxisMap::absolute(outer.z) - 1] * (outer.z < 0 ? -1 : 1)};
}

// Previous Shuttle Compass used +X, -Y, -Z on BMI270 native axes to match the
// LCD. Keep that IMU mapping. Recheck if the daughterboard or LCD rotation
// changes.
inline constexpr AxisMap accel_map = {1, -2, -3};
static_assert(accel_map.determinant() == 1, "BMI270 screen axes must form a right-handed permutation");

// BMM350 and BMI270 sit on the same ShuttleBoard-BMI270&BMM350 PCB. Native
// magnetometer axes are treated as BMI270-native until a PCB-derived mounting
// is confirmed. Apply this AFTER hard/soft-iron correction in the BMM350 frame.
inline constexpr AxisMap mag_to_imu = {1, 2, 3};
inline constexpr AxisMap mag_map = compose(accel_map, mag_to_imu);
static_assert(mag_map.determinant() == 1, "Invalid Shuttle magnetic axis composition");

} // namespace esp_brookesia::apps::compass_board_frame
