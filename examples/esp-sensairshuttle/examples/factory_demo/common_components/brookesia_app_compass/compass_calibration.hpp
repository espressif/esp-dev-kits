/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>

namespace esp_brookesia::apps::compass_calibration {

struct Sample {
    float x, y, z;
};

// Allocate this object on the heap. Reservoir replacement keeps accepting new
// orientations after capacity is reached without favouring the latest pose.
class SampleBuffer {
public:
    static constexpr std::size_t capacity = 768;
    bool add(float x, float y, float z)
    {
        last_index_ = capacity;
        if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z) ||
                std::fabs(x) > 4000 || std::fabs(y) > 4000 || std::fabs(z) > 4000) {
            return false;
        }
        ++seen_;
        std::size_t index = size_;
        if (size_ == capacity) {
            random_ ^= random_ << 13;
            random_ ^= random_ >> 17;
            random_ ^= random_ << 5;
            index = static_cast<std::size_t>((static_cast<uint64_t>(random_) * seen_) >> 32);
            if (index >= capacity) {
                return true;
            }
        } else {
            ++size_;
        }
        samples_[index] = {x, y, z};
        last_index_ = index;
        return true;
    }
    void clear()
    {
        size_ = 0;
        seen_ = 0;
        random_ = 0x6d2b79f5U;
    }
    std::size_t size() const
    {
        return size_;
    }
    const Sample &operator[](std::size_t i) const
    {
        return samples_[i];
    }
    std::size_t lastStoredIndex() const
    {
        return last_index_;
    }

private:
    std::array<Sample, capacity> samples_{};
    std::size_t size_ = 0;
    uint32_t seen_ = 0;
    uint32_t random_ = 0x6d2b79f5U;
    std::size_t last_index_ = capacity;
};

struct Result {
    float offset[3] {};
    float matrix[3][3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
    float field_norm = 0;
    float fit_error = 0; // Relative radial RMS: 0.02 means 2 percent.
    unsigned coverage = 0; // Number of populated corrected-space octants, 0..8.
    std::size_t inlier_count = 0;
};

enum class Status {
    Ok, TooFewSamples, BadInput, InsufficientCoverage, SingularFit,
    NotPositiveDefinite, ImplausibleCorrection, PoorResidual, TooManyOutliers
};

namespace detail {

struct Model {
    double offset[3] {};
    double matrix[3][3] {};
    double field = 0;
};

struct Workspace {
    std::array<bool, SampleBuffer::capacity> included{};
    std::array<bool, SampleBuffer::capacity> seed_included{};
    std::array<double, SampleBuffer::capacity> residuals{};
};

// Jacobi eigenvectors of a real symmetric 3x3 matrix. Columns of vectors are
// orthonormal eigenvectors; this also supplies the SPD square root below.
inline bool eigen(double a[3][3], double values[3], double vectors[3][3])
{
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            vectors[i][j] = i == j ? 1 : 0;
        }
    }
    for (int iteration = 0; iteration < 40; ++iteration) {
        int p = 0, q = 1;
        if (std::fabs(a[0][2]) > std::fabs(a[p][q])) {
            p = 0;
            q = 2;
        }
        if (std::fabs(a[1][2]) > std::fabs(a[p][q])) {
            p = 1;
            q = 2;
        }
        const double norm = std::fabs(a[0][0]) + std::fabs(a[1][1]) + std::fabs(a[2][2]);
        if (!std::isfinite(norm)) {
            return false;
        }
        if (std::fabs(a[p][q]) <= 1e-12 * std::max(norm, 1e-20)) {
            for (int i = 0; i < 3; ++i) {
                values[i] = a[i][i];
            }
            return true;
        }
        const double tau = (a[q][q] - a[p][p]) / (2 * a[p][q]);
        const double t = std::copysign(1.0, tau) / (std::fabs(tau) + std::hypot(1.0, tau));
        const double c = 1 / std::sqrt(1 + t * t), s = t * c;
        const double off = a[p][q];
        a[p][p] -= t * off;
        a[q][q] += t * off;
        a[p][q] = a[q][p] = 0;
        for (int k = 0; k < 3; ++k) {
            if (k != p && k != q) {
                const double kp = a[k][p], kq = a[k][q];
                a[k][p] = a[p][k] = c * kp - s * kq;
                a[k][q] = a[q][k] = s * kp + c * kq;
            }
            const double vp = vectors[k][p], vq = vectors[k][q];
            vectors[k][p] = c * vp - s * vq;
            vectors[k][q] = s * vp + c * vq;
        }
    }
    return false;
}

inline Status estimate(const SampleBuffer &samples, const Workspace &work, bool holdout, Model &model)
{
    double mean[3] {};
    std::size_t count = 0;
    for (std::size_t i = 0; i < samples.size(); ++i) {
        if (!work.included[i] || (holdout && i % 5 == 0)) {
            continue;
        }
        mean[0] += samples[i].x; mean[1] += samples[i].y; mean[2] += samples[i].z;
        ++count;
    }
    if (count < 80) {
        return Status::TooFewSamples;
    }
    for (double &v : mean) {
        v /= count;
    }
    double covariance[3][3] {};
    for (std::size_t i = 0; i < samples.size(); ++i) {
        if (!work.included[i] || (holdout && i % 5 == 0)) {
            continue;
        }
        const double v[3] = {samples[i].x - mean[0], samples[i].y - mean[1], samples[i].z - mean[2]};
        for (int row = 0; row < 3; ++row) {
            for (int col = 0; col < 3; ++col) {
                covariance[row][col] += v[row] * v[col] / count;
            }
        }
    }
    const double scale = std::sqrt(covariance[0][0] + covariance[1][1] + covariance[2][2]);
    double values[3], vectors[3][3];
    if (!(scale > 1e-3) || !eigen(covariance, values, vectors)) {
        return Status::InsufficientCoverage;
    }
    const auto limits = std::minmax_element(values, values + 3);
    // Rotating about one fixed oblique axis produces a planar circle. Its
    // eight raw octants can look convincing, but covariance still has rank 2.
    if (*limits.first < *limits.second * 0.001) {
        return Status::InsufficientCoverage;
    }

    double normal[9][10] {};
    for (std::size_t i = 0; i < samples.size(); ++i) {
        if (!work.included[i] || (holdout && i % 5 == 0)) {
            continue;
        }
        const double x = (samples[i].x - mean[0]) / scale;
        const double y = (samples[i].y - mean[1]) / scale;
        const double z = (samples[i].z - mean[2]) / scale;
        const double terms[9] = {x * x, y * y, z * z, 2 * x * y, 2 * x * z, 2 * y * z, 2 * x, 2 * y, 2 * z};
        for (int row = 0; row < 9; ++row) {
            for (int col = 0; col < 9; ++col) {
                normal[row][col] += terms[row] * terms[col];
            }
            normal[row][9] += terms[row];
        }
    }
    double column_scale[9];
    for (int i = 0; i < 9; ++i) {
        column_scale[i] = std::sqrt(normal[i][i]);
        if (!(column_scale[i] > 1e-10)) {
            return Status::SingularFit;
        }
    }
    for (int row = 0; row < 9; ++row) {
        for (int col = 0; col < 9; ++col) {
            normal[row][col] /= column_scale[row] * column_scale[col];
        }
        normal[row][9] /= column_scale[row];
    }
    for (int col = 0; col < 9; ++col) {
        int pivot = col;
        for (int row = col + 1; row < 9; ++row) {
            if (std::fabs(normal[row][col]) > std::fabs(normal[pivot][col])) {
                pivot = row;
            }
        }
        if (std::fabs(normal[pivot][col]) < 1e-8) {
            return Status::SingularFit;
        }
        for (int j = col; j < 10; ++j) {
            std::swap(normal[col][j], normal[pivot][j]);
        }
        const double divisor = normal[col][col];
        for (int j = col; j < 10; ++j) {
            normal[col][j] /= divisor;
        }
        for (int row = 0; row < 9; ++row) {
            if (row == col) {
                continue;
            }
            const double factor = normal[row][col];
            for (int j = col; j < 10; ++j) {
                normal[row][j] -= factor * normal[col][j];
            }
        }
    }
    double p[9];
    for (int i = 0; i < 9; ++i) {
        p[i] = normal[i][9] / column_scale[i];
    }
    double a[3][3] = {{p[0], p[3], p[4]}, {p[3], p[1], p[5]}, {p[4], p[5], p[2]}};
    if (!eigen(a, values, vectors)) {
        return Status::NotPositiveDefinite;
    }
    const auto eigen_limits = std::minmax_element(values, values + 3);
    if (!(*eigen_limits.first > 1e-9)) {
        return Status::NotPositiveDefinite;
    }
    if (*eigen_limits.second / *eigen_limits.first > 64) {
        return Status::ImplausibleCorrection;
    }
    double center[3] {};
    for (int k = 0; k < 3; ++k) {
        double dot = 0;
        for (int i = 0; i < 3; ++i) {
            dot += vectors[i][k] * p[6 + i];
        }
        for (int i = 0; i < 3; ++i) {
            center[i] -= vectors[i][k] * dot / values[k];
        }
    }
    double radius_squared = 1;
    for (int i = 0; i < 3; ++i) {
        radius_squared -= center[i] * p[6 + i];
    }
    if (!(radius_squared > 0) || !std::isfinite(radius_squared)) {
        return Status::NotPositiveDefinite;
    }
    // Magnetic norm alone cannot identify a rotation or the absolute scale.
    // Select the symmetric positive-definite correction with determinant one.
    const double factor = std::pow(values[0] * values[1] * values[2], -1.0 / 6.0);
    for (int i = 0; i < 3; ++i) {
        model.offset[i] = mean[i] + scale * center[i];
        for (int j = 0; j < 3; ++j) {
            model.matrix[i][j] = 0;
            for (int k = 0; k < 3; ++k) {
                model.matrix[i][j] += vectors[i][k] * std::sqrt(values[k]) * factor * vectors[j][k];
            }
        }
    }
    model.field = scale * std::sqrt(radius_squared) * factor;
    if (!std::isfinite(model.field) || model.field < 5 || model.field > 500) {
        return Status::ImplausibleCorrection;
    }
    return Status::Ok;
}

inline double corrected(const Sample &sample, const Model &model, double v[3])
{
    const double raw[3] = {sample.x - model.offset[0], sample.y - model.offset[1], sample.z - model.offset[2]};
    for (int i = 0; i < 3; ++i) {
        v[i] = 0;
        for (int j = 0; j < 3; ++j) {
            v[i] += model.matrix[i][j] * raw[j];
        }
    }
    return std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

// A few extreme samples can make the initial unconstrained quadratic indefinite,
// before residual trimming gets a chance to run. Try bounded, deterministic
// subsets to find an SPD seed supported by at least 90% of the FULL point cloud.
// This only initializes trimming; all original final quality gates still apply.
inline bool robustSeed(const SampleBuffer &samples, Workspace &work, Model &model)
{
    uint32_t random = 0xa341316cU;
    std::size_t best_count = 0;
    double best_score = 1e30;
    Model best_model;
    constexpr double cutoff = .06;
    for (int trial = 0; trial < 32; ++trial) {
        work.included.fill(false);
        std::size_t selected = 0;
        for (int attempt = 0; attempt < 3200 && selected < 80; ++attempt) {
            random ^= random << 13; random ^= random >> 17; random ^= random << 5;
            const std::size_t i = random % samples.size();
            if (!work.included[i]) {
                work.included[i] = true;
                ++selected;
            }
        }
        if (selected < 80) {
            continue;
        }
        Model candidate;
        if (estimate(samples, work, false, candidate) != Status::Ok) {
            continue;
        }
        std::size_t count = 0;
        double score = 0;
        for (std::size_t i = 0; i < samples.size(); ++i) {
            double v[3];
            const double error = std::fabs(corrected(samples[i], candidate, v) / candidate.field - 1);
            work.included[i] = std::isfinite(error) && error <= cutoff;
            if (work.included[i]) {
                ++count;
            }
            score += std::isfinite(error) ? std::min(error * error, cutoff * cutoff) : cutoff * cutoff;
        }
        if (count * 10 < samples.size() * 9) {
            continue;
        }
        if (count > best_count || (count == best_count && score < best_score)) {
            best_count = count; best_score = score; best_model = candidate;
            work.seed_included = work.included;
        }
        if (best_count * 100 >= samples.size() * 95 && best_score / samples.size() < .0001) {
            break;
        }
    }
    if (best_count * 10 < samples.size() * 9) {
        return false;
    }
    work.included = work.seed_included;
    model = best_model;
    return true;
}

} // namespace detail

// Stored models need the same mathematical checks as newly fitted models.
// Coverage/error metadata is deliberately excluded so an NVS adapter can
// validate the numerical model before interpreting its versioned metadata.
inline bool isValidModel(const Result &result)
{
    if (!std::isfinite(result.field_norm) || result.field_norm < 5 || result.field_norm > 500) {
        return false;
    }
    double matrix[3][3];
    for (int i = 0; i < 3; ++i) {
        if (!std::isfinite(result.offset[i]) || std::fabs(result.offset[i]) > 4000) {
            return false;
        }
        for (int j = 0; j < 3; ++j) {
            matrix[i][j] = result.matrix[i][j];
            if (!std::isfinite(matrix[i][j]) ||
                    std::fabs(matrix[i][j] - result.matrix[j][i]) > 1e-5) {
                return false;
            }
        }
    }
    double values[3], vectors[3][3];
    if (!detail::eigen(matrix, values, vectors)) {
        return false;
    }
    const auto limits = std::minmax_element(values, values + 3);
    const double determinant = values[0] * values[1] * values[2];
    return *limits.first > 0 && *limits.second / *limits.first <= 8.001 &&
           std::isfinite(determinant) && std::fabs(determinant - 1) < 0.02;
}

// Full 3D ellipsoid fitting, implemented from the mathematical model in NXP
// AN4246. No NXP library code is used. Failure leaves the caller's result intact.
inline Status fit(const SampleBuffer &samples, Result &result)
{
    if (samples.size() < 120) {
        return Status::TooFewSamples;
    }
    auto work = std::unique_ptr<detail::Workspace>(new (std::nothrow) detail::Workspace);
    if (!work) {
        return Status::BadInput;
    }
    work->included.fill(true);
    detail::Model model;
    Status status = detail::estimate(samples, *work, false, model);
    if (status == Status::NotPositiveDefinite || status == Status::SingularFit ||
            status == Status::ImplausibleCorrection) {
        const Status initial_status = status;
        if (!detail::robustSeed(samples, *work, model)) {
            return initial_status;
        }
        status = detail::estimate(samples, *work, false, model);
    }
    if (status != Status::Ok) {
        return status;
    }
    for (int pass = 0; pass < 3; ++pass) {
        std::size_t count = 0;
        for (std::size_t i = 0; i < samples.size(); ++i) {
            if (!work->included[i]) {
                continue;
            }
            double v[3];
            work->residuals[count++] = std::fabs(detail::corrected(samples[i], model, v) / model.field - 1);
        }
        std::sort(work->residuals.begin(), work->residuals.begin() + count);
        const double cutoff = std::max(0.035, 5 * work->residuals[count / 2]);
        std::size_t excluded = 0;
        bool changed = false;
        for (std::size_t i = 0; i < samples.size(); ++i) {
            double v[3];
            const double residual = std::fabs(detail::corrected(samples[i], model, v) / model.field - 1);
            const bool include = residual <= cutoff;
            if (work->included[i] != include) {
                work->included[i] = include;
                changed = true;
            }
            if (!work->included[i]) {
                ++excluded;
            }
        }
        // A biased initial fit can temporarily reject good points. Reconsider
        // every sample after refitting; the final acceptance limit is stricter.
        if (excluded * 100 > samples.size() * 35) {
            return Status::TooManyOutliers;
        }
        if (!changed) {
            break;
        }
        status = detail::estimate(samples, *work, false, model);
        if (status != Status::Ok) {
            return status;
        }
    }
    std::size_t excluded = 0;
    for (std::size_t i = 0; i < samples.size(); ++i) {
        if (!work->included[i]) {
            ++excluded;
        }
    }
    if (excluded * 10 > samples.size()) {
        return Status::TooManyOutliers;
    }
    // Reserve every fifth retained sample from the final fit. Residual quality
    // is checked on that holdout too, not just on the least-squares training set.
    status = detail::estimate(samples, *work, true, model);
    if (status != Status::Ok) {
        return status;
    }
    unsigned octants[8] {};
    double minimum[3] = {1, 1, 1}, maximum[3] = {-1, -1, -1};
    double squared_error = 0, holdout_error = 0, maximum_error = 0;
    std::size_t count = 0, holdout_count = 0;
    for (std::size_t i = 0; i < samples.size(); ++i) {
        if (!work->included[i]) {
            continue;
        }
        double v[3];
        const double radius = detail::corrected(samples[i], model, v);
        if (!(radius > 1e-6) || !std::isfinite(radius)) {
            return Status::BadInput;
        }
        const double error = std::fabs(radius / model.field - 1);
        squared_error += error * error;
        maximum_error = std::max(maximum_error, error);
        ++count;
        if (i % 5 == 0) {
            holdout_error += error * error;
            ++holdout_count;
        }
        unsigned octant = 0;
        for (int axis = 0; axis < 3; ++axis) {
            const double direction = v[axis] / radius;
            if (direction >= 0) {
                octant |= 1U << axis;
            }
            minimum[axis] = std::min(minimum[axis], direction);
            maximum[axis] = std::max(maximum[axis], direction);
        }
        ++octants[octant];
    }
    Result candidate;
    for (unsigned n : octants) {
        if (n >= 2) {
            ++candidate.coverage;
        }
    }
    if (candidate.coverage < 6) {
        return Status::InsufficientCoverage;
    }
    for (int axis = 0; axis < 3; ++axis) {
        if (minimum[axis] > -0.6 || maximum[axis] < 0.6) {
            return Status::InsufficientCoverage;
        }
    }
    if (count < 120 || holdout_count < 16) {
        return Status::TooFewSamples;
    }
    candidate.fit_error = static_cast<float>(std::sqrt(squared_error / count));
    if (candidate.fit_error > 0.03 || std::sqrt(holdout_error / holdout_count) > 0.04 || maximum_error > 0.12) {
        return Status::PoorResidual;
    }
    for (int i = 0; i < 3; ++i) {
        candidate.offset[i] = static_cast<float>(model.offset[i]);
        for (int j = 0; j < 3; ++j) {
            candidate.matrix[i][j] = static_cast<float>(model.matrix[i][j]);
        }
    }
    candidate.field_norm = static_cast<float>(model.field);
    candidate.inlier_count = count;
    if (!isValidModel(candidate)) {
        return Status::ImplausibleCorrection;
    }
    result = candidate;
    return Status::Ok;
}

} // namespace esp_brookesia::apps::compass_calibration
