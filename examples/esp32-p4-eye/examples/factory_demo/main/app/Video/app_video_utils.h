#ifndef APP_VIDEO_UTILS_H
#define APP_VIDEO_UTILS_H

esp_err_t app_video_utils_init(void);

esp_err_t app_video_utils_deinit(void);

esp_err_t app_image_process_scale_crop(
    uint8_t *in_buf, uint32_t in_width, uint32_t in_height,
    uint32_t crop_width, uint32_t crop_height,
    uint8_t *out_buf, uint32_t out_width, uint32_t out_height, size_t out_buf_size,
    ppa_srm_rotation_angle_t rotation_angle);

esp_err_t app_image_process_magnify(
    uint8_t *in_buf, uint32_t in_width, uint32_t in_height,
    uint16_t magnification_factor,
    uint8_t *out_buf, size_t out_buf_size); 

esp_err_t app_image_process_video_frame(
    uint8_t *in_buf, uint32_t in_width, uint32_t in_height,
    int scale_level, ppa_srm_rotation_angle_t rotation_angle,
    uint8_t *out_buf, size_t out_buf_size);

esp_err_t app_image_encode_jpeg(
    uint8_t *src_buf, 
    uint32_t width, 
    uint32_t height, 
    uint8_t quality,
    uint8_t *out_buf, 
    size_t out_buf_size, 
    uint32_t *out_size);

void swap_rgb565_bytes(uint16_t *buffer, int pixel_count);

#endif
