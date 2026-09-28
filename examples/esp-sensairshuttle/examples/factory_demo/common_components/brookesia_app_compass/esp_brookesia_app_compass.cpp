/*
 * SPDX-FileCopyrightText: 2025-2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "driver/gpio.h"
#include "esp_brookesia_app_compass.hpp"
#include "compass_board_frame.hpp"
#include "compass_calibration.hpp"
#include "compass_axis_alignment.hpp"
#include "compass_nine_axis_storage.hpp"
#include "esp_err.h"
#include "esp_lib_utils.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "ui/ui.h"
#include "esp_board_manager.h"
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <memory>
#include <new>

static const char *TAG = "CompassApp";

using namespace esp_brookesia::systems::phone;
using namespace esp_brookesia::gui;

#define APP_NAME "Compass"

#define SAMPLE_RATE_MS 10 // Poll interval; fusion uses measured sample timestamps.
#define SENSOR_STALE_TIMEOUT_US (3LL * 1000000)

#define I2C_MASTER_SDO_IO GPIO_NUM_9

static void configure_imu_sdo_from_board(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << I2C_MASTER_SDO_IO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "IMU SDO GPIO%d config failed: %s", (int)I2C_MASTER_SDO_IO, esp_err_to_name(ret));
        return;
    }
    gpio_set_level(I2C_MASTER_SDO_IO, 0);
    ESP_LOGI(TAG, "IMU SDO GPIO%d pulled low for address select", (int)I2C_MASTER_SDO_IO);
}

// NVS storage configuration
/* v3/v4 magnetic-only records remain untouched; v5 stores all nine-axis stages. */
#define NVS_NAMESPACE "compass_cal_v5"
#define NVS_KEY_MODEL "model"

LV_IMG_DECLARE(img_app_compass);

namespace esp_brookesia::apps {

constexpr esp_brookesia::systems::base::App::Config CORE_DATA = {
    .name = APP_NAME,
    .launcher_icon = esp_brookesia::gui::StyleImage::IMAGE(&img_app_compass),
    .screen_size = esp_brookesia::gui::StyleSize::RECT_PERCENT(100, 100),
    .flags = {
        .enable_default_screen = 1,
        .enable_recycle_resource = 1,
        .enable_resize_visual_area = 1,
    },
};
constexpr App::Config APP_DATA = {
    .app_launcher_page_index = 0,
    .flags = {
        .enable_navigation_gesture = 1,
    },
};

namespace {

using CalibrationRecord = compass_nine_axis_storage::Record;

bool usableCalibrationModel(const compass_calibration::Result &model)
{
    return compass_calibration::isValidModel(model) && std::isfinite(model.fit_error) &&
           model.fit_error >= 0.0f && model.fit_error <= 0.03001f;
}

} // namespace

struct Compass::MagneticCalibration {
    compass_calibration::SampleBuffer magnetic;
    std::array<compass_heading::Vec3, compass_calibration::SampleBuffer::capacity> up{};
    int64_t started_us = 0;
    unsigned observed_axes = 0;
    uint32_t generation = 0;
};

Compass::Compass()
    : App(CORE_DATA, APP_DATA)
    , bmi_handle_(nullptr)
    , i2c_bus_(nullptr)
    , bmi2_dev_(nullptr)
    , mag_(nullptr)
    , mag_cal_{}
    , current_heading_(0.0f)
    , bmm_running_(false)
{
    ESP_LOGI(TAG, "Compass app constructor");
}

Compass::~Compass()
{
    ESP_UTILS_LOG_TRACE_GUARD_WITH_THIS();
    ESP_LOGI(TAG, "Compass app destructor");
    deinit();
}

Compass *Compass::requestInstance()
{
    if (_instance == nullptr) {
        _instance = new Compass();
    }
    return _instance;
}

void Compass::setCorrect(bool correct)
{
    std::lock_guard<std::mutex> guard(calibration_mutex_);
    mag_cal_.calibrated = correct;
    if (!correct) {
        imu_cal_ = {};
        ++calibration_generation_;
    }
}

void Compass::externalBack()
{
    back();
}

bool Compass::init()
{
    ESP_LOGI(TAG, "Initializing Compass app");

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Calibration storage unavailable: %s; keeping session-only calibration", esp_err_to_name(ret));
        return true;
    }

    // Load calibration data from NVS
    if (loadCalibrationFromNVS()) {
        ESP_LOGI(TAG, "Calibration data loaded from NVS");
    } else {
        ESP_LOGI(TAG, "No calibration data found in NVS, using defaults");
    }

    return true;
}

bool Compass::deinit()
{
    ESP_LOGI(TAG, "Deinitializing Compass app");

    return close();
}

bool Compass::run()
{
    ESP_LOGI(TAG, "Running Compass app");

    is_initialized_ = startSensorTask();
    createCompassUI();
    ui_timer_ = lv_timer_create([](lv_timer_t *timer) {
        static_cast<Compass *>(lv_timer_get_user_data(timer))->updateUI();
    }, 50, this);
    if (ui_timer_ == nullptr) {
        ESP_LOGE(TAG, "Failed to create compass UI timer");
        close();
        return false;
    }
    updateUI();
    return true;
}

bool Compass::back()
{
    ESP_LOGI(TAG, "Compass app back");
    ESP_UTILS_CHECK_FALSE_RETURN(notifyCoreClosed(), false, "Notify core closed failed");
    return true;
}

bool Compass::resume()
{
    ESP_LOGI(TAG, "Compass app resume");

    is_initialized_ = startSensorTask();
    if (ui_timer_) {
        lv_timer_resume(ui_timer_);
    }
    updateUI();
    return true;
}

bool Compass::pause()
{
    ESP_LOGI(TAG, "Compass app pause");

    if (ui_timer_) {
        lv_timer_pause(ui_timer_);
    }
    stopSensorTask();
    if (!deinitSensors()) {
        ESP_LOGE(TAG, "Failed to deinitialize Compass sensors");
    }
    is_initialized_ = false;
    return true;
}

bool Compass::close()
{
    ESP_LOGI(TAG, "Closing Compass app");
    if (ui_timer_) {
        lv_timer_delete(ui_timer_);
        ui_timer_ = nullptr;
    }
    stopSensorTask();
    if (nvs_thread_.joinable()) {
        nvs_thread_.join();
    }
    if (!deinitSensors()) {
        ESP_LOGE(TAG, "Failed to deinitialize Compass sensors");
    }
    is_initialized_ = false;
    // Screens stay alive until the framework finishes unloading the active one.
    return true;
}

bool Compass::cleanResource()
{
    // The framework owns all screens recorded during run() and deletes them
    // immediately after this callback. Clear aliases without deleting twice.
    ui_CompassCorrectScreen = nullptr;
    ui_CorrectImage = nullptr;
    ui_CorrectTips = nullptr;
    ui_CompassScreen = nullptr;
    ui_Pointer = nullptr;
    ui_ProgressBar = nullptr;
    ui_CalibrationTip = nullptr;
    ui_CompassTipScreen = nullptr;
    ui_CompassTipText = nullptr;
    return true;
}

void Compass::createCompassUI()
{
    compass_ui_init(is_initialized_);
}

bool Compass::startSensorTask()
{
    stopSensorTask();
    if (!initSensors()) {
        deinitSensors();
        ui_state_ = UiState::SensorError;
        return false;
    }
    ui_state_ = UiState::Preparing;
    fusion_.reset();
    north_reference_.reset();
    gyro_stationary_.reset();
    heading_is_north_ = false;
    current_heading_ = 0;
    last_valid_sample_us_ = 0;
    last_fresh_sample_us_ = esp_timer_get_time();
    last_magnetic_sample_us_ = 0;
    last_magnetic_accept_us_ = 0;
    next_magnetic_fit_us_ = 0;
    bmm_running_ = true;
    try {
        // Ellipsoid fitting and sensor logging need more than the default 3 KB.
        // Keep the extra stack local to this worker, not all application threads.
        boost::thread::attributes attributes;
        attributes.set_stack_size(8192);
        bmm_data_thread = boost::thread(attributes, [this]() {
            bmmDataThread();
        });
    } catch (const boost::thread_resource_error &error) {
        ESP_LOGE(TAG, "Failed to start sensor task: %s", error.what());
        bmm_running_ = false;
        deinitSensors();
        ui_state_ = UiState::SensorError;
        return false;
    }
    return true;
}

void Compass::stopSensorTask()
{
    {
        std::lock_guard<std::mutex> guard(calibration_mutex_);
        bmm_running_ = false;
        ++calibration_generation_;
    }
    // The worker never takes the GUI lock, so joining from a GUI event is safe.
    if (bmm_data_thread.joinable()) {
        bmm_data_thread.join();
    }
    if (calibration_thread_.joinable()) {
        calibration_thread_.join();
    }
    magnetic_fit_running_ = false;
    magnetic_samples_.reset();
}

bool Compass::initSensors()
{
    ESP_LOGI(TAG, "Initializing BMI270 + BMM350 sensors...");

    vTaskDelay(10 / portTICK_PERIOD_MS);
    configure_imu_sdo_from_board();
    vTaskDelay(10 / portTICK_PERIOD_MS);

    esp_err_t ret = esp_board_manager_get_periph_handle("i2c_master", (void **)&i2c_bus_);
    if (ret != ESP_OK || i2c_bus_ == nullptr) {
        ESP_LOGE(TAG, "Failed to get I2C master bus from Board Manager: %s", esp_err_to_name(ret));
        return false;
    }
    ESP_LOGI(TAG, "I2C master bus handle: %p", i2c_bus_);

    vTaskDelay(10 / portTICK_PERIOD_MS);

    ret = bmi270_sensor_create_from_master_bus(i2c_bus_, &bmi_handle_, bmi270_config_file, 0);
    if (ret != ESP_OK || bmi_handle_ == NULL) {
        ESP_LOGE(TAG, "BMI270 creation failed: %s", esp_err_to_name(ret));
        bmi2_dev_ = (struct bmi2_dev *)bmi_handle_;
        if (bmi_handle_ != nullptr) {
            ESP_LOGW(TAG, "BMI270 handle retained for cleanup retry");
        }
        return false;
    }

    bmi2_dev_ = (struct bmi2_dev *)bmi_handle_;
    ESP_LOGI(TAG, "BMI270 initialized successfully");

    struct bmi2_sens_config config[2];
    config[BMI2_ACCEL].type = BMI2_ACCEL;
    config[BMI2_GYRO].type = BMI2_GYRO;

    int8_t rslt = bmi2_get_sensor_config(config, 2, bmi2_dev_);
    if (rslt == BMI2_OK) {
        // Configure accelerometer
        config[BMI2_ACCEL].cfg.acc.odr = BMI2_ACC_ODR_100HZ;        // 100Hz sample rate
        config[BMI2_ACCEL].cfg.acc.range = BMI2_ACC_RANGE_4G;        // ±4G range
        config[BMI2_ACCEL].cfg.acc.bwp = BMI2_ACC_NORMAL_AVG4;        // Normal averaging
        config[BMI2_ACCEL].cfg.acc.filter_perf = BMI2_PERF_OPT_MODE; // Filter performance

        // Configure gyroscope
        config[BMI2_GYRO].cfg.gyr.odr = BMI2_GYR_ODR_100HZ;          // 100Hz sample rate
        config[BMI2_GYRO].cfg.gyr.range = BMI2_GYR_RANGE_2000;       // ±2000dps range
        config[BMI2_GYRO].cfg.gyr.bwp = BMI2_GYR_NORMAL_MODE;       // Normal filter
        config[BMI2_GYRO].cfg.gyr.noise_perf = BMI2_POWER_OPT_MODE;  // Noise performance
        config[BMI2_GYRO].cfg.gyr.filter_perf = BMI2_PERF_OPT_MODE; // Filter performance

        rslt = bmi2_set_sensor_config(config, 2, bmi2_dev_);
        if (rslt != BMI2_OK) {
            ESP_LOGE(TAG, "BMI270 sensor config failed: %d", rslt);
            return false;
        }
    } else {
        ESP_LOGE(TAG, "BMI270 get sensor config failed: %d", rslt);
        return false;
    }

    uint8_t sens_list[2] = {BMI2_ACCEL, BMI2_GYRO};
    rslt = bmi2_sensor_enable(sens_list, 2, bmi2_dev_);
    if (rslt != BMI2_OK) {
        ESP_LOGE(TAG, "BMI270 sensor enable failed: %d", rslt);
        return false;
    }
    ESP_LOGI(TAG, "BMI270 sensors enabled");

    if (!initMagnetometer()) {
        ESP_LOGW(TAG, "Magnetometer unavailable; keeping IMU relative direction available");
    }

    vTaskDelay(pdMS_TO_TICKS(20));
    struct bmi2_sens_data sensor_snapshot = {};
    float magnetic_snapshot[3] = {};
    const int8_t imu_snapshot_result = bmi2_get_sensor_data(&sensor_snapshot, bmi2_dev_);
    const bool magnetic_snapshot_ok = readMagnetometer(magnetic_snapshot);
    if (imu_snapshot_result == BMI2_OK && magnetic_snapshot_ok) {
        ESP_LOGI(
            TAG,
            "Sensor snapshot: accel=[%d,%d,%d], gyro=[%d,%d,%d], mag=[%.2f,%.2f,%.2f]",
            sensor_snapshot.acc.x, sensor_snapshot.acc.y, sensor_snapshot.acc.z,
            sensor_snapshot.gyr.x, sensor_snapshot.gyr.y, sensor_snapshot.gyr.z,
            magnetic_snapshot[0], magnetic_snapshot[1], magnetic_snapshot[2]
        );
    } else {
        ESP_LOGW(
            TAG, "Sensor snapshot unavailable: bmi270=%d, magnetometer=%s",
            imu_snapshot_result, magnetic_snapshot_ok ? "ready" : "read_failed"
        );
    }

    return true;
}

bool Compass::initMagnetometer()
{
    esp_err_t ret = compass_mag_delete(&mag_);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to clean up previous magnetometer: %s", esp_err_to_name(ret));
        return false;
    }

    compass_mag_config_t mag_cfg = {
        .i2c_bus = i2c_bus_,
        .chip = "bmm350",
        .i2c_addr = 0,
        .frequency_hz = 0,
    };

    ret = compass_mag_create(&mag_cfg, &mag_);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Magnetometer (%s) init failed: %s", mag_cfg.chip, esp_err_to_name(ret));
        return false;
    }

    ESP_LOGI(TAG, "Magnetometer backend ready: %s", compass_mag_chip_name(mag_));
    return true;
}

bool Compass::readMagnetometer(float data[3])
{
    if (mag_ == nullptr || data == nullptr) {
        return false;
    }
    return compass_mag_read(mag_, data) == ESP_OK;
}

bool Compass::deinitSensors()
{
    ESP_LOGI(TAG, "Deinitializing sensors...");

    bool success = true;
    esp_err_t ret = compass_mag_delete(&mag_);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "BMM350 deletion failed: %s", esp_err_to_name(ret));
        success = false;
    }

    if (bmi_handle_) {
        ret = bmi270_sensor_del(&bmi_handle_);
        if (ret == ESP_OK) {
            bmi2_dev_ = nullptr;
        } else {
            ESP_LOGE(TAG, "BMI270 deletion failed: %s", esp_err_to_name(ret));
            success = false;
        }
    }

    return success;
}

bool Compass::calibrationActive() const
{
    return bmm_running_;
}

bool Compass::calibrateGyroscope()
{
    compass_imu_calibration::Model model;
    {
        std::lock_guard<std::mutex> guard(calibration_mutex_);
        model = imu_cal_;
    }
    compass_imu_calibration::StationaryWindow stationary;
    ui_state_ = UiState::GyroCalibration;
    int64_t start = esp_timer_get_time(), previous = 0;
    bool gyro_ready = false;
    while (calibrationActive() && esp_timer_get_time() - start < 10000000) {
        bmi2_sens_data imu = {};
        const int64_t now = esp_timer_get_time();
        if (bmi2_get_sensor_data(&imu, bmi2_dev_) == BMI2_OK &&
                (imu.status & BMI2_DRDY_ACC_MASK) && (imu.status & BMI2_DRDY_GYR_MASK)) {
            const float dt = previous ? (now - previous) / 1000000.0f : .01f;
            previous = now;
            stationary.add({imu.acc.x / 8192.0f, imu.acc.y / 8192.0f, imu.acc.z / 8192.0f},
            {imu.gyr.x / 16.4f, imu.gyr.y / 16.4f, imu.gyr.z / 16.4f}, dt);
            if (stationary.ready(2)) {
                model.gyro_bias = stationary.gyroMean();
                model.gyro_valid = true;
                gyro_ready = compass_imu_calibration::valid(model);
                break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(SAMPLE_RATE_MS));
    }
    if (!gyro_ready || !calibrationActive()) {
        ESP_LOGW(TAG, "Gyro calibration deferred; keeping the previous/factory bias");
        return false;
    }
    {
        std::lock_guard<std::mutex> guard(calibration_mutex_);
        imu_cal_ = model;
    }
    saveCalibrationToNVS();
    ESP_LOGI(TAG, "Gyro bias accepted: [%.4f, %.4f, %.4f] dps",
             model.gyro_bias.x, model.gyro_bias.y, model.gyro_bias.z);
    return true;
}

void Compass::collectMagneticCalibration(const float raw[3], compass_heading::Vec3 up,
                                         compass_heading::Vec3 gyro, int64_t now)
{
    uint32_t generation;
    {
        std::lock_guard<std::mutex> guard(calibration_mutex_);
        if (!bmm_running_ || !imu_cal_.gyro_valid || mag_cal_.calibrated) {
            return;
        }
        generation = calibration_generation_;
    }
    const float a2 = compass_heading::dot(up, up);
    const float gyro2 = compass_heading::dot(gyro, gyro);
    // Ignore an idle board so the reservoir is not filled with one pose, and
    // ignore fast flicks that distort the magnetometer while moving.
    constexpr float kMinGyroDps = 6.0f;
    constexpr float kMaxGyroDps = 80.0f;
    if (a2 < .85f * .85f || a2 > 1.15f * 1.15f || gyro2 < kMinGyroDps * kMinGyroDps ||
            gyro2 > kMaxGyroDps * kMaxGyroDps) {
        return;
    }
    if (!magnetic_samples_ || magnetic_samples_->generation != generation) {
        magnetic_samples_.reset(new (std::nothrow) MagneticCalibration());
        if (!magnetic_samples_) {
            return;
        }
        magnetic_samples_->started_us = now;
        magnetic_samples_->generation = generation;
    }
    auto &samples = *magnetic_samples_;
    if (samples.magnetic.add(raw[0], raw[1], raw[2])) {
        const auto index = samples.magnetic.lastStoredIndex();
        if (index < samples.magnetic.capacity) {
            samples.up[index] = up;
        }
        const float axes[] = {fabsf(up.x), fabsf(up.y), fabsf(up.z)};
        unsigned axis = 0;
        for (unsigned i = 1; i < 3; ++i) {
            if (axes[i] > axes[axis]) {
                axis = i;
            }
        }
        samples.observed_axes |= 1U << axis;
    }
    // No modal workflow or progress bar. Natural movement can supply the data.
    // Only try after observing all three dominant axes, never for an idle board.
    if (samples.observed_axes != 7 || samples.magnetic.size() < 320 ||
            now - samples.started_us < 20000000 || now < next_magnetic_fit_us_ || magnetic_fit_running_) {
        return;
    }
    next_magnetic_fit_us_ = now + 5000000;
    if (calibration_thread_.joinable()) {
        calibration_thread_.join();
    }
    try {
        auto *copy = new (std::nothrow) MagneticCalibration(samples);
        if (!copy) {
            return;
        }
        std::shared_ptr<MagneticCalibration> job(copy);
        const uint32_t job_generation = job->generation;
        boost::thread::attributes attributes;
        attributes.set_stack_size(8192);
        magnetic_fit_running_ = true;
        calibration_thread_ = boost::thread(attributes, [this, job, job_generation]() {
            // Fit below the sensor/UI priority so it cannot block gyro tracking.
            vTaskPrioritySet(nullptr, 1);
            compass_calibration::Result result;
            const auto status = compass_calibration::fit(job->magnetic, result);
            compass_axis_alignment::Result agreement;
            const bool valid = status == compass_calibration::Status::Ok &&
                               usableCalibrationModel(result) &&
                               compass_axis_alignment::check(job->magnetic, job->up, result, compass_board_frame::mag_map, agreement);
            bool published = false;
            if (valid) {
                std::lock_guard<std::mutex> guard(calibration_mutex_);
                if (bmm_running_ && calibration_generation_ == job_generation && !mag_cal_.calibrated) {
                    memcpy(mag_cal_.hard_iron, result.offset, sizeof(mag_cal_.hard_iron));
                    memcpy(mag_cal_.soft_iron, result.matrix, sizeof(mag_cal_.soft_iron));
                    mag_cal_.field_norm = result.field_norm;
                    mag_cal_.fit_error = result.fit_error;
                    mag_cal_.axes[0] = compass_board_frame::mag_map.x;
                    mag_cal_.axes[1] = compass_board_frame::mag_map.y;
                    mag_cal_.axes[2] = compass_board_frame::mag_map.z;
                    mag_cal_.axis_error = agreement.deviation;
                    mag_cal_.calibrated = true;
                    published = true;
                }
            }
            if (published) {
                saveCalibrationToNVS();
                ESP_LOGI(TAG, "Background magnetic calibration accepted: rms=%.4f agreement=%.4f",
                         result.fit_error, agreement.deviation);
            } else if (bmm_running_ && calibration_generation_ == job_generation) {
                ESP_LOGI(TAG, "Background magnetic calibration deferred: fit=%d; relative tracking continues",
                         static_cast<int>(status));
            }
            magnetic_fit_running_ = false;
        });
    } catch (const boost::thread_resource_error &error) {
        magnetic_fit_running_ = false;
        ESP_LOGW(TAG, "Background calibration task unavailable: %s", error.what());
    } catch (const std::bad_alloc &) {
        magnetic_fit_running_ = false;
        ESP_LOGW(TAG, "Background calibration allocation unavailable");
    }
}

void Compass::updateSensorData()
{
    // IMU propagation is independent of the slower 20 Hz magnetic data-ready.
    bmi2_sens_data imu = {};
    const int64_t now = esp_timer_get_time();
    if (bmi2_get_sensor_data(&imu, bmi2_dev_) != BMI2_OK ||
            !(imu.status & BMI2_DRDY_ACC_MASK) || !(imu.status & BMI2_DRDY_GYR_MASK)) {
        if (now - last_fresh_sample_us_ > 200000) {
            heading_is_north_ = false;
        }
        if (now - last_fresh_sample_us_ > SENSOR_STALE_TIMEOUT_US) {
            ui_state_ = UiState::SensorError;
        }
        return;
    }
    const float dt = last_valid_sample_us_ ? (now - last_valid_sample_us_) / 1000000.0f : .01f;
    last_valid_sample_us_ = now;
    last_fresh_sample_us_ = now;
    compass_imu_calibration::Model imu_model;
    mag_calibration_t magnetic_model;
    {
        std::lock_guard<std::mutex> guard(calibration_mutex_);
        imu_model = imu_cal_;
        magnetic_model = mag_cal_;
    }
    const compass_heading::Vec3 raw_acc{imu.acc.x / 8192.0f, imu.acc.y / 8192.0f, imu.acc.z / 8192.0f};
    const compass_heading::Vec3 raw_gyro{imu.gyr.x / 16.4f, imu.gyr.y / 16.4f, imu.gyr.z / 16.4f};
    if (!imu_model.gyro_valid) {
        gyro_stationary_.add(raw_acc, raw_gyro, dt);
        if (gyro_stationary_.ready(2)) {
            imu_model.gyro_bias = gyro_stationary_.gyroMean();
            imu_model.gyro_valid = true;
            if (compass_imu_calibration::valid(imu_model)) {
                {
                    std::lock_guard<std::mutex> guard(calibration_mutex_);
                    imu_cal_ = imu_model;
                }
                saveCalibrationToNVS();
                ESP_LOGI(TAG, "Gyro bias accepted: [%.4f, %.4f, %.4f] dps",
                         imu_model.gyro_bias.x, imu_model.gyro_bias.y, imu_model.gyro_bias.z);
            } else {
                imu_model.gyro_valid = false;
                gyro_stationary_.reset();
            }
        }
    }
    const auto acceleration = compass_board_frame::accel_map.apply(
                                  compass_imu_calibration::acceleration(imu_model, raw_acc));
    const auto gyro = compass_board_frame::accel_map.apply(
                          compass_imu_calibration::gyroscope(imu_model, raw_gyro));
    if (!fusion_.update(gyro, acceleration, dt)) {
        north_reference_.reset();
        heading_is_north_ = false;
        ui_state_ = UiState::Motion;
        return;
    }
    float raw[3] = {};
    if (readMagnetometer(raw)) {
        const float mag_dt = last_magnetic_sample_us_ ?
                             (now - last_magnetic_sample_us_) / 1000000.0f : .05f;
        last_magnetic_sample_us_ = now;
        if (magnetic_model.calibrated) {
            const bool had_magnetic_reference = fusion_.northReferenced();
            float heading_before = 0;
            const bool heading_before_valid = fusion_.heading(heading_before);
            const float centered[] = {raw[0] - magnetic_model.hard_iron[0], raw[1] - magnetic_model.hard_iron[1],
                                      raw[2] - magnetic_model.hard_iron[2]
                                     };
            float corrected[3] = {};
            for (int r = 0; r < 3; ++r) {
                for (int c = 0; c < 3; ++c) {
                    corrected[r] += magnetic_model.soft_iron[r][c] * centered[c];
                }
            }
            const compass_heading::Vec3 field{corrected[0], corrected[1], corrected[2]};
            const float norm = std::sqrt(compass_heading::dot(field, field));
            if (std::isfinite(norm) && std::fabs(norm - magnetic_model.field_norm) <
                    magnetic_model.field_norm * .25f + 3 &&
                    fusion_.correctMagnetic(compass_board_frame::mag_map.apply(field), mag_dt)) {
                if (!had_magnetic_reference && heading_before_valid) {
                    float heading_after;
                    if (fusion_.heading(heading_after)) {
                        north_reference_.preserveCorrection(heading_before, heading_after);
                    }
                }
                last_magnetic_accept_us_ = now;
            }
        } else if (imu_model.gyro_valid) {
            collectMagneticCalibration(raw, acceleration, gyro, now);
        }
    }
    float heading;
    if (!fusion_.heading(heading)) {
        heading_is_north_ = false;
        ui_state_ = UiState::UnobservablePose;
        return;
    }
    const bool magnetic_reliable = fusion_.northReferenced() && now - last_magnetic_accept_us_ < 300000;
    float displayed_heading;
    if (!north_reference_.update(heading, dt, magnetic_reliable, displayed_heading)) {
        return;
    }
    const bool aligned = magnetic_reliable && north_reference_.aligned();
    if (aligned && !heading_is_north_) {
        ESP_LOGI(TAG, "Magnetic north reference established");
    }
    heading_is_north_ = aligned;
    current_heading_.store(displayed_heading, std::memory_order_relaxed);
    ui_state_ = UiState::Ready;
}

void Compass::bmmDataThread()
{
    ESP_LOGI(TAG, "Compass fusion task started");
    bool need_gyro;
    {
        std::lock_guard<std::mutex> guard(calibration_mutex_);
        need_gyro = !imu_cal_.gyro_valid;
    }
    if (need_gyro) {
        calibrateGyroscope();
    }
    gyro_stationary_.reset();
    fusion_.reset();
    north_reference_.reset();
    heading_is_north_ = false;
    last_valid_sample_us_ = 0;
    last_magnetic_sample_us_ = 0;
    last_magnetic_accept_us_ = 0;
    while (bmm_running_) {
        updateSensorData();
        vTaskDelay(pdMS_TO_TICKS(SAMPLE_RATE_MS));
    }
    ESP_LOGI(TAG, "Compass fusion task stopped");
}

void Compass::updateCompassDisplay()
{
    const int16_t rotation = static_cast<int16_t>(
                                 compass_heading::pointer_angle_deg(current_heading_.load(std::memory_order_relaxed)) * 10.0f);
    lv_image_set_rotation(ui_Pointer, rotation);
    // Keep the original pointer fully visible; calibration quality remains internal.
    lv_obj_set_style_opa(ui_Pointer, LV_OPA_COVER, 0);
}

void Compass::updateUI()
{
    // Keep the original three screens, background, pointer and long-press ring.
    const UiState state = ui_state_.load();
    const bool tracking = state == UiState::Ready || state == UiState::UnobservablePose ||
                          state == UiState::Motion;
    lv_obj_t *screen = tracking ? ui_CompassScreen : ui_CompassCorrectScreen;
    const char *text = "Keep the board still\nfor two seconds";
    if (state == UiState::SensorError) {
        screen = ui_CompassTipScreen;
        if (ui_CompassTipText) {
            lv_label_set_text(ui_CompassTipText, "Compass sensor unavailable.");
        }
    } else if (!tracking && ui_CorrectTips) {
        lv_label_set_text(ui_CorrectTips, text);
    }
    if (!tracking && ui_Pointer) {
        lv_obj_add_flag(ui_Pointer, LV_OBJ_FLAG_HIDDEN);
    }
    if (screen && lv_screen_active() != screen) {
        if (tracking) {
            if (ui_ProgressBar) {
                lv_arc_set_value(ui_ProgressBar, 0);
            }
            if (ui_CalibrationTip) {
                lv_obj_add_flag(ui_CalibrationTip, LV_OBJ_FLAG_HIDDEN);
            }
            if (ui_Pointer) {
                lv_obj_remove_flag(ui_Pointer, LV_OBJ_FLAG_HIDDEN);
            }
        }
        lv_screen_load(screen);
    }
    if (tracking && ui_Pointer) {
        updateCompassDisplay();
    }
}

// Versioned, single-blob calibration storage. All keys belong to v5 only.
bool Compass::saveCalibrationToNVS()
{
    // Serialize snapshot + write so a late background save cannot undo reset.
    std::lock_guard<std::mutex> storage_guard(nvs_mutex_);
    mag_calibration_t magnetic;
    compass_imu_calibration::Model imu;
    {
        std::lock_guard<std::mutex> guard(calibration_mutex_);
        magnetic = mag_cal_;
        imu = imu_cal_;
    }
    CalibrationRecord record;
    record.flags = (imu.gyro_valid ? 1U : 0U) | (imu.accel_valid ? 2U : 0U) |
                   (magnetic.calibrated ? 4U : 0U);
    record.gyro_bias[0] = imu.gyro_bias.x; record.gyro_bias[1] = imu.gyro_bias.y; record.gyro_bias[2] = imu.gyro_bias.z;
    record.accel_bias[0] = imu.accel_bias.x; record.accel_bias[1] = imu.accel_bias.y; record.accel_bias[2] = imu.accel_bias.z;
    record.accel_scale[0] = imu.accel_scale.x; record.accel_scale[1] = imu.accel_scale.y; record.accel_scale[2] = imu.accel_scale.z;
    if (magnetic.calibrated) {
        memcpy(record.offset, magnetic.hard_iron, sizeof(record.offset));
        memcpy(record.matrix, magnetic.soft_iron, sizeof(record.matrix));
        memcpy(record.axes, magnetic.axes, sizeof(record.axes));
        record.field_norm = magnetic.field_norm;
        record.fit_error = magnetic.fit_error;
        record.axis_error = magnetic.axis_error;
    }
    if (!compass_nine_axis_storage::valid(record)) {
        ESP_LOGW(TAG, "Refusing invalid nine-axis calibration record");
        return false;
    }
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        return false;
    }
    err = nvs_set_blob(handle, NVS_KEY_MODEL, &record, sizeof(record));
    if (err == ESP_OK) {
        err = nvs_commit(handle);
    }
    nvs_close(handle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Nine-axis calibration save failed: %s", esp_err_to_name(err));
    }
    return err == ESP_OK;
}

bool Compass::loadCalibrationFromNVS()
{
    std::lock_guard<std::mutex> storage_guard(nvs_mutex_);
    nvs_handle_t handle;
    if (nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle) != ESP_OK) {
        return false;
    }
    CalibrationRecord record;
    size_t size = sizeof(record);
    const esp_err_t err = nvs_get_blob(handle, NVS_KEY_MODEL, &record, &size);
    nvs_close(handle);
    if (err != ESP_OK || size != sizeof(record) || !compass_nine_axis_storage::valid(record)) {
        ESP_LOGW(TAG, "No valid v5 nine-axis calibration");
        return false;
    }
    mag_calibration_t magnetic;
    if (record.flags & 4) {
        memcpy(magnetic.hard_iron, record.offset, sizeof(magnetic.hard_iron));
        memcpy(magnetic.soft_iron, record.matrix, sizeof(magnetic.soft_iron));
        memcpy(magnetic.axes, record.axes, sizeof(magnetic.axes));
        magnetic.field_norm = record.field_norm;
        magnetic.fit_error = record.fit_error;
        magnetic.axis_error = record.axis_error;
        magnetic.calibrated = true;
    }
    {
        std::lock_guard<std::mutex> guard(calibration_mutex_);
        imu_cal_ = compass_nine_axis_storage::imuModel(record);
        mag_cal_ = magnetic;
        ++calibration_generation_;
    }
    ESP_LOGI(TAG, "Loaded v5 calibration: gyro=%d accel=%d mag+axes=%d",
             bool(record.flags & 1), bool(record.flags & 2), bool(record.flags & 4));
    return true;
}

// Async NVS operations
void Compass::saveCalibrationToNVSAsync()
{
    ESP_LOGI(TAG, "Starting async NVS save operation...");
    if (nvs_thread_.joinable()) {
        nvs_thread_.join();
    }
    try {
        nvs_thread_ = boost::thread([this]() {
            bool result = saveCalibrationToNVS();
            ESP_LOGI(TAG, "Async NVS save completed with result: %s", result ? "success" : "failed");
        });
    } catch (const boost::thread_resource_error &error) {
        ESP_LOGE(TAG, "Failed to start NVS save task: %s", error.what());
        saveCalibrationToNVS();
    }
}

ESP_UTILS_REGISTER_PLUGIN_WITH_CONSTRUCTOR(systems::base::App, Compass, "Compass", []()
{
    return std::shared_ptr<Compass>(Compass::requestInstance(), [](Compass * p) {});
})

} // namespace esp_brookesia::apps
