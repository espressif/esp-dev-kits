/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once
#include "compass_imu_calibration.hpp"
#include "compass_calibration.hpp"
#include "compass_board_frame.hpp"
#include <cstring>

namespace esp_brookesia::apps::compass_nine_axis_storage {
struct Record {
    uint32_t version = 1, byte_size = 132;
    float offset[3] {};
    float matrix[3][3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
    float field_norm = 0, fit_error = 0;
    uint32_t flags = 0; // gyro=1, accelerometer=2, magnetometer+axis mapping=4
    float gyro_bias[3] {}, accel_bias[3] {}, accel_scale[3] = {1, 1, 1};
    int32_t axes[3] = {compass_board_frame::mag_map.x, compass_board_frame::mag_map.y,
                       compass_board_frame::mag_map.z
                      };
    float axis_error = 0;
    int32_t imu_axes[3] = {compass_board_frame::accel_map.x, compass_board_frame::accel_map.y,
                           compass_board_frame::accel_map.z
                          };
};
static_assert(sizeof(Record) == 132);
inline compass_imu_calibration::Model imuModel(const Record &r)
{
    return {{r.gyro_bias[0], r.gyro_bias[1], r.gyro_bias[2]},
        {r.accel_bias[0], r.accel_bias[1], r.accel_bias[2]},
        {r.accel_scale[0], r.accel_scale[1], r.accel_scale[2]},
        bool(r.flags & 1), bool(r.flags & 2)};
}
inline bool valid(const Record &r)
{
    if (r.version != 1 || r.byte_size != sizeof(r) || (r.flags & ~7U) ||
            r.imu_axes[0] != compass_board_frame::accel_map.x ||
            r.imu_axes[1] != compass_board_frame::accel_map.y ||
            r.imu_axes[2] != compass_board_frame::accel_map.z ||
            !compass_imu_calibration::valid(imuModel(r))) {
        return false;
    }
    if (!(r.flags & 4)) {
        return true;
    }
    // Six-face accelerometer calibration is optional. Factory acceleration
    // scaling can be used with a valid gyro bias and magnetic calibration.
    if (!(r.flags & 1)) {
        return false;
    }
    if (r.axes[0] != compass_board_frame::mag_map.x || r.axes[1] != compass_board_frame::mag_map.y ||
            r.axes[2] != compass_board_frame::mag_map.z) {
        return false;
    }
    compass_calibration::Result mag;
    std::memcpy(mag.offset, r.offset, sizeof(mag.offset));
    std::memcpy(mag.matrix, r.matrix, sizeof(mag.matrix));
    mag.field_norm = r.field_norm;
    const compass_board_frame::AxisMap axes{r.axes[0], r.axes[1], r.axes[2]};
    return compass_calibration::isValidModel(mag) && axes.determinant() == 1 &&
           std::isfinite(r.fit_error) && r.fit_error >= 0 && r.fit_error <= .03001f &&
           std::isfinite(r.axis_error) && r.axis_error >= 0 && r.axis_error <= .03501f;
}
} // namespace esp_brookesia::apps::compass_nine_axis_storage
