/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once
#include "compass_board_frame.hpp"
#include "compass_calibration.hpp"
#include <array>

namespace esp_brookesia::apps::compass_axis_alignment {
using compass_heading::Vec3;
using compass_board_frame::AxisMap;
struct Result {
    AxisMap map{1, 2, 3};
    float deviation = 1;
};

// The PCB defines the mapping. Paired gravity observations check magnetic
// consistency; they must never select a different mounting to fit noisy data.
inline bool check(const compass_calibration::SampleBuffer &samples,
                  const std::array<Vec3, compass_calibration::SampleBuffer::capacity> &up,
                  const compass_calibration::Result &model, AxisMap map, Result &result)
{
    if (samples.size() < 320 || !compass_calibration::isValidModel(model) || map.determinant() != 1) {
        return false;
    }
    double sum = 0, square = 0;
    unsigned count = 0;
    for (std::size_t i = 0; i < samples.size(); ++i) {
        const auto sample = samples[i];
        const float centered[] = {sample.x - model.offset[0], sample.y - model.offset[1], sample.z - model.offset[2]};
        float corrected[3] {};
        for (int r = 0; r < 3; ++r) {
            for (int c = 0; c < 3; ++c) {
                corrected[r] += model.matrix[r][c] * centered[c];
            }
        }
        Vec3 m, u;
        const Vec3 v{corrected[0], corrected[1], corrected[2]};
        const float norm = std::sqrt(compass_heading::dot(v, v));
        if (std::fabs(norm / model.field_norm - 1) > .12f ||
                !compass_heading::normalize(v, m) || !compass_heading::normalize(up[i], u)) {
            continue;
        }
        const float dot = compass_heading::dot(u, map.apply(m));
        sum += dot; square += dot * dot; ++count;
    }
    if (count < 280) {
        return false;
    }
    const double mean = sum / count;
    const float deviation = std::sqrt(std::max(0.0, square / count - mean * mean));
    if (deviation > .035f || std::fabs(mean) > .95f) {
        return false;
    }
    result = {map, deviation};
    return true;
}
} // namespace esp_brookesia::apps::compass_axis_alignment
