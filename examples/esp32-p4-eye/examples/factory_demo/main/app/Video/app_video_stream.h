/**
 * @file app_video_stream.h
 * @brief Video streaming application interface
 *
 * This module provides functionality for initializing and managing video streaming
 * from a camera to a display. It handles camera initialization, buffer management,
 * frame processing with scaling capabilities, photo capture, and interval photography.
 */

#pragma once

#include <esp_err.h>
#include "driver/i2c_master.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Photo resolution enumeration
 */
typedef enum {
    PHOTO_RESOLUTION_480P = 0,  // 640x480
    PHOTO_RESOLUTION_720P = 1,  // 1280x720
    PHOTO_RESOLUTION_1080P = 2, // 1920x1080
    PHOTO_RESOLUTION_MAX
} photo_resolution_t;

/* Resolution constants */
#define PHOTO_WIDTH_480P    640
#define PHOTO_HEIGHT_480P   480
#define PHOTO_WIDTH_720P    1280
#define PHOTO_HEIGHT_720P   720
#define PHOTO_WIDTH_1080P   1920
#define PHOTO_HEIGHT_1080P  1080

/* JPEG encoder aligned height (16-byte aligned) */
#define PHOTO_HEIGHT_1088P  1088

/* Default shared photo buffer size (720P max) */
#define SHARED_PHOTO_BUF_WIDTH   PHOTO_WIDTH_720P
#define SHARED_PHOTO_BUF_HEIGHT  PHOTO_HEIGHT_720P

/* Initialization */
/**
 * @brief Initialize the video streaming application
 *
 * This function initializes the PPA (Parallel Pixel Accelerator), camera, and
 * allocates necessary buffers for video streaming. It sets up the video capture
 * device and starts the streaming task.
 *
 * @param i2c_handle I2C master bus handle for camera communication
 * @return ESP_OK on success, or an error code on failure
 */
esp_err_t app_video_stream_init(i2c_master_bus_handle_t i2c_handle);

/**
 * @brief Deinitialize the video streaming application
 *
 * This function cleans up resources, unregisters callbacks, and stops all
 * video streaming related tasks and processes.
 *
 * @return ESP_OK on success, or an error code on failure
 */
esp_err_t app_video_stream_deinit(void);

/* Photo and video control */
/**
 * @brief Take a photo with the current settings
 *
 * @return ESP_OK on success, or an error code on failure
 */
esp_err_t app_video_stream_take_photo(void);

/**
 * @brief Stop photo capture process
 *
 * @return ESP_OK on success, or an error code on failure
 */
esp_err_t app_video_stream_stop_take_photo(void);

/**
 * @brief Start video recording
 *
 * @return ESP_OK on success, or an error code on failure
 */
esp_err_t app_video_stream_take_video(void);

/**
 * @brief Stop video recording
 *
 * @return ESP_OK on success, or an error code on failure
 */
esp_err_t app_video_stream_stop_take_video(void);

/**
 * @brief Set flash light state
 *
 * @param is_on Whether to turn flash on or off
 * @return ESP_OK on success, or an error code on failure
 */
esp_err_t app_video_stream_set_flash_light(bool is_on);

/* Interval photo functions */
/**
 * @brief Start interval photo mode
 *
 * @param interval_minutes Interval between photos in minutes
 * @return ESP_OK on success, or an error code on failure
 */
esp_err_t app_video_stream_start_interval_photo(uint16_t interval_minutes);

/**
 * @brief Stop interval photo mode
 *
 * @return ESP_OK on success, or an error code on failure
 */
esp_err_t app_video_stream_stop_interval_photo(void);

/**
 * @brief Check if device woke up for interval photo
 *
 * @return ESP_OK if interval photo should continue, ESP_FAIL otherwise
 */
esp_err_t app_video_stream_check_interval_wakeup(void);

/* Resolution management */
/**
 * @brief Get current photo resolution
 *
 * @return Current photo resolution enum value
 */
photo_resolution_t app_video_stream_get_photo_resolution(void);

/**
 * @brief Set photo resolution by string
 *
 * @param resolution_str Resolution string ("480P", "720P", "1080P")
 * @return ESP_OK on success, or an error code on failure
 */
esp_err_t app_video_stream_set_photo_resolution_by_string(const char *resolution_str);

/**
 * @brief Get scaled camera buffer
 *
 * @param buf Pointer to store buffer address
 * @param size Pointer to store buffer size
 */
void app_video_stream_get_scaled_camera_buf(uint8_t **buf, uint32_t *size);

/**
 * @brief Get JPEG buffer
 *
 * @param buf Pointer to store buffer address
 * @param size Pointer to store buffer size
 */
void app_video_stream_get_jpg_buf(uint8_t **buf, uint32_t *size);

/**
 * @brief Get shared photo buffer for record and photo modules
 *
 * @param buf Pointer to store buffer address
 * @param size Pointer to store buffer size (720P max RGB565 format)
 */
void app_video_stream_get_shared_photo_buf(uint8_t **buf, uint32_t *size);

/**
 * @brief Get flash light state
 *
 * @return True if flash light is on, false otherwise
 */
bool app_video_stream_get_flash_light_state(void);

/**
 * @brief Get interval photo state
 *
 * @return True if interval photo is active, false otherwise
 */
bool app_video_stream_get_interval_photo_state(void);

/**
 * @brief Get current interval minutes
 *
 * @return Current interval minutes
 */
uint16_t app_video_stream_get_current_interval_minutes(void);

/**
 * @brief Get video file descriptor
 *
 * @return Video file descriptor
 */
int app_video_stream_get_video_fd(void);

#ifdef __cplusplus
}
#endif