#ifndef APP_VIDEO_PHOTO_H
#define APP_VIDEO_PHOTO_H

/**
 * @brief Initialize the photo module
 * 
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t app_video_photo_init(void);

/**
 * @brief Deinitialize the photo module
 * 
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t app_video_photo_deinit(void);

/**
 * @brief Process and save a photo
 * 
 * @param camera_buf Camera buffer containing the image
 * @param width Image width
 * @param height Image height
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t take_and_save_photo(uint8_t *camera_buf, uint32_t width, uint32_t height);

/**
 * @brief Set photo resolution
 * 
 * @param resolution Resolution enum value
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t app_video_stream_set_photo_resolution(photo_resolution_t resolution);
#endif
