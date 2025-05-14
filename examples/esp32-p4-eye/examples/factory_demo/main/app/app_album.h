/**
 * @file app_album.h
 * @brief Album functionality for browsing images from SD card
 */

#pragma once

#include <esp_err.h>
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize album functionality
 * 
 * This function initializes the album interface, creates a canvas control,
 * scans for images on the SD card, and displays the first image.
 * 
 * @param parent Parent LVGL container where the canvas will be created
 * @return ESP_OK on success, ESP_FAIL otherwise
 */
esp_err_t app_album_init(lv_obj_t *parent);

/**
 * @brief Switch to the next image in the album
 * 
 * @return ESP_OK on success, ESP_FAIL otherwise
 */
esp_err_t app_album_next_image(void);

/**
 * @brief Switch to the previous image in the album
 * 
 * @return ESP_OK on success, ESP_FAIL otherwise
 */
esp_err_t app_album_prev_image(void);

/**
 * @brief Refresh album content by rescanning the SD card
 * 
 * This function rescans the SD card for images, useful after
 * taking new photos or when the SD card content has changed.
 * 
 * @return ESP_OK on success, ESP_FAIL otherwise
 */
esp_err_t app_album_refresh(void);

/**
 * @brief Clean up album resources
 * 
 * This function frees all allocated memory and resources.
 * Call this when exiting the album page.
 */
void app_album_deinit(void);

/**
 * @brief Get the number of images in the album
 * 
 * @return Number of images found on the SD card
 */
int app_album_get_image_count(void);

/**
 * @brief Get the current image index
 * 
 * @return Current image index (0-based)
 */
int app_album_get_current_index(void);

/**
 * @brief Delete the current image from SD card
 * 
 * This function deletes the currently displayed image from the SD card
 * and automatically loads the next available image.
 * 
 * @return ESP_OK on success, ESP_FAIL otherwise
 */
esp_err_t app_album_delete_current_image(void);

/**
 * @brief Check if the SD card has enough space to store a new image
 * 
 * @return true if there is enough space, false otherwise
 */
bool app_album_can_store_new_image(void);

/**
 * @brief Notify the album module that a new photo has been saved
 */
void app_album_photo_saved(void);

/**
 * @brief Check if the SD card has enough space to store a new MP4 video
 * 
 * @param estimated_size_mb Estimated video size in MB, uses default estimate if 0
 * @return true if enough space is available, false otherwise
 */
bool app_video_stream_can_store_new_mp4(float estimated_size_mb);

/**
 * @brief Enable or disable COCO OD detection
 * 
 * @param enable true to enable, false to disable
 */
void app_album_enable_coco_od(bool enable);

#ifdef __cplusplus
}
#endif