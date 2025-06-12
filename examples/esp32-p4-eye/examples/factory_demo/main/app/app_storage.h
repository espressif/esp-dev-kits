#ifndef APP_STORAGE_H
#define APP_STORAGE_H

#include "esp_err.h"
#include "ui_extra.h"  // For settings_info_t

/**
 * @brief Initialize storage subsystem
 * 
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t app_storage_init(void);

/**
 * @brief Save picture data to SD card
 * 
 * @param data Pointer to image data
 * @param len Length of image data in bytes
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t app_storage_save_picture(const uint8_t *data, size_t len);

/**
 * @brief Save application settings to NVS
 * 
 * @param settings Pointer to settings structure
 * @param interval_time Interval time for timed shooting (in minutes)
 * @param magnification Magnification value
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t app_storage_save_settings(settings_info_t *settings, uint16_t interval_time, uint16_t magnification);

/**
 * @brief Load application settings from NVS
 * 
 * @param settings Pointer to settings structure to be filled
 * @param interval_time Pointer to store interval time
 * @param magnification Pointer to store magnification value
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t app_storage_load_settings(settings_info_t *settings, uint16_t *interval_time, uint16_t *magnification);

/**
 * @brief Save interval shooting state
 * 
 * @param is_active Whether interval shooting is active
 * @param next_wake_time Next wake time for interval shooting
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t app_storage_save_interval_state(bool is_active, uint32_t next_wake_time);

/**
 * @brief Get interval shooting state
 * 
 * @param is_active Pointer to store active state
 * @param next_wake_time Pointer to store next wake time
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t app_storage_get_interval_state(bool *is_active, uint32_t *next_wake_time);

/**
 * @brief save camera settings to nvs
 * 
 * @param contrast contrast percent
 * @param saturation saturation percent
 * @param brightness brightness percent
 * @param hue hue percent
 * @return ESP_OK success, error code otherwise
 */
esp_err_t app_storage_save_camera_settings(uint32_t contrast, uint32_t saturation, 
                                          uint32_t brightness, uint32_t hue);

/**
 * @brief load camera settings from nvs
 * 
 * @param contrast pointer to store contrast percent
 * @param saturation pointer to store saturation percent
 * @param brightness pointer to store brightness percent
 * @param hue pointer to store hue percent
 * @return ESP_OK success, error code otherwise
 */
esp_err_t app_storage_load_camera_settings(uint32_t *contrast, uint32_t *saturation, 
                                          uint32_t *brightness, uint32_t *hue);

/**
 * @brief Save photo count to NVS
 * 
 * @param count Photo count to save
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t app_storage_save_photo_count(uint16_t count);

/**
 * @brief Get photo count from NVS
 * 
 * @param count Pointer to store photo count
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t app_storage_get_photo_count(uint16_t *count);

/**
 * @brief Save gyroscope setting to NVS
 * 
 * @param enabled Whether gyroscope is enabled
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t app_storage_save_gyroscope_setting(bool enabled);

/**
 * @brief Load gyroscope setting from NVS
 * 
 * @param enabled Pointer to store gyroscope enabled state
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t app_storage_load_gyroscope_setting(bool *enabled);

#endif /* APP_STORAGE_H */