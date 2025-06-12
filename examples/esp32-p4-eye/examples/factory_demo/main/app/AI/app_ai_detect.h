#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize AI detection module
 * 
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t app_ai_detect_init(void);

/**
 * @brief Initialize AI detection buffers
 * 
 * @param cache_line_size Size of cache line for alignment
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t app_ai_detection_init_buffers(size_t cache_line_size);

/**
 * @brief Process frame for AI detection
 * 
 * @param detect_buf Buffer containing the frame to detect
 * @param width Frame width
 * @param height Frame height
 * @param ai_detect_mode Current AI detection mode
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t app_ai_detection_process_frame(uint8_t *detect_buf, uint32_t width, uint32_t height, int ai_detect_mode);

/**
 * @brief Free AI detection buffers and resources
 * 
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t app_ai_detection_deinit(void);

/**
 * @brief Run COCO object detection on input data
 * 
 * @param data Input image data
 * @param width Image width
 * @param height Image height
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t app_coco_od_detect(uint16_t *data, int width, int height);

/**
 * @brief Detect human faces in input frame and draw results on output buffer
 * 
 * @param detect_buf Input buffer containing the frame
 * @param draw_buf Output buffer to draw detection results
 * @param width Frame width
 * @param height Frame height
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t app_humanface_ai_detect(uint16_t *detect_buf, uint16_t *draw_buf, int width, int height);

/**
 * @brief Detect pedestrians in input frame and draw results on output buffer
 * 
 * @param detect_buf Input buffer containing the frame
 * @param draw_buf Output buffer to draw detection results
 * @param width Frame width
 * @param height Frame height
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t app_pedestrian_ai_detect(uint16_t *detect_buf, uint16_t *draw_buf, int width, int height);

#ifdef __cplusplus
}
#endif
