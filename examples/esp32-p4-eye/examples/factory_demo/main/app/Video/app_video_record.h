#ifndef APP_VIDEO_RECORD_H
#define APP_VIDEO_RECORD_H

/**
 * @brief Initialize video recording module
 * 
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t app_video_record_init(void);

/**
 * @brief Process and save a video frame
 * 
 * @param camera_buf Camera buffer containing the image
 * @param width Image width
 * @param height Image height
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t take_and_save_video(uint8_t *camera_buf, uint32_t width, uint32_t height);

/**
 * @brief Set video resolution
 * 
 * @param resolution Resolution enum value
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t app_video_stream_set_video_resolution(photo_resolution_t resolution);

/**
 * @brief Start video recording
 * 
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t app_video_stream_start_recording(void);

/**
 * @brief Stop video recording
 * 
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t app_video_stream_stop_recording(void);

/**
 * @brief Deinitialize video recording module
 * 
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t app_video_record_deinit(void);

#endif
