#include <sys/stat.h> 
#include <dirent.h>
#include <stdio.h>
#include "esp_log.h"
#include "esp_private/esp_cache_private.h"
#include "driver/ppa.h"
#include "driver/jpeg_encode.h"
#include "bsp/esp-bsp.h"

#include "app_video_utils.h"

#define SCALE_LEVELS            4                         // Resolution scale levels

static const char *TAG = "app_video_utils";

static ppa_client_handle_t ppa_srm_handle = NULL;
static jpeg_encoder_handle_t jpeg_handle;

static int scale_level_res[SCALE_LEVELS] = {960, 480, 240, 120};
static const uint32_t adj_resolution_width[SCALE_LEVELS] = {1920, 960, 480, 240};
static const uint32_t adj_resolution_height[SCALE_LEVELS] = {1080, 540, 270, 135};

esp_err_t app_video_utils_init(void)
{
    // Initialize PPA
    ppa_client_config_t ppa_srm_config = {
        .oper_type = PPA_OPERATION_SRM,
    };
    
    esp_err_t ret = ppa_register_client(&ppa_srm_config, &ppa_srm_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register PPA client: 0x%x", ret);
    }

    // Initialize JPEG encoder
    jpeg_encode_engine_cfg_t encode_eng_cfg = {
        .timeout_ms = 70,
    };

    ret = jpeg_new_encoder_engine(&encode_eng_cfg, &jpeg_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create JPEG encoder: 0x%x", ret);
    }

    return ret;
}

esp_err_t app_video_utils_deinit(void)
{
    ppa_unregister_client(ppa_srm_handle);
    ppa_srm_handle = NULL;
    

    if (jpeg_handle != NULL) {
        jpeg_del_encoder_engine(jpeg_handle);
        jpeg_handle = NULL;
    }

    return ESP_OK;
}

/**
 * @brief Generic function to perform image scaling, rotation and mirroring
 * 
 * @param in_buf Input image buffer
 * @param in_width Input image width
 * @param in_height Input image height
 * @param crop_width Crop region width
 * @param crop_height Crop region height
 * @param out_buf Output image buffer
 * @param out_width Output image width
 * @param out_height Output image height
 * @param out_buf_size Output buffer size
 * @param rotation_angle Rotation angle
 * @return esp_err_t ESP_OK on success, error code otherwise
 */
esp_err_t app_image_process_scale_crop(
    uint8_t *in_buf, uint32_t in_width, uint32_t in_height,
    uint32_t crop_width, uint32_t crop_height,
    uint8_t *out_buf, uint32_t out_width, uint32_t out_height, size_t out_buf_size,
    ppa_srm_rotation_angle_t rotation_angle)
{
    ppa_srm_oper_config_t srm_config = {
        .in.buffer = in_buf,
        .in.pic_w = in_width,
        .in.pic_h = in_height,
        .in.block_w = crop_width,
        .in.block_h = crop_height,
        .in.block_offset_x = (in_width - crop_width) / 2,
        .in.block_offset_y = (in_height - crop_height) / 2,
        .in.srm_cm = PPA_SRM_COLOR_MODE_RGB565,
        .out.buffer = out_buf,
        .out.buffer_size = out_buf_size,
        .out.pic_w = out_width,
        .out.pic_h = out_height,
        .out.block_offset_x = 0,
        .out.block_offset_y = 0,
        .out.srm_cm = PPA_SRM_COLOR_MODE_RGB565,
        .rotation_angle = rotation_angle,
        .scale_x = (float)out_width / crop_width,
        .scale_y = (float)out_height / crop_height,
        .rgb_swap = 0,
        .byte_swap = 0,
        .mode = PPA_TRANS_MODE_BLOCKING,
    };

    return ppa_do_scale_rotate_mirror(ppa_srm_handle, &srm_config);
}

/**
 * @brief Perform image magnification processing
 * 
 * @param in_buf Input image buffer
 * @param in_width Input image width
 * @param in_height Input image height
 * @param magnification_factor Magnification factor
 * @param out_buf Output image buffer
 * @param out_buf_size Output buffer size
 * @return esp_err_t ESP_OK on success, error code otherwise
 */
esp_err_t app_image_process_magnify(
    uint8_t *in_buf, uint32_t in_width, uint32_t in_height,
    uint16_t magnification_factor,
    uint8_t *out_buf, size_t out_buf_size)
{
    if (magnification_factor < 1 || magnification_factor > SCALE_LEVELS) {
        return ESP_ERR_INVALID_ARG;
    }

    uint32_t crop_width = adj_resolution_width[magnification_factor - 1];
    uint32_t crop_height = adj_resolution_height[magnification_factor - 1];

    return app_image_process_scale_crop(
        in_buf, in_width, in_height,
        crop_width, crop_height,
        out_buf, in_width, in_height, out_buf_size,
        PPA_SRM_ROTATION_ANGLE_0
    );
}

/**
 * @brief Process video frame for display
 * 
 * @param in_buf Input image buffer
 * @param in_width Input image width
 * @param in_height Input image height
 * @param scale_level Scale level
 * @param rotation_angle Rotation angle for compensation
 * @param out_buf Output image buffer
 * @param out_buf_size Output buffer size
 * @return esp_err_t ESP_OK on success, error code otherwise
 */
esp_err_t app_image_process_video_frame(
    uint8_t *in_buf, uint32_t in_width, uint32_t in_height,
    int scale_level, ppa_srm_rotation_angle_t rotation_angle,
    uint8_t *out_buf, size_t out_buf_size)
{
    if (scale_level < 1 || scale_level > SCALE_LEVELS) {
        return ESP_ERR_INVALID_ARG;
    }

    int res_width = scale_level_res[scale_level - 1];
    int res_height = scale_level_res[scale_level - 1];

    return app_image_process_scale_crop(
        in_buf, in_width, in_height,
        res_width, res_height,
        out_buf, BSP_LCD_H_RES, BSP_LCD_V_RES, out_buf_size,
        rotation_angle
    );
}

/**
 * @brief Encode RGB565 image to JPEG format
 * 
 * @param src_buf Source image buffer in RGB565 format
 * @param width Image width
 * @param height Image height
 * @param quality JPEG quality (0-100)
 * @param out_buf Output JPEG buffer
 * @param out_buf_size Size of output buffer
 * @param out_size Pointer to store the actual JPEG size
 * @return esp_err_t ESP_OK on success, error code otherwise
 */
esp_err_t app_image_encode_jpeg(
    uint8_t *src_buf, 
    uint32_t width, 
    uint32_t height, 
    uint8_t quality,
    uint8_t *out_buf, 
    size_t out_buf_size, 
    uint32_t *out_size)
{
    if (!src_buf || !out_buf || !out_size || width == 0 || height == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    // Configure JPEG encoding
    jpeg_encode_cfg_t enc_config = {
        .src_type = JPEG_ENCODE_IN_FORMAT_RGB565,
        .sub_sample = JPEG_DOWN_SAMPLING_YUV420,
        .image_quality = quality,
        .width = width,
        .height = height,
    };

    // Perform JPEG encoding
    esp_err_t ret = jpeg_encoder_process(
        jpeg_handle, 
        &enc_config, 
        src_buf, 
        width * height * 2, 
        out_buf, 
        out_buf_size, 
        out_size
    );

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "JPEG encoding failed: 0x%x", ret);
    }

    return ret;
}

/* Utility functions */
/**
 * @brief Swap RGB565 bytes for correct display format
 * 
 * @param buffer RGB565 buffer to process
 * @param pixel_count Number of pixels in the buffer
 */
void swap_rgb565_bytes(uint16_t *buffer, int pixel_count)
{
    for (int i = 0; i < pixel_count; i++) {
        uint16_t swap16 = *(buffer + i);
        swap16 = (swap16 >> 8) | (swap16 << 8);
        *(buffer + i) = swap16;
    }
}

