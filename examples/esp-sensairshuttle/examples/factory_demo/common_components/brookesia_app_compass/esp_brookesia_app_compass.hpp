/*
 * SPDX-FileCopyrightText: 2025-2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ESP_BROOKESIA_APP_COMPASS_HPP
#define ESP_BROOKESIA_APP_COMPASS_HPP

#include "bmi270_api.h"
#include "boost/thread.hpp"
#include "brookesia/system_phone/app.hpp"
#include "compass_mag.hpp"
#include "compass_heading.hpp"
#include "compass_fusion.hpp"
#include "compass_imu_calibration.hpp"
#include "driver/i2c_master.h"
#include "lvgl.h"
#include <atomic>
#include <cstdint>
#include <mutex>
#include <memory>

namespace esp_brookesia::apps {

/* Magnetometer calibration structure */
struct mag_calibration_t {
    float hard_iron[3] {};
    float soft_iron[3][3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
    float field_norm = 0;
    float fit_error = 0;
    int32_t axes[3] = {1, 2, 3};
    float axis_error = 0;
    bool calibrated = false;
};

class Compass: public systems::phone::App {
public:
    Compass(const Compass &) = delete;
    Compass(Compass &&) = delete;
    Compass &operator=(const Compass &) = delete;
    Compass &operator=(Compass &&) = delete;

    ~Compass();

    static Compass *requestInstance();
    void setCorrect(bool correct);
    void externalBack();

    // NVS storage methods
    bool saveCalibrationToNVS();
    bool loadCalibrationFromNVS();

    // Async NVS operations (for UI thread safety)
    void saveCalibrationToNVSAsync();

protected:
    bool run() override;
    bool back() override;
    bool pause() override;
    bool close() override;
    bool init() override;
    bool deinit() override;
    bool resume() override;
    bool cleanResource() override;
    void createCompassUI();

private:
    inline static Compass *_instance = nullptr;
    Compass();

    bool initSensors();
    bool deinitSensors();
    bool initMagnetometer();
    bool readMagnetometer(float data[3]);
    bool startSensorTask();
    void stopSensorTask();
    struct MagneticCalibration;
    void collectMagneticCalibration(const float raw[3], compass_heading::Vec3 up, compass_heading::Vec3 gyro, int64_t now);
    bool calibrateGyroscope();
    bool calibrationActive() const;
    void updateSensorData();
    void bmmDataThread();
    void updateCompassDisplay();
    void updateUI();

    enum class UiState {
        Preparing, Ready, SensorError, Motion, UnobservablePose, GyroCalibration
    };

    // Worker-owned state, reset for every run/resume.
    compass_fusion::Filter fusion_;
    compass_heading::NorthReference north_reference_;
    compass_imu_calibration::StationaryWindow gyro_stationary_;
    int64_t last_valid_sample_us_ = 0;
    int64_t last_fresh_sample_us_ = 0;
    int64_t last_magnetic_sample_us_ = 0;
    int64_t last_magnetic_accept_us_ = 0;
    int64_t next_magnetic_fit_us_ = 0;

    bool is_initialized_ = false;
    // BMI270 IMU + ShuttleBoard BMM350 magnetometer
    bmi270_handle_t bmi_handle_;
    i2c_master_bus_handle_t i2c_bus_;
    struct bmi2_dev *bmi2_dev_;
    compass_mag_t *mag_ = nullptr;

    // Calibration data
    mag_calibration_t mag_cal_;
    compass_imu_calibration::Model imu_cal_;
    mutable std::mutex calibration_mutex_;
    std::mutex nvs_mutex_;

    // Current sensor data
    std::atomic<float> current_heading_;
    std::atomic<bool> heading_is_north_{false};

    // Task handle for sensor reading
    std::atomic<bool> bmm_running_{false};
    std::atomic<UiState> ui_state_{UiState::SensorError};
    std::unique_ptr<MagneticCalibration> magnetic_samples_;
    std::atomic<bool> magnetic_fit_running_{false};
    std::atomic<uint32_t> calibration_generation_{0};
    lv_timer_t *ui_timer_ = nullptr;
    boost::thread bmm_data_thread;
    boost::thread calibration_thread_;
    boost::thread nvs_thread_;
};

} // namespace esp_brookesia::apps

#endif
