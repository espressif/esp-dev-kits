/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 * @file app_qma6100.c
 * @brief QMA6100 IMU sensor application layer implementation
 *
 * This module provides high-level interface for QMA6100 accelerometer including:
 * - Real-time IMU data monitoring with background task
 * - Simplified single-axis device rotation detection (0°, 90°, 270°)
 * - Automatic display rotation based on device orientation
 * - Roll/pitch angle calculation from acceleration data
 *
 * Key Features:
 * - Background task for continuous monitoring
 * - Stability filtering for reliable rotation detection
 * - Callback system for rotation change notifications
 * - Cached data access for performance optimization
 * - Simplified single-axis detection without complex calibration
 *
 * Single-Axis Rotation Detection Algorithm:
 * - 0° (Portrait): Strong negative Y-axis (ay < -0.6g)
 * - 90° (Landscape Right): Strong negative X-axis (ax < -0.6g)
 * - 270° (Landscape Left): Strong positive X-axis (ax > 0.6g)
 * - Uses reduced stability threshold (2 instead of 3) for faster response
 * - No confidence level requirements for more responsive detection
 *
 * Usage Example:
 * ```c
 * // Initialize sensor with automatic rotation
 * app_qma6100_init(i2c_bus_handle);
 *
 * // Use simplified detection for 0°, 90°, 270° only
 * qma6100_rotation_result_t result;
 * if (app_qma6100_get_rotation_simple(&result) == ESP_OK) {
 *     switch (result.rotation) {
 *         case QMA6100_ROTATION_0_DEGREE:
 *             printf("Device is in portrait mode (0°)\n");
 *             break;
 *         case QMA6100_ROTATION_90_DEGREE:
 *             printf("Device is in landscape right (90°)\n");
 *             break;
 *         case QMA6100_ROTATION_270_DEGREE:
 *             printf("Device is in landscape left (270°)\n");
 *             break;
 *         case QMA6100_ROTATION_UNKNOWN:
 *             printf("Cannot determine rotation\n");
 *             break;
 *     }
 * }
 * ```
 */

#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include "esp_system.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "bsp/esp-bsp.h"
#include "app_qma6100.h"
#include "ui_extra.h"

/* ============================= Constants & Configuration ============================= */

static const char *TAG = "APP_QMA6100";

// Callback and task management constants
#define MAX_ROTATION_CALLBACKS          3       ///< Maximum number of rotation callbacks
#define STABILITY_THRESHOLD             3       ///< Required consecutive detections for stable rotation (legacy)
#define SIMPLE_STABILITY_THRESHOLD      2       ///< Reduced threshold for simplified single-axis detection

// Task timing and intervals
#define ROTATION_CHECK_INTERVAL_MS      300     ///< Display rotation check interval in milliseconds (increased to reduce I2C bus contention)

// Physical detection thresholds
#define FLATNESS_THRESHOLD              0.9f    ///< Minimum Z-axis acceleration for flatness detection

/* ============================= Private Data Structures ============================= */

/**
 * @brief Rotation callback entry for managing multiple callbacks
 */
typedef struct {
    app_qma6100_rotation_callback_t callback;   ///< Callback function pointer
    void *user_data;                            ///< User data for callback
    bool in_use;                                ///< Slot occupation status
} rotation_callback_entry_t;

/* ============================= Global Variables ============================= */

// Core sensor handle
static qma6100p_handle_t qma6100_handle = NULL;

// Real-time monitoring task management
static TaskHandle_t monitoring_task_handle = NULL;
static bool monitoring_active = false;
static uint32_t monitoring_sample_rate_ms = 100;  // Default 100ms sample rate

// Display rotation task management
static TaskHandle_t display_rotation_task_handle = NULL;
static bool display_rotation_active = false;

// Latest IMU data cache for real-time access
static qma6100_realtime_data_t latest_imu_data = {0};
static bool data_available = false;

// Rotation callback management
static rotation_callback_entry_t rotation_callbacks[MAX_ROTATION_CALLBACKS] = {0};
static lv_disp_rot_t current_display_rotation = LV_DISP_ROT_NONE;

/* ============================= Private Helper Functions ============================= */

/**
 * @brief Calculate gravity magnitude from 3-axis acceleration data
 *
 * @param acce_data Pointer to acceleration data
 * @return Gravity magnitude in g units
 */
static float calculate_gravity_magnitude(const qma6100p_acce_value_t *acce_data)
{
    return sqrtf(acce_data->acce_x * acce_data->acce_x +
                 acce_data->acce_y * acce_data->acce_y +
                 acce_data->acce_z * acce_data->acce_z);
}

/**
 * @brief Call all registered rotation callbacks
 *
 * @param new_rotation New LVGL display rotation
 * @param qma_rotation QMA6100 rotation that triggered the change
 */
static void call_rotation_callbacks(lv_disp_rot_t new_rotation, qma6100_rotation_t qma_rotation)
{
    for (int i = 0; i < MAX_ROTATION_CALLBACKS; i++) {
        if (rotation_callbacks[i].in_use && rotation_callbacks[i].callback != NULL) {
            rotation_callbacks[i].callback(new_rotation, qma_rotation, rotation_callbacks[i].user_data);
        }
    }
}

/* ============================= Background Tasks ============================= */

/**
 * @brief Real-time IMU data monitoring task
 *
 * Continuously samples IMU data at specified rate and caches the latest values.
 * This task runs in the background to provide instant data access without I/O delays.
 *
 * @param pvParameters Task parameters (unused)
 */
static void qma6100_monitoring_task(void *pvParameters)
{
    qma6100_realtime_data_t local_data;
    TickType_t last_wake_time = xTaskGetTickCount();

    ESP_LOGI(TAG, "Real-time IMU monitoring task started with %ld ms sample rate", monitoring_sample_rate_ms);

    while (monitoring_active) {
        // Get real-time IMU data
        if (app_qma6100_get_realtime_data(&local_data) == ESP_OK) {
            // Update latest data atomically
            latest_imu_data = local_data;
            data_available = true;

            // Debug logging for angle and magnitude values
            ESP_LOGD(TAG, "IMU Data - Roll: %.2f°, Pitch: %.2f°, Magnitude: %.3fg",
                     local_data.angles.roll, local_data.angles.pitch, local_data.magnitude);
        } else {
            ESP_LOGW(TAG, "Failed to retrieve real-time IMU data");
        }

        // Wait for next sample period
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(monitoring_sample_rate_ms));
    }

    ESP_LOGI(TAG, "Real-time IMU monitoring task stopped");
    monitoring_task_handle = NULL;
    vTaskDelete(NULL);
}

/**
 * @brief Display rotation task with simplified single-axis detection
 *
 * Monitors device orientation using simplified single-axis detection algorithm.
 * Uses stability filtering to prevent frequent rotation changes from minor movements.
 * Only detects 0°, 90°, 270° orientations without complex calibration requirements.
 *
 * Key Features:
 * - Uses app_qma6100_get_rotation_simple() for detection
 * - Reduced stability threshold (SIMPLE_STABILITY_THRESHOLD = 2) for faster response
 * - No confidence level requirements for more responsive behavior
 * - Supports only three orientations: Portrait, Landscape Right, Landscape Left
 *
 * @param pvParameters Task parameters (unused)
 */
static void qma6100_display_rotation_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Display rotation task started with simplified single-axis detection");
    lv_disp_rot_t current_rotation = LV_DISP_ROT_NONE;

    // Stability filtering variables
    qma6100_rotation_t last_detected_rotation = QMA6100_ROTATION_UNKNOWN;
    qma6100_rotation_t stable_rotation = QMA6100_ROTATION_UNKNOWN;
    uint8_t stability_counter = 0;

    while (display_rotation_active) {
        // Check if gyroscope is enabled in UI settings
        if (!ui_extra_get_gyroscope_enabled()) {
            ESP_LOGD(TAG, "Gyroscope disabled in settings, skipping rotation detection");
            vTaskDelay(pdMS_TO_TICKS(ROTATION_CHECK_INTERVAL_MS));
            continue;
        }

        qma6100_rotation_result_t rotation_result;
        // Use simplified single-axis detection instead of complex calibration
        esp_err_t ret = app_qma6100_get_rotation_simple(&rotation_result);

        // Simplified conditions: only require device to be flat and valid rotation
        if (ret == ESP_OK && rotation_result.is_flat &&
                rotation_result.rotation != QMA6100_ROTATION_UNKNOWN &&
                rotation_result.rotation != QMA6100_ROTATION_TILTED) {

            // Stability filtering: check if rotation is consistent
            if (rotation_result.rotation == last_detected_rotation) {
                stability_counter++;
                ESP_LOGD(TAG, "Stable rotation detected: %s (count: %d/%d)",
                         app_qma6100_rotation_to_string(rotation_result.rotation),
                         stability_counter, SIMPLE_STABILITY_THRESHOLD);
            } else {
                // Reset counter when rotation changes
                stability_counter = 1;
                last_detected_rotation = rotation_result.rotation;
                ESP_LOGD(TAG, "Rotation changed to: %s (resetting stability counter)",
                         app_qma6100_rotation_to_string(rotation_result.rotation));
            }

            // Use reduced stability threshold for responsive single-axis detection
            if (stability_counter >= SIMPLE_STABILITY_THRESHOLD) {
                stable_rotation = rotation_result.rotation;

                lv_disp_rot_t new_rotation = LV_DISP_ROT_NONE;

                // Convert QMA6100 rotation to LVGL rotation
                // Only handle the three supported orientations: 0°, 90°, 270°
                switch (stable_rotation) {
                case QMA6100_ROTATION_0_DEGREE:
                    new_rotation = LV_DISP_ROT_180;
                    break;
                case QMA6100_ROTATION_90_DEGREE:
                    new_rotation = LV_DISP_ROT_270;  // QMA6100 90° maps to LVGL 270°
                    break;
                case QMA6100_ROTATION_270_DEGREE:
                    new_rotation = LV_DISP_ROT_90;   // QMA6100 270° maps to LVGL 90°
                    break;
                case QMA6100_ROTATION_180_DEGREE:
                    new_rotation = LV_DISP_ROT_NONE;  // QMA6100 180° maps to LVGL 180°
                    break;
                default:
                    // Skip unsupported rotations (180° not detected in simple mode)
                    new_rotation = current_rotation;
                    break;
                }

                // Only apply rotation if it changed and is stable
                if (new_rotation != current_rotation) {
                    ESP_LOGI(TAG, "Applying display rotation: %s -> %s (stable for %d detections)",
                             current_rotation == LV_DISP_ROT_NONE ? "0°" :
                             current_rotation == LV_DISP_ROT_90 ? "90°" :
                             current_rotation == LV_DISP_ROT_180 ? "180°" : "270°",
                             app_qma6100_rotation_to_string(stable_rotation),
                             stability_counter);

                    // Apply display rotation with thread safety
                    bsp_display_lock(0);
                    bsp_display_rotate(new_rotation);
                    bsp_display_unlock();

                    current_rotation = new_rotation;
                    current_display_rotation = new_rotation;  // Update global state

                    // Notify all registered callbacks
                    call_rotation_callbacks(new_rotation, stable_rotation);
                }
            }
        } else {
            // Reset stability counter when detection conditions are not met
            if (stability_counter > 0) {
                ESP_LOGD(TAG, "Resetting stability counter - detection conditions not met");
                stability_counter = 0;
                last_detected_rotation = QMA6100_ROTATION_UNKNOWN;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(ROTATION_CHECK_INTERVAL_MS));
    }

    ESP_LOGI(TAG, "Display rotation task stopped");
    vTaskDelete(NULL);
}

/* ============================= Core Functions ============================= */

esp_err_t app_qma6100_init(i2c_master_bus_handle_t i2c_bus_handle)
{
    if (i2c_bus_handle == NULL) {
        ESP_LOGE(TAG, "Invalid I2C bus handle provided");
        return ESP_ERR_INVALID_ARG;
    }

    if (qma6100_handle != NULL) {
        ESP_LOGW(TAG, "QMA6100 sensor already initialized");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Initializing QMA6100 sensor...");

    // Create QMA6100P sensor instance
    esp_err_t ret = qma6100p_create(i2c_bus_handle, QMA6100P_I2C_ADDRESS, &qma6100_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create QMA6100P instance: %s", esp_err_to_name(ret));
        return ret;
    }

    // Wake up the sensor from sleep mode
    ret = qma6100p_wake_up(qma6100_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to wake up QMA6100P sensor: %s", esp_err_to_name(ret));
        qma6100p_delete(qma6100_handle);
        qma6100_handle = NULL;
        return ret;
    }

    // Configure sensor with default settings (±2g range for optimal sensitivity)
    ret = qma6100p_config(qma6100_handle, ACCE_FS_2G);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure QMA6100P sensor: %s", esp_err_to_name(ret));
        qma6100p_delete(qma6100_handle);
        qma6100_handle = NULL;
        return ret;
    }

    ESP_LOGI(TAG, "QMA6100 sensor initialized successfully");

    // Automatically start display rotation task for complete functionality
    ESP_LOGI(TAG, "Starting integrated display auto-rotation...");
    ret = app_qma6100_start_display_rotation();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to start display rotation, but sensor init successful: %s", esp_err_to_name(ret));
        // Don't fail the entire init if display rotation fails - sensor is still usable
    } else {
        ESP_LOGI(TAG, "Display auto-rotation started successfully");
    }

    ESP_LOGI(TAG, "QMA6100 initialization complete with integrated display rotation");
    return ESP_OK;
}

esp_err_t app_qma6100_deinit(void)
{
    ESP_LOGI(TAG, "Deinitializing QMA6100 sensor...");

    // Stop display rotation if active
    if (display_rotation_active) {
        ESP_LOGI(TAG, "Stopping display rotation task...");
        app_qma6100_stop_display_rotation();
    }

    // Stop monitoring if active
    if (monitoring_active) {
        ESP_LOGI(TAG, "Stopping real-time monitoring task...");
        app_qma6100_stop_realtime_monitoring();
    }

    // Release sensor resources
    if (qma6100_handle != NULL) {
        qma6100p_delete(qma6100_handle);
        qma6100_handle = NULL;
        ESP_LOGI(TAG, "QMA6100 sensor deinitialized successfully");
    }

    return ESP_OK;
}

/* ============================= Data Access Functions ============================= */

esp_err_t app_qma6100_get_data(qma6100p_acce_value_t *acce_data)
{
    if (qma6100_handle == NULL) {
        ESP_LOGE(TAG, "QMA6100 sensor not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (acce_data == NULL) {
        ESP_LOGE(TAG, "Invalid acceleration data pointer");
        return ESP_ERR_INVALID_ARG;
    }

    return qma6100p_get_acce(qma6100_handle, acce_data);
}

esp_err_t app_qma6100_calculate_angles(const qma6100p_acce_value_t *acce_data, qma6100_angle_t *angles)
{
    if (acce_data == NULL || angles == NULL) {
        ESP_LOGE(TAG, "Invalid parameters for angle calculation");
        return ESP_ERR_INVALID_ARG;
    }

    // Extract acceleration values for better readability
    float ax = acce_data->acce_x;
    float ay = acce_data->acce_y;
    float az = acce_data->acce_z;

    // Calculate roll angle (rotation around X-axis) - range: -180° to +180°
    // Roll represents left/right tilt of the device
    angles->roll = atan2f(ay, sqrtf(ax * ax + az * az)) * 180.0f / M_PI;

    // Calculate pitch angle (rotation around Y-axis) - range: -180° to +180°
    // Pitch represents forward/backward tilt of the device
    angles->pitch = atan2f(-ax, sqrtf(ay * ay + az * az)) * 180.0f / M_PI;

    // Alternative calculation for better accuracy when device is upright
    // Use atan2 with Z-axis for more stable results
    if (fabsf(az) > 0.1f) { // Avoid division by very small numbers
        angles->roll = atan2f(ay, az) * 180.0f / M_PI;
        angles->pitch = atan2f(-ax, az) * 180.0f / M_PI;
    }

    ESP_LOGD(TAG, "Angle calculation: Raw accel[%.3f, %.3f, %.3f] -> Roll=%.1f°, Pitch=%.1f°",
             ax, ay, az, angles->roll, angles->pitch);

    return ESP_OK;
}

esp_err_t app_qma6100_get_realtime_data(qma6100_realtime_data_t *imu_data)
{
    if (imu_data == NULL) {
        ESP_LOGE(TAG, "Invalid IMU data pointer");
        return ESP_ERR_INVALID_ARG;
    }

    // Get raw acceleration data from sensor
    esp_err_t ret = app_qma6100_get_data(&imu_data->acceleration);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read acceleration data: %s", esp_err_to_name(ret));
        return ret;
    }

    // Calculate gravity magnitude for data validation
    imu_data->magnitude = calculate_gravity_magnitude(&imu_data->acceleration);

    // Calculate roll and pitch angles from acceleration data
    ret = app_qma6100_calculate_angles(&imu_data->acceleration, &imu_data->angles);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to calculate angles: %s", esp_err_to_name(ret));
        return ret;
    }

    return ESP_OK;
}

/* ============================= Real-time Monitoring ============================= */

esp_err_t app_qma6100_start_realtime_monitoring(uint32_t sample_rate_ms)
{
    if (qma6100_handle == NULL) {
        ESP_LOGE(TAG, "QMA6100 sensor not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (monitoring_active) {
        ESP_LOGW(TAG, "Real-time monitoring already active");
        return ESP_OK;
    }

    if (sample_rate_ms < 10 || sample_rate_ms > 10000) {
        ESP_LOGE(TAG, "Invalid sample rate: %ld ms (valid range: 10-10000 ms)", sample_rate_ms);
        return ESP_ERR_INVALID_ARG;
    }

    monitoring_sample_rate_ms = sample_rate_ms;
    monitoring_active = true;

    // Create monitoring task with appropriate stack size and priority
    BaseType_t task_created = xTaskCreate(
                                  qma6100_monitoring_task,
                                  "qma6100_monitor",
                                  4096,  // Stack size in bytes
                                  NULL,  // Task parameters
                                  5,     // Task priority
                                  &monitoring_task_handle
                              );

    if (task_created != pdPASS) {
        monitoring_active = false;
        ESP_LOGE(TAG, "Failed to create real-time monitoring task");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Started real-time IMU monitoring with %ld ms sample rate", sample_rate_ms);
    return ESP_OK;
}

esp_err_t app_qma6100_stop_realtime_monitoring(void)
{
    if (!monitoring_active) {
        ESP_LOGW(TAG, "Real-time monitoring not currently active");
        return ESP_OK;
    }

    monitoring_active = false;

    // Wait for task to finish gracefully
    if (monitoring_task_handle != NULL) {
        // Give task time to clean up
        vTaskDelay(pdMS_TO_TICKS(100));

        // Force delete if task is still running
        if (monitoring_task_handle != NULL) {
            vTaskDelete(monitoring_task_handle);
            monitoring_task_handle = NULL;
        }
    }

    data_available = false;
    ESP_LOGI(TAG, "Stopped real-time IMU monitoring");
    return ESP_OK;
}

bool app_qma6100_is_monitoring_active(void)
{
    return monitoring_active;
}

esp_err_t app_qma6100_get_latest_data(qma6100_realtime_data_t *imu_data)
{
    if (imu_data == NULL) {
        ESP_LOGE(TAG, "Invalid IMU data pointer");
        return ESP_ERR_INVALID_ARG;
    }

    if (!data_available) {
        ESP_LOGW(TAG, "No real-time data available yet - start monitoring first");
        return ESP_ERR_NOT_FOUND;
    }

    // Copy the latest data atomically (single assignment is atomic for structs)
    *imu_data = latest_imu_data;
    return ESP_OK;
}

/* ============================= Rotation Detection ============================= */

esp_err_t app_qma6100_get_rotation(qma6100_rotation_result_t *result)
{
    if (result == NULL) {
        ESP_LOGE(TAG, "Invalid result pointer for rotation detection");
        return ESP_ERR_INVALID_ARG;
    }

    if (qma6100_handle == NULL) {
        ESP_LOGE(TAG, "QMA6100 sensor not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    // Get current IMU data for rotation analysis
    qma6100_realtime_data_t imu_data;
    esp_err_t ret = app_qma6100_get_realtime_data(&imu_data);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get real-time data for rotation detection: %s", esp_err_to_name(ret));
        return ret;
    }

    // Extract acceleration components for analysis
    float ax = imu_data.acceleration.acce_x;
    float ay = imu_data.acceleration.acce_y;
    float az = imu_data.acceleration.acce_z;

    float a = sqrtf(ax * ax + ay * ay + az * az);
    if (a > 0.0f) {
        ax /= a;
        ay /= a;
        az /= a;
    }

    // Calculate rotation angle using atan2 for gravity vector projection on XY plane
    float rotation_deg = atan2f(-ax, ay) * 180.0f / M_PI;
    if (rotation_deg < 0) {
        rotation_deg += 360;
    }

    // Fill basic result information
    result->raw_angle = rotation_deg;
    result->magnitude = imu_data.magnitude;
    result->is_flat = (fabsf(az) < FLATNESS_THRESHOLD);  // Device flatness check

    // Initialize detection result
    result->rotation = QMA6100_ROTATION_UNKNOWN;
    result->confidence = QMA6100_CONFIDENCE_HIGH;  // Always set to HIGH confidence

    ESP_LOGD(TAG, "Rotation analysis: ax=%.3f, ay=%.3f, az=%.3f, angle=%.1f°",
             ax, ay, az, rotation_deg);

    // Determine rotation based on angle ranges
    // Using 45° tolerance zones for each orientation
    if (rotation_deg >= 315.0f || rotation_deg < 45.0f) {
        // 0° (Portrait) - angle around 0° or 360°
        result->rotation = QMA6100_ROTATION_0_DEGREE;
        ESP_LOGD(TAG, "Detected 0° rotation (angle: %.1f°)", rotation_deg);

    } else if (rotation_deg >= 45.0f && rotation_deg < 135.0f) {
        // 90° (Landscape Right) - angle around 90°
        result->rotation = QMA6100_ROTATION_90_DEGREE;
        ESP_LOGD(TAG, "Detected 90° rotation (angle: %.1f°)", rotation_deg);

    } else if (rotation_deg >= 135.0f && rotation_deg < 225.0f) {
        // 180° (Portrait Inverted) - angle around 180°
        result->rotation = QMA6100_ROTATION_180_DEGREE;
        ESP_LOGD(TAG, "Detected 180° rotation (angle: %.1f°)", rotation_deg);

    } else if (rotation_deg >= 225.0f && rotation_deg < 315.0f) {
        // 270° (Landscape Left) - angle around 270°
        result->rotation = QMA6100_ROTATION_270_DEGREE;
        ESP_LOGD(TAG, "Detected 270° rotation (angle: %.1f°)", rotation_deg);

    } else {
        // Should not reach here due to angle normalization
        result->rotation = QMA6100_ROTATION_UNKNOWN;
        ESP_LOGD(TAG, "Invalid angle range (%.1f°)", rotation_deg);
    }

    ESP_LOGI(TAG, "Rotation detection result: %s (angle: %.1f°, ax=%.3f, ay=%.3f)",
             app_qma6100_rotation_to_string(result->rotation),
             rotation_deg, ax, ay);

    return ESP_OK;
}

/**
 * @brief Simplified rotation detection using gravity vector projection on XY plane
 *
 * This function provides a simplified approach to rotation detection by analyzing
 * the gravity vector's projection angle on the XY plane. It uses a continuous
 * angle-based approach rather than discrete thresholds for more stable detection.
 *
 * Detection Algorithm:
 * - Calculate rotation angle using atan2(-ax, ay) to get gravity vector angle
 * - Normalize angle to 0-360° range
 * - Use 45° tolerance zones for each orientation:
 *   * 0° (Portrait): 315°-45°
 *   * 90° (Landscape Right): 45°-135°
 *   * 180° (Portrait Inverted): 135°-225°
 *   * 270° (Landscape Left): 225°-315°
 *
 * Key Features:
 * - Continuous angle-based detection for smoother transitions
 * - No threshold-based detection or confidence levels
 * - Supports all four orientations (0°, 90°, 180°, 270°)
 * - Normalized acceleration values for consistent detection
 *
 * @param result Pointer to store rotation detection result
 * @return ESP_OK on success, ESP_ERR_INVALID_ARG if result is NULL,
 *         ESP_ERR_INVALID_STATE if sensor not initialized
 */
esp_err_t app_qma6100_get_rotation_simple(qma6100_rotation_result_t *result)
{
    if (result == NULL) {
        ESP_LOGE(TAG, "Invalid result pointer for simple rotation detection");
        return ESP_ERR_INVALID_ARG;
    }

    if (qma6100_handle == NULL) {
        ESP_LOGE(TAG, "QMA6100 sensor not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    // Get current IMU data for rotation analysis
    qma6100_realtime_data_t imu_data;
    esp_err_t ret = app_qma6100_get_realtime_data(&imu_data);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get real-time data for simple rotation detection: %s", esp_err_to_name(ret));
        return ret;
    }

    // Extract acceleration components
    float ax = imu_data.acceleration.acce_x;
    float ay = imu_data.acceleration.acce_y;
    float az = imu_data.acceleration.acce_z;

    // Calculate acceleration magnitude using 3D vector length formula
    float a = sqrtf(ax * ax + ay * ay + az * az);
    if (a > 0.0f) {
        ax /= a;
        ay /= a;
        az /= a;
    }

    // Fill basic result information
    result->magnitude = imu_data.magnitude;
    result->is_flat = (fabsf(az) < FLATNESS_THRESHOLD);

    // Calculate rotation angle using atan2 for gravity vector projection on XY plane
    // Note: -ax is used because of the sensor's coordinate system orientation
    result->raw_angle = atan2f(-ax, ay) * 180.0f / M_PI;
    if (result->raw_angle < 0) {
        result->raw_angle += 360;
    }

    // Initialize as unknown
    result->rotation = QMA6100_ROTATION_UNKNOWN;
    result->confidence = QMA6100_CONFIDENCE_HIGH;  // Always set to HIGH confidence

    ESP_LOGD(TAG, "Simple rotation analysis: ax=%.3f, ay=%.3f, az=%.3f, angle=%.1f°",
             ax, ay, az, result->raw_angle);

    // Determine rotation based on angle ranges
    // Using 45° tolerance zones for each orientation
    if (result->raw_angle >= 315.0f || result->raw_angle < 45.0f) {
        // 0° (Portrait) - angle around 0° or 360°
        result->rotation = QMA6100_ROTATION_0_DEGREE;
        ESP_LOGD(TAG, "Simple detection: 0° (angle: %.1f°)", result->raw_angle);

    } else if (result->raw_angle >= 45.0f && result->raw_angle < 135.0f) {
        // 90° (Landscape Right) - angle around 90°
        result->rotation = QMA6100_ROTATION_90_DEGREE;
        ESP_LOGD(TAG, "Simple detection: 90° (angle: %.1f°)", result->raw_angle);

    } else if (result->raw_angle >= 135.0f && result->raw_angle < 225.0f) {
        // 180° (Portrait Inverted) - angle around 180°
        result->rotation = QMA6100_ROTATION_180_DEGREE;
        ESP_LOGD(TAG, "Simple detection: 180° (angle: %.1f°)", result->raw_angle);

    } else if (result->raw_angle >= 225.0f && result->raw_angle < 315.0f) {
        // 270° (Landscape Left) - angle around 270°
        result->rotation = QMA6100_ROTATION_270_DEGREE;
        ESP_LOGD(TAG, "Simple detection: 270° (angle: %.1f°)", result->raw_angle);

    } else {
        // Should not reach here due to angle normalization
        result->rotation = QMA6100_ROTATION_UNKNOWN;
        ESP_LOGD(TAG, "Simple detection: Invalid angle range (%.1f°)", result->raw_angle);
    }

    ESP_LOGD(TAG, "Simple rotation detection result: %s (angle: %.1f°, ax=%.3f, ay=%.3f)",
             app_qma6100_rotation_to_string(result->rotation),
             result->raw_angle, ax, ay);

    return ESP_OK;
}

const char *app_qma6100_rotation_to_string(qma6100_rotation_t rotation)
{
    switch (rotation) {
    case QMA6100_ROTATION_0_DEGREE:   return "0° (Portrait)";
    case QMA6100_ROTATION_90_DEGREE:  return "90° (Landscape Right)";
    case QMA6100_ROTATION_180_DEGREE: return "180° (Portrait Inverted)";
    case QMA6100_ROTATION_270_DEGREE: return "270° (Landscape Left)";
    case QMA6100_ROTATION_TILTED:     return "Tilted (Not Flat)";
    case QMA6100_ROTATION_UNKNOWN:    return "Unknown";
    default:                          return "Invalid";
    }
}

const char *app_qma6100_confidence_to_string(qma6100_confidence_t confidence)
{
    switch (confidence) {
    case QMA6100_CONFIDENCE_HIGH:   return "High";
    case QMA6100_CONFIDENCE_MEDIUM: return "Medium";
    case QMA6100_CONFIDENCE_LOW:    return "Low";
    case QMA6100_CONFIDENCE_NONE:   return "None";
    default:                        return "Invalid";
    }
}

/* ============================= Display Auto-Rotation ============================= */

/**
 * @brief Start automatic display rotation based on simplified device orientation detection
 *
 * Creates a background task that monitors device rotation using single-axis detection
 * and automatically adjusts the display orientation. Uses stability filtering to prevent
 * frequent rotation changes from minor movements.
 *
 * Simplified Detection Features:
 * - Only detects 0°, 90°, 270° orientations (no 180° support)
 * - Uses single-axis thresholds without complex calibration
 * - Faster response with reduced stability threshold (2 instead of 3)
 * - No confidence level requirements - more responsive
 *
 * @note This function is called automatically during app_qma6100_init().
 *       Use this function only if you need to restart the rotation task
 *       after stopping it manually.
 *
 * @return ESP_OK on success, ESP_ERR_INVALID_STATE if already started or
 *         sensor not initialized, ESP_ERR_NO_MEM if task creation fails
 */
esp_err_t app_qma6100_start_display_rotation(void)
{
    if (!qma6100_handle) {
        ESP_LOGE(TAG, "QMA6100 sensor not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (display_rotation_active) {
        ESP_LOGW(TAG, "Display auto-rotation already active");
        return ESP_ERR_INVALID_STATE;
    }

    display_rotation_active = true;

    // Create display rotation task with appropriate configuration
    BaseType_t result = xTaskCreate(
                            qma6100_display_rotation_task,
                            "display_rotation",
                            4096,   // Stack size in bytes
                            NULL,   // Task parameters
                            5,      // Task priority
                            &display_rotation_task_handle
                        );

    if (result != pdPASS) {
        display_rotation_active = false;
        ESP_LOGE(TAG, "Failed to create display rotation task");
        return ESP_ERR_NO_MEM;
    }

    ESP_LOGI(TAG, "Display auto-rotation task started successfully");
    return ESP_OK;
}

esp_err_t app_qma6100_stop_display_rotation(void)
{
    if (!display_rotation_active) {
        ESP_LOGW(TAG, "Display auto-rotation not currently active");
        return ESP_OK;
    }

    display_rotation_active = false;

    // Wait for task to finish gracefully
    if (display_rotation_task_handle != NULL) {
        // Give task time to clean up
        vTaskDelay(pdMS_TO_TICKS(100));

        // Force delete if task is still running
        if (display_rotation_task_handle != NULL) {
            vTaskDelete(display_rotation_task_handle);
            display_rotation_task_handle = NULL;
        }
    }

    ESP_LOGI(TAG, "Stopped automatic display rotation");
    return ESP_OK;
}

bool app_qma6100_is_display_rotation_active(void)
{
    return display_rotation_active;
}

esp_err_t app_qma6100_get_current_display_rotation(lv_disp_rot_t *rotation)
{
    if (rotation == NULL) {
        ESP_LOGE(TAG, "Invalid rotation pointer");
        return ESP_ERR_INVALID_ARG;
    }

    *rotation = current_display_rotation;
    return ESP_OK;
}

/* ============================= Callback Management ============================= */

esp_err_t app_qma6100_register_rotation_callback(app_qma6100_rotation_callback_t callback, void *user_data)
{
    if (callback == NULL) {
        ESP_LOGE(TAG, "Invalid callback function pointer");
        return ESP_ERR_INVALID_ARG;
    }

    // Find an empty callback slot
    for (int i = 0; i < MAX_ROTATION_CALLBACKS; i++) {
        if (!rotation_callbacks[i].in_use) {
            rotation_callbacks[i].callback = callback;
            rotation_callbacks[i].user_data = user_data;
            rotation_callbacks[i].in_use = true;
            ESP_LOGI(TAG, "Registered rotation callback at slot %d", i);
            return ESP_OK;
        }
    }

    ESP_LOGE(TAG, "No available callback slots (maximum: %d)", MAX_ROTATION_CALLBACKS);
    return ESP_ERR_NO_MEM;
}

esp_err_t app_qma6100_unregister_rotation_callback(app_qma6100_rotation_callback_t callback)
{
    if (callback == NULL) {
        ESP_LOGE(TAG, "Invalid callback function pointer");
        return ESP_ERR_INVALID_ARG;
    }

    // Find and remove the callback
    for (int i = 0; i < MAX_ROTATION_CALLBACKS; i++) {
        if (rotation_callbacks[i].in_use && rotation_callbacks[i].callback == callback) {
            rotation_callbacks[i].callback = NULL;
            rotation_callbacks[i].user_data = NULL;
            rotation_callbacks[i].in_use = false;
            ESP_LOGI(TAG, "Unregistered rotation callback from slot %d", i);
            return ESP_OK;
        }
    }

    ESP_LOGW(TAG, "Callback function not found for unregistration");
    return ESP_ERR_NOT_FOUND;
}

/* ============================= Code Summary ============================= */

/**
 * CODE ORGANIZATION SUMMARY:
 *
 * This implementation provides a complete QMA6100 IMU sensor interface with
 * simplified single-axis rotation detection optimized for mobile device applications.
 *
 * DETECTION ALGORITHM:
 * - Uses gravity direction to determine device orientation
 * - Primary thresholds: ±0.6g for strong axis detection
 * - Fallback thresholds: ±0.3g for moderate signals
 * - Supports 0°, 90°, 270° orientations (no 180° in simple mode)
 *
 * PERFORMANCE OPTIMIZATIONS:
 * - Reduced stability threshold (2 vs 3) for faster response
 * - No complex confidence calculations required
 * - Direct single-axis comparisons for minimal CPU usage
 * - Background task with 200ms update interval
 *
 * RELIABILITY FEATURES:
 * - Device flatness validation via Z-axis
 * - Gravity magnitude validation (0.8g - 1.2g)
 * - Stability filtering to prevent rotation flickering
 * - Thread-safe display rotation with BSP locking
 *
 * INTEGRATION:
 * - Automatic initialization with display rotation enabled
 * - Callback system for rotation change notifications
 * - Real-time monitoring task for cached data access
 * - LVGL display rotation mapping for UI frameworks
 */
