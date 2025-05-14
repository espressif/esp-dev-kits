#ifndef ESP_QMA6100_H
#define ESP_QMA6100_H

#include "esp_err.h"
#include "qma6100p.h"
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================= Type Definitions ============================= */

/**
 * @brief IMU angle data structure for roll and pitch angles
 * 
 * Roll: Rotation around X-axis (left/right tilt)
 * Pitch: Rotation around Y-axis (forward/backward tilt)
 */
typedef struct {
    float roll;     ///< Roll angle in degrees (rotation around X-axis)
    float pitch;    ///< Pitch angle in degrees (rotation around Y-axis)
} qma6100_angle_t;

/**
 * @brief Device rotation orientations
 */
typedef enum {
    QMA6100_ROTATION_0_DEGREE = 0,      ///< Normal orientation (portrait)
    QMA6100_ROTATION_90_DEGREE,         ///< Rotated 90° clockwise (landscape right)
    QMA6100_ROTATION_180_DEGREE,        ///< Upside down (portrait inverted)
    QMA6100_ROTATION_270_DEGREE,        ///< Rotated 270° clockwise (landscape left)
    QMA6100_ROTATION_UNKNOWN,           ///< Cannot determine rotation
    QMA6100_ROTATION_TILTED             ///< Device is tilted, not flat enough
} qma6100_rotation_t;

/**
 * @brief Confidence levels for rotation detection
 */
typedef enum {
    QMA6100_CONFIDENCE_HIGH = 0,        ///< High confidence (>90% accuracy)
    QMA6100_CONFIDENCE_MEDIUM,          ///< Medium confidence (70-90% accuracy)
    QMA6100_CONFIDENCE_LOW,             ///< Low confidence (50-70% accuracy)
    QMA6100_CONFIDENCE_NONE             ///< No confidence (<50% accuracy)
} qma6100_confidence_t;

/**
 * @brief Complete rotation detection result with metadata
 */
typedef struct {
    qma6100_rotation_t rotation;        ///< Detected rotation angle
    qma6100_confidence_t confidence;    ///< Detection confidence level
    float raw_angle;                    ///< Raw atan2 angle (0-360°)
    bool is_flat;                       ///< Device flatness status for reliable detection
    float magnitude;                    ///< Gravity magnitude (should be ~1.0g)
} qma6100_rotation_result_t;

/**
 * @brief Real-time IMU data package
 */
typedef struct {
    qma6100p_acce_value_t acceleration; ///< Raw acceleration data (X, Y, Z axes)
    qma6100_angle_t angles;             ///< Calculated roll and pitch angles
    float magnitude;                    ///< Gravity magnitude
} qma6100_realtime_data_t;

/**
 * @brief Callback function type for display rotation change notifications
 * 
 * @param new_rotation The new LVGL display rotation value
 * @param qma_rotation The QMA6100 rotation that triggered this change
 * @param user_data User-provided data passed to the callback
 */
typedef void (*app_qma6100_rotation_callback_t)(lv_disp_rot_t new_rotation, 
                                                qma6100_rotation_t qma_rotation, 
                                                void *user_data);

/* ============================= Core Functions ============================= */

/**
 * @brief Initialize QMA6100 sensor with integrated display auto-rotation
 * 
 * Configures the sensor with default settings (±2g range) and automatically
 * starts the display auto-rotation task for complete functionality. After
 * successful initialization, the device will automatically rotate the display
 * based on orientation changes.
 * 
 * @param i2c_bus_handle I2C bus handle from BSP initialization
 * @return ESP_OK on success, ESP_ERR_* on failure
 * 
 * @note The display rotation task is started automatically. If it fails to start,
 *       a warning is logged but the sensor initialization is still considered successful.
 */
esp_err_t app_qma6100_init(i2c_master_bus_handle_t i2c_bus_handle);

/**
 * @brief Deinitialize QMA6100 sensor and cleanup resources
 * 
 * Stops all running tasks and releases sensor resources.
 * 
 * @return ESP_OK on success
 */
esp_err_t app_qma6100_deinit(void);

/* ============================= Data Access Functions ============================= */

/**
 * @brief Get raw accelerometer data from QMA6100 sensor
 * 
 * @param acce_data Pointer to store raw acceleration data
 * @return ESP_OK on success, ESP_ERR_* on failure
 */
esp_err_t app_qma6100_get_data(qma6100p_acce_value_t *acce_data);

/**
 * @brief Calculate roll and pitch angles from acceleration data
 * 
 * Uses atan2 functions to compute device orientation angles from 
 * 3-axis acceleration data.
 * 
 * @param acce_data Input acceleration data
 * @param angles Pointer to store calculated angles
 * @return ESP_OK on success, ESP_ERR_INVALID_ARG if parameters are NULL
 */
esp_err_t app_qma6100_calculate_angles(const qma6100p_acce_value_t *acce_data, 
                                      qma6100_angle_t *angles);

/**
 * @brief Get comprehensive real-time IMU data
 * 
 * Retrieves acceleration data, calculates angles, and computes gravity magnitude
 * in a single call.
 * 
 * @param imu_data Pointer to store complete IMU data package
 * @return ESP_OK on success, ESP_ERR_* on failure
 */
esp_err_t app_qma6100_get_realtime_data(qma6100_realtime_data_t *imu_data);

/* ============================= Real-time Monitoring ============================= */

/**
 * @brief Start continuous real-time IMU angle monitoring task
 * 
 * Creates a background task that continuously samples IMU data at the 
 * specified rate and caches the latest values for quick access.
 * 
 * @param sample_rate_ms Sample rate in milliseconds (10-10000ms valid range)
 * @return ESP_OK on success, ESP_ERR_* on failure
 */
esp_err_t app_qma6100_start_realtime_monitoring(uint32_t sample_rate_ms);

/**
 * @brief Stop real-time IMU angle monitoring task
 * 
 * @return ESP_OK on success
 */
esp_err_t app_qma6100_stop_realtime_monitoring(void);

/**
 * @brief Check if real-time monitoring is currently active
 * 
 * @return true if monitoring task is running, false otherwise
 */
bool app_qma6100_is_monitoring_active(void);

/**
 * @brief Get the latest cached IMU data from real-time monitoring
 * 
 * Provides instant access to the most recent IMU data without sensor I/O.
 * Requires real-time monitoring to be active.
 * 
 * @param imu_data Pointer to store the latest IMU data
 * @return ESP_OK on success, ESP_ERR_NOT_FOUND if no data available
 */
esp_err_t app_qma6100_get_latest_data(qma6100_realtime_data_t *imu_data);

/* ============================= Rotation Detection ============================= */

/**
 * @brief Detect current device rotation angle with confidence assessment
 * 
 * Analyzes acceleration data to determine device orientation (0°, 90°, 180°, 270°)
 * and provides confidence level for the detection. Includes flatness validation
 * to ensure reliable rotation detection.
 * 
 * @param result Pointer to store complete rotation detection result
 * @return ESP_OK on success, ESP_ERR_INVALID_ARG if result is NULL,
 *         ESP_ERR_INVALID_STATE if sensor not initialized
 */
esp_err_t app_qma6100_get_rotation(qma6100_rotation_result_t *result);

/**
 * @brief Simplified single-axis rotation detection for 0°, 90°, 270° (without calibration)
 * 
 * Uses simplified thresholds on X and Y axes to detect basic orientations:
 * - 0° (Portrait): Strong negative Y-axis (ay < -0.6g)
 * - 90° (Landscape Right): Strong negative X-axis (ax < -0.6g) 
 * - 270° (Landscape Left): Strong positive X-axis (ax > 0.6g)
 * - Unknown: All other cases or transition zones
 * 
 * Key Features:
 * - No complex calibration required
 * - Uses fixed physical thresholds based on gravity
 * - Fallback detection for moderate signals
 * - Faster response compared to multi-axis algorithms
 * 
 * @param result Pointer to store rotation detection result
 * @return ESP_OK on success, ESP_ERR_INVALID_ARG if result is NULL,
 *         ESP_ERR_INVALID_STATE if sensor not initialized
 * 
 * @note This method is simpler but may be less accurate than the full detection algorithm.
 *       It requires the device to be relatively flat (checked via Z-axis).
 */
esp_err_t app_qma6100_get_rotation_simple(qma6100_rotation_result_t *result);

/**
 * @brief Convert rotation enum to human-readable string
 * 
 * @param rotation Rotation enum value
 * @return String representation of rotation angle
 */
const char* app_qma6100_rotation_to_string(qma6100_rotation_t rotation);

/**
 * @brief Convert confidence level to human-readable string
 * 
 * @param confidence Confidence enum value
 * @return String representation of confidence level
 */
const char* app_qma6100_confidence_to_string(qma6100_confidence_t confidence);

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
esp_err_t app_qma6100_start_display_rotation(void);

/**
 * @brief Stop automatic display rotation task
 * 
 * @return ESP_OK on success
 */
esp_err_t app_qma6100_stop_display_rotation(void);

/**
 * @brief Check if display auto-rotation is currently active
 * 
 * @return true if display rotation task is running, false otherwise
 */
bool app_qma6100_is_display_rotation_active(void);

/**
 * @brief Get the current display rotation setting
 * 
 * @param rotation Pointer to store the current LVGL rotation value
 * @return ESP_OK on success, ESP_ERR_INVALID_ARG if rotation pointer is NULL
 */
esp_err_t app_qma6100_get_current_display_rotation(lv_disp_rot_t *rotation);

/* ============================= Callback Management ============================= */

/**
 * @brief Register a callback for display rotation change notifications
 * 
 * Allows registration of up to 3 callback functions that will be called
 * whenever the display rotation changes.
 * 
 * @param callback The callback function to register
 * @param user_data User data to pass to the callback (can be NULL)
 * @return ESP_OK on success, ESP_ERR_INVALID_ARG if callback is NULL,
 *         ESP_ERR_NO_MEM if all callback slots are occupied
 */
esp_err_t app_qma6100_register_rotation_callback(app_qma6100_rotation_callback_t callback, 
                                                 void *user_data);

/**
 * @brief Unregister a previously registered rotation callback
 * 
 * @param callback The callback function to unregister
 * @return ESP_OK on success, ESP_ERR_INVALID_ARG if callback is NULL,
 *         ESP_ERR_NOT_FOUND if callback was not previously registered
 */
esp_err_t app_qma6100_unregister_rotation_callback(app_qma6100_rotation_callback_t callback);

#ifdef __cplusplus
}
#endif

#endif // ESP_QMA6100_H