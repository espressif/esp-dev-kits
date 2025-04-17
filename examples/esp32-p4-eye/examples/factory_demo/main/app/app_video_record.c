/**
 * @file app_video_record.c
 * @brief Video recording functionality implementation
 */
#include <sys/stat.h> 
#include <dirent.h>
#include <stdio.h>
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_private/esp_cache_private.h"
#include "driver/ppa.h"
#include "driver/jpeg_encode.h"
#include "driver/jpeg_decode.h"
#include "bsp/esp-bsp.h"

#include "esp_audio_enc_default.h"
#include "esp_audio_enc_reg.h"
#include "esp_audio_enc.h"
#include "esp_muxer.h"
#include "mp4_muxer.h"

#include "ui_extra.h"
#include "app_video.h"
#include "app_storage.h"
#include "app_video_stream.h"
#include "app_album.h"
#include "app_video_utils.h"
#include "app_video_record.h"

/* Constants */
#define ALIGN_UP(num, align)       (((num) + ((align) - 1)) & ~((align) - 1))
#define JPEG_VIDEO_QUALITY         80            // JPEG quality setting
#define CROP_PHOTO_WIDTH           1280
#define CROP_PHOTO_HEIGHT          960
#define FILE_SLICE_DURATION        600000
#define VIDEO_BUF_MULTIPLIER       10
#define VIDEO_FRAME_RATE           30
#define REC_AUDIO_SAMPLE_RATE      16000
#define REC_AUDIO_CHANNEL          2
#define REC_AUDIO_BITS_PER_SAMPLE  16

static const char *TAG = "app_video_record";

/**
 * @brief Recorder context structure
 */
typedef struct {
    esp_muxer_handle_t muxer;           // MP4 muxer handle
    int video_stream_idx;               // Video stream index
    int audio_stream_idx;               // Audio stream index
    bool recording;                     // Recording status flag
    SemaphoreHandle_t recording_mutex;  // Recording mutex
    esp_audio_enc_handle_t encoder;     // Audio encoder handle
    uint32_t start_time;                // Recording start time
    TaskHandle_t audio_capture_task_handle; // Audio capture task handle
    uint32_t video_frame_count;         // Video frame count
} recorder_ctx_t;

/* Static variables */
static recorder_ctx_t recorder_ctx = {
    .muxer = NULL,
    .video_stream_idx = -1,
    .audio_stream_idx = -1,
    .recording = false,
    .recording_mutex = NULL,
    .encoder = NULL,
    .start_time = 0,
    .video_frame_count = 0
};

static size_t data_cache_line_size = 0;
static photo_resolution_t current_resolution = PHOTO_RESOLUTION_1080P;
static const uint32_t photo_resolution_width[PHOTO_RESOLUTION_MAX] = {640, 1280, 1920};
static const uint32_t photo_resolution_height[PHOTO_RESOLUTION_MAX] = {480, 720, 1080};

static uint8_t *scaled_camera_buf = NULL;
static uint32_t scaled_camera_buf_size = 0;
static uint8_t *photo_buf = NULL;
static uint8_t *jpg_buf = NULL;
static uint32_t rx_buffer_size = 0;
static uint32_t jpg_size = 0;

/* Forward declarations */
static esp_err_t init_mp4_muxer(void);
static esp_err_t deinit_mp4_muxer(void);
static int get_next_file_number(const char *dir_path);
static int file_pattern_cb(char *file_name, int len, int slice_idx);
static void audio_capture_encode_task(void *pvParameters);

/* Public function implementations */

/**
 * @brief Set video resolution
 * 
 * @param resolution Resolution enum value
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t app_video_stream_set_video_resolution(photo_resolution_t resolution)
{
    if (resolution >= PHOTO_RESOLUTION_MAX) {
        return ESP_ERR_INVALID_ARG;
    }
    
    current_resolution = resolution;
    ESP_LOGI(TAG, "Photo resolution set to %ldx%ld", 
             photo_resolution_width[current_resolution],
             photo_resolution_height[current_resolution]);
    
    return ESP_OK;
}

/**
 * @brief Process and save a video frame
 * 
 * @param camera_buf Camera buffer containing the image
 * @param width Image width
 * @param height Image height
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t take_and_save_video(uint8_t *camera_buf, uint32_t width, uint32_t height)
{
    esp_err_t ret = ESP_OK;
    uint8_t *pic_buf = NULL;

    uint32_t photo_width = photo_resolution_width[current_resolution];
    uint32_t photo_height = photo_resolution_height[current_resolution];

    // Adjust resolution to match camera capabilities
    if (photo_width > width) {
        ESP_LOGW(TAG, "Requested width %ld exceeds camera capability %ld, adjusting", photo_width, width);
        photo_width = width;
    }
    if (photo_height > height) {
        ESP_LOGW(TAG, "Requested height %ld exceeds camera capability %ld, adjusting", photo_height, height);
        photo_height = height;
    }

    uint16_t magnification_factor = app_extra_get_magnification_factor();
    uint8_t *pre_handle_buf = camera_buf;

    // Apply magnification if needed
    if(magnification_factor > 1) {
        ret = app_image_process_magnify(
            camera_buf, width, height,
            magnification_factor,
            scaled_camera_buf, 
            ALIGN_UP(width * height * 2, data_cache_line_size)
        );
        
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to magnify image: 0x%x", ret);
            goto cleanup;
        }

        pre_handle_buf = scaled_camera_buf;
    } else {
        pre_handle_buf = camera_buf;
    }

    // Process image based on resolution
    if(current_resolution != PHOTO_RESOLUTION_1080P) {
        photo_buf = (uint8_t*)heap_caps_aligned_calloc(data_cache_line_size, 1, 
                                                       photo_width * photo_height * 2, 
                                                       MALLOC_CAP_SPIRAM);
        if (photo_buf == NULL) {
            ESP_LOGE(TAG, "Failed to allocate photo buffer");
            ret = ESP_FAIL;
            goto cleanup;
        }

        ret = app_image_process_scale_crop(
            pre_handle_buf, width, height,
            CROP_PHOTO_WIDTH, CROP_PHOTO_HEIGHT,
            photo_buf, photo_width, photo_height,
            ALIGN_UP(photo_width * photo_height * 2, data_cache_line_size),
            PPA_SRM_ROTATION_ANGLE_0
        );
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to scale image: 0x%x", ret);
            goto cleanup;
        }
        pic_buf = photo_buf;
    } else {
        if(magnification_factor > 1) {
            pic_buf = scaled_camera_buf;
        } else {
            pic_buf = camera_buf;
        }
    }

    ret = app_image_encode_jpeg(
        pic_buf,
        photo_width,
        photo_height,
        JPEG_VIDEO_QUALITY,
        jpg_buf,
        rx_buffer_size,
        &jpg_size
    );
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "JPEG encoding failed: 0x%x", ret);
        goto cleanup;
    }

    jpeg_decode_picture_info_t header_info;
    ret = jpeg_decoder_get_info(jpg_buf, jpg_size, &header_info);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Invalid JPEG data: failed to parse header (error %d)", ret);
        return ret;
    }

    uint32_t frame_time = esp_timer_get_time() / 1000 - recorder_ctx.start_time;
    ESP_LOGD(TAG, "frame_time: %ld", frame_time);

    xSemaphoreTake(recorder_ctx.recording_mutex, portMAX_DELAY);
    if (recorder_ctx.recording) {
        esp_muxer_video_packet_t video_packet = {
            .data = jpg_buf,
            .len = jpg_size,
            .pts = frame_time,
            .dts = frame_time,
            .key_frame = (recorder_ctx.video_frame_count % 30 == 0), 
        };
        
        int ret = esp_muxer_add_video_packet(recorder_ctx.muxer, recorder_ctx.video_stream_idx, &video_packet);
        if (ret != ESP_MUXER_ERR_OK) {
            ESP_LOGE(TAG, "Failed to add video packet, error: %x", ret);
        } else {
            recorder_ctx.video_frame_count++;
        }
    }
    xSemaphoreGive(recorder_ctx.recording_mutex);
    
cleanup:
    // Free resources
    if (photo_buf != NULL && pic_buf == photo_buf) {
        heap_caps_free(photo_buf);
        photo_buf = NULL;
    }

    return ret;
}

/**
 * @brief Start video recording
 * 
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t app_video_stream_start_recording(void)
{
    esp_err_t ret = ESP_OK;
    ESP_LOGI(TAG, "Starting recording");

    // Create mutex
    recorder_ctx.recording_mutex = xSemaphoreCreateMutex();
    if (!recorder_ctx.recording_mutex) {
        ESP_LOGE(TAG, "Failed to create mutex");
        return ESP_FAIL;
    }

    // Initialize MP4 muxer
    ret = init_mp4_muxer();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize MP4 muxer: 0x%x", ret);
        return ret;
    }

    // Set recording flag
    recorder_ctx.recording = true;
    recorder_ctx.start_time = esp_timer_get_time() / 1000;

    // Create audio capture and encode task
    xTaskCreatePinnedToCore(audio_capture_encode_task, "audio_cap_enc_task", 4096, NULL, 5, &recorder_ctx.audio_capture_task_handle, 0);

    ESP_LOGI(TAG, "Recording started at %lu", recorder_ctx.start_time);

    return ret;
}

/**
 * @brief Stop video recording
 * 
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t app_video_stream_stop_recording(void)
{
    esp_err_t ret = ESP_OK;
    ESP_LOGI(TAG, "Stopping recording");

    // Set stop flag
    xSemaphoreTake(recorder_ctx.recording_mutex, portMAX_DELAY);
    recorder_ctx.recording = false;
    xSemaphoreGive(recorder_ctx.recording_mutex);

    if (recorder_ctx.audio_capture_task_handle != NULL) {
        vTaskDelay(pdMS_TO_TICKS(100));
        if (eTaskGetState(recorder_ctx.audio_capture_task_handle) != eDeleted) {
            ESP_LOGI(TAG, "Waiting for audio capture task to end");
            for (int i = 0; i < 30 && eTaskGetState(recorder_ctx.audio_capture_task_handle) != eDeleted; i++) {
                vTaskDelay(pdMS_TO_TICKS(100));
            }
            if (eTaskGetState(recorder_ctx.audio_capture_task_handle) != eDeleted) {
                ESP_LOGW(TAG, "Force deleting audio capture task");
                vTaskDelete(recorder_ctx.audio_capture_task_handle);
            }
        }
        recorder_ctx.audio_capture_task_handle = NULL;
    }

    recorder_ctx.video_frame_count = 0;
    
    // Deinitialize MP4 muxer
    ret = deinit_mp4_muxer();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to deinitialize MP4 muxer: 0x%x", ret);
    }

    if (recorder_ctx.recording_mutex) {
        vSemaphoreDelete(recorder_ctx.recording_mutex);
        recorder_ctx.recording_mutex = NULL;
    }
    
    ESP_LOGI(TAG, "Recording stopped");

    return ret;
}

/**
 * @brief Initialize video recording module
 * 
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t app_video_record_init(void)
{
    esp_err_t ret = esp_cache_get_alignment(MALLOC_CAP_SPIRAM, &data_cache_line_size);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get data cache line size: 0x%x", ret);
        return ret;
    }

    app_video_stream_get_scaled_camera_buf(&scaled_camera_buf, &scaled_camera_buf_size);
    app_video_stream_get_jpg_buf(&jpg_buf, &rx_buffer_size);

    // Initialize PDM codec
    ret = bsp_extra_pdm_codec_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize PDM codec: 0x%x", ret);
    }

    // Register PCM encoder
    ESP_ERROR_CHECK(esp_pcm_enc_register());

    esp_pcm_enc_config_t pcm_cfg = {
        .sample_rate = REC_AUDIO_SAMPLE_RATE,
        .channel = REC_AUDIO_CHANNEL,
        .bits_per_sample = REC_AUDIO_BITS_PER_SAMPLE,
    };

    esp_audio_enc_config_t enc_cfg = {
        .type = ESP_AUDIO_TYPE_PCM,
        .cfg = &pcm_cfg,
        .cfg_sz = sizeof(pcm_cfg)
    };

    ESP_ERROR_CHECK(esp_audio_enc_open(&enc_cfg, &recorder_ctx.encoder));

    return ret;
}

/* Private function implementations */

/**
 * @brief Get the next available file number by scanning the directory
 * 
 * @param dir_path Directory path to scan
 * @return Next available file number
 */
static int get_next_file_number(const char *dir_path)
{
    DIR *dir = opendir(dir_path);
    if (dir == NULL) {
        ESP_LOGE(TAG, "Failed to open directory: %s", dir_path);
        return 0;
    }
    
    int max_num = -1;
    struct dirent *entry;
    
    // Scan all files in the directory
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {  // Regular file
            int file_num;
            // Try to parse file number from "mp4_X.mp4" format
            if (sscanf(entry->d_name, "mp4_%d.mp4", &file_num) == 1) {
                if (file_num > max_num) {
                    max_num = file_num;
                }
            }
        }
    }
    
    closedir(dir);
    return max_num + 1;  // Return next available number
}

/**
 * @brief File name pattern callback function for MP4 muxer
 * 
 * @param file_name Buffer to store the generated file name
 * @param len Buffer length
 * @param slice_idx Slice index
 * @return 0 on success, -1 on failure
 */
static int file_pattern_cb(char *file_name, int len, int slice_idx)
{
    // Ensure directory exists
    char dir_path[64];
    snprintf(dir_path, sizeof(dir_path), "%s/esp32_p4_mp4_save", BSP_SD_MOUNT_POINT);
    
    struct stat st;
    if (stat(dir_path, &st) != 0) {
        // Directory doesn't exist, create it
        if (mkdir(dir_path, 0755) != 0) {
            ESP_LOGE(TAG, "Failed to create directory: %s", dir_path);
            return -1;
        }
        ESP_LOGI(TAG, "Created directory: %s", dir_path);
    }
    
    // Get the next available file number
    int file_num = get_next_file_number(dir_path);
    
    // Generate filename with new format
    snprintf(file_name, len, "%s/esp32_p4_mp4_save/mp4_%d.mp4", 
             BSP_SD_MOUNT_POINT, file_num);
    
    ESP_LOGI(TAG, "Creating file: %s", file_name);
    return 0;
}

/**
 * @brief Initialize MP4 muxer
 * 
 * @return ESP_OK on success, error code otherwise
 */
static esp_err_t init_mp4_muxer(void)
{
    ESP_LOGI(TAG, "Initializing MP4 muxer");
    
    // Register MP4 muxer
    mp4_muxer_register();
    
    // Configure MP4 muxer
    mp4_muxer_config_t mp4_cfg = {0};
    esp_muxer_config_t *base_cfg = &mp4_cfg.base_config;
    base_cfg->muxer_type = ESP_MUXER_TYPE_MP4;
    base_cfg->url_pattern = file_pattern_cb;
    base_cfg->slice_duration = FILE_SLICE_DURATION;
    base_cfg->ram_cache_size = 32 * 1024;  // 32KB cache
    mp4_cfg.display_in_order = true;
    
    // Open muxer
    recorder_ctx.muxer = esp_muxer_open(base_cfg, sizeof(mp4_muxer_config_t));
    if (recorder_ctx.muxer == NULL) {
        ESP_LOGE(TAG, "Failed to open muxer");
        return ESP_FAIL;
    }
    
    // Add video stream
    esp_muxer_video_stream_info_t video_stream = {
        .width = photo_resolution_width[current_resolution],
        .height = photo_resolution_height[current_resolution],
        .fps = VIDEO_FRAME_RATE,
        .codec = ESP_MUXER_VDEC_MJPEG,  
    };
    int ret = esp_muxer_add_video_stream(recorder_ctx.muxer, &video_stream, &recorder_ctx.video_stream_idx);
    if (ret != ESP_MUXER_ERR_OK) {
        ESP_LOGE(TAG, "Failed to add video stream, error: %d", ret);
        return ESP_FAIL;
    }
    
    // Add audio stream
    esp_muxer_audio_stream_info_t audio_stream = {
        .bits_per_sample = REC_AUDIO_BITS_PER_SAMPLE,
        .sample_rate = REC_AUDIO_SAMPLE_RATE,
        .channel = REC_AUDIO_CHANNEL,  
        .codec = ESP_MUXER_ADEC_PCM,  
    };
    ret = esp_muxer_add_audio_stream(recorder_ctx.muxer, &audio_stream, &recorder_ctx.audio_stream_idx);
    if (ret != ESP_MUXER_ERR_OK) {
        ESP_LOGE(TAG, "Failed to add audio stream, error: %d", ret);
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "MP4 muxer initialized successfully");
    return ESP_OK;
}

/**
 * @brief Deinitialize MP4 muxer
 * 
 * @return ESP_OK on success, error code otherwise
 */
static esp_err_t deinit_mp4_muxer(void)
{
    esp_err_t ret = ESP_OK;
    ESP_LOGI(TAG, "Deinitializing MP4 muxer");
    
    // Close muxer
    if (recorder_ctx.muxer) {
        esp_muxer_close(recorder_ctx.muxer);
        recorder_ctx.muxer = NULL;
        ESP_LOGI(TAG, "MP4 muxer closed successfully");
    }
    
    // unregister all muxer
    esp_muxer_unreg_all();  

    ESP_LOGI(TAG, "MP4 muxer deinitialized successfully");
    return ret;
}

/**
 * @brief Combined audio capture and encode task
 * 
 * @param pvParameters Task parameters (unused)
 */
static void audio_capture_encode_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Audio capture and encode task started");

    int pcm_frame_size = 0, output_frame_size = 0;
    esp_audio_enc_get_frame_size(recorder_ctx.encoder, &pcm_frame_size, &output_frame_size);
    ESP_LOGI(TAG, "pcm_frame_size: %d, output_frame_size: %d", pcm_frame_size, output_frame_size);
    
    // Use a single buffer for audio capture
    uint8_t *sample_buffer = heap_caps_aligned_calloc(data_cache_line_size, 1, 
                                                     pcm_frame_size * VIDEO_BUF_MULTIPLIER, 
                                                     MALLOC_CAP_SPIRAM);
    if (sample_buffer == NULL) {
        ESP_LOGE(TAG, "Failed to allocate sample buffer");
        vTaskDelete(NULL);
        return;
    }
    
    // Allocate encode buffer
    size_t enc_size = output_frame_size * VIDEO_BUF_MULTIPLIER;
    uint8_t *enc_data = heap_caps_aligned_calloc(data_cache_line_size, 1, 
                                                enc_size, 
                                                MALLOC_CAP_SPIRAM);
    if (enc_data == NULL) {
        ESP_LOGE(TAG, "Failed to allocate encode buffer");
        free(sample_buffer);
        vTaskDelete(NULL);
        return;
    }
    
    while (recorder_ctx.recording) {
        // Capture audio data
        size_t bytes_read = 0;
        esp_err_t ret = bsp_extra_pdm_i2s_read(sample_buffer, pcm_frame_size * 10, &bytes_read, portMAX_DELAY);
        if (ret != ESP_OK || bytes_read == 0) {
            ESP_LOGE(TAG, "I2S read failed: %x, bytes_read: %zu", ret, bytes_read);
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }
        
        // Encode audio data directly
        esp_audio_enc_in_frame_t in_frame = {
            .buffer = sample_buffer,
            .len = bytes_read,
        };
        esp_audio_enc_out_frame_t out_frame = {
            .buffer = enc_data,
            .len = enc_size,
        };
        ret = esp_audio_enc_process(recorder_ctx.encoder, &in_frame, &out_frame);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Audio encoding failed: %x", ret);
            continue;
        }
        
        // Add encoded audio packet to muxer
        uint32_t audio_timestamp = esp_timer_get_time() / 1000 - recorder_ctx.start_time;
        ESP_LOGD(TAG, "audio_timestamp: %ld", audio_timestamp);
        
        xSemaphoreTake(recorder_ctx.recording_mutex, portMAX_DELAY);
        if (recorder_ctx.recording) {
            esp_muxer_audio_packet_t audio_packet = {
                .data = enc_data,
                .len = out_frame.len,
                .pts = audio_timestamp,
            };
            
            ret = esp_muxer_add_audio_packet(recorder_ctx.muxer, recorder_ctx.audio_stream_idx, &audio_packet);
            if (ret != ESP_MUXER_ERR_OK) {
                ESP_LOGE(TAG, "Failed to add audio packet, error: %x", ret);
            }
        }
        xSemaphoreGive(recorder_ctx.recording_mutex);
    }
    
    // Free resources
    free(enc_data);
    free(sample_buffer);
    
    ESP_LOGI(TAG, "Audio capture and encode task ended");
    vTaskDelete(NULL);
}