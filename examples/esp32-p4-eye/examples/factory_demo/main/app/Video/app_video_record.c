/**
 * @file app_video_record.c
 * @brief Video recording functionality implementation
 */
#include <sys/stat.h> 
#include <dirent.h>
#include <stdio.h>
#include <string.h>
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
#include "esp_pcm_enc.h"
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
#define AUDIO_VIDEO_SYNC_THRESHOLD 100          // Maximum allowed A/V sync drift in ms
#define VIDEO_FRAME_INTERVAL       (1000/VIDEO_FRAME_RATE) // Time in ms between frames at desired FPS
#define MIN_VIDEO_FRAME_INTERVAL   20           // Minimum interval between frames in ms to prevent timestamp jumps

// Dynamic audio offset calculation instead of fixed offset
#define AUDIO_OFFSET_SMOOTH_FACTOR 0.1f         // Smoothing factor for dynamic offset calculation
#define MAX_AUDIO_OFFSET           500          // Maximum allowed audio offset in ms
#define MIN_AUDIO_OFFSET           -500         // Minimum allowed audio offset in ms
#define AUDIO_QUALITY_CHECK_INTERVAL 50         // Check audio quality every N frames

static const char *TAG = "app_video_record";

/**
 * @brief Audio quality monitoring structure
 */
typedef struct {
    uint32_t total_frames;
    uint32_t failed_frames;
    uint32_t overflow_count;
    uint32_t timestamp_corrections;
    float success_rate;
} audio_quality_t;

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
    uint32_t last_video_pts;            // Last video PTS for timing consistency
    uint32_t last_audio_pts;            // Last audio PTS for A/V sync monitoring
    int64_t last_frame_time;            // Timestamp of last frame capture for FPS regulation
    uint32_t actual_fps;                // Actual measured FPS (x100 for precision)
    int32_t dynamic_audio_offset;       // Dynamic audio offset in ms
    audio_quality_t audio_quality;      // Audio quality monitoring
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
    .video_frame_count = 0,
    .last_video_pts = 0,
    .last_audio_pts = 0,
    .last_frame_time = 0,
    .actual_fps = 1500,  // Initial estimate: 15 fps (x100)
    .dynamic_audio_offset = 0,
    .audio_quality = {0}
};

static size_t data_cache_line_size = 0;
static photo_resolution_t current_resolution = PHOTO_RESOLUTION_1080P;
static const uint32_t photo_resolution_width[PHOTO_RESOLUTION_MAX] = {640, 1280, 1920};
static const uint32_t photo_resolution_height[PHOTO_RESOLUTION_MAX] = {480, 720, 1080};

static uint8_t *scaled_camera_buf = NULL;
static uint32_t scaled_camera_buf_size = 0;
static uint8_t *photo_buf = NULL;               // Will point to shared buffer from video_stream
static uint32_t photo_buf_size = 0;             // Size of shared buffer
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
    ESP_LOGI(TAG, "Video resolution set to %ldx%ld", 
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

    // Maintain consistent frame rate by regulating timing
    int64_t current_time = esp_timer_get_time();
    int64_t time_since_last_frame = (current_time - recorder_ctx.last_frame_time) / 1000; // Convert to ms
    
    if (recorder_ctx.last_frame_time != 0 && time_since_last_frame < MIN_VIDEO_FRAME_INTERVAL) {
        // Skip frame processing if frames are coming too quickly
        // This helps maintain a reasonable minimum interval between frames
        vTaskDelay(pdMS_TO_TICKS(MIN_VIDEO_FRAME_INTERVAL - time_since_last_frame));
        current_time = esp_timer_get_time();
        time_since_last_frame = (current_time - recorder_ctx.last_frame_time) / 1000;
    }
    
    // Update actual FPS measurement (using exponential moving average)
    if (recorder_ctx.video_frame_count > 1 && time_since_last_frame > 0) {
        uint32_t instantaneous_fps = 100000 / time_since_last_frame; // FPS x100
        // Update moving average: 90% previous + 10% new measurement
        recorder_ctx.actual_fps = (recorder_ctx.actual_fps * 90 + instantaneous_fps * 10) / 100;
        
        // Sanity check - keep FPS within reasonable bounds
        if (recorder_ctx.actual_fps < 500) {  // Min 5 FPS
            recorder_ctx.actual_fps = 500;
        } else if (recorder_ctx.actual_fps > 3000) {  // Max 30 FPS
            recorder_ctx.actual_fps = 3000;
        }
    }
    
    recorder_ctx.last_frame_time = current_time;

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
        // Use pre-allocated photo buffer instead of dynamic allocation
        if (photo_buf == NULL) {
            ESP_LOGE(TAG, "Photo buffer not initialized");
            ret = ESP_FAIL;
            goto cleanup;
        }

        // Check if resolution exceeds shared buffer capacity (720P max)
        if (photo_width > 1280 || photo_height > 720) {
            ESP_LOGW(TAG, "Resolution %ldx%ld exceeds shared buffer capacity (1280x720), adjusting", 
                     photo_width, photo_height);
            photo_width = (photo_width > 1280) ? 1280 : photo_width;
            photo_height = (photo_height > 720) ? 720 : photo_height;
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

    // Generate frame timestamp based on real time
    uint32_t frame_time = esp_timer_get_time() / 1000 - recorder_ctx.start_time;
    
    // Ensure timestamp never goes backward from previous video timestamp
    if (frame_time <= recorder_ctx.last_video_pts && recorder_ctx.last_video_pts > 0) {
        frame_time = recorder_ctx.last_video_pts + 1;
    }
    
    recorder_ctx.last_video_pts = frame_time;
    ESP_LOGD(TAG, "frame_time: %ld, fps: %ld.%02ld", 
             frame_time, recorder_ctx.actual_fps/100, recorder_ctx.actual_fps%100);

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
    // Note: photo_buf is pre-allocated and managed by init/deinit functions, don't free it here

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

    // Initialize PCM encoder 
    if (recorder_ctx.encoder == NULL) {
        // Configure PCM encoder
        esp_pcm_enc_config_t pcm_cfg = {
            .sample_rate = REC_AUDIO_SAMPLE_RATE,
            .channel = REC_AUDIO_CHANNEL,
            .bits_per_sample = REC_AUDIO_BITS_PER_SAMPLE,
            .frame_duration = 10,  // 10ms frame duration
        };

        esp_audio_enc_config_t enc_cfg = {
            .type = ESP_AUDIO_TYPE_PCM,
            .cfg = &pcm_cfg,
            .cfg_sz = sizeof(pcm_cfg)
        };

        ret = esp_audio_enc_open(&enc_cfg, &recorder_ctx.encoder);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initialize PCM encoder: 0x%x", ret);
            if (recorder_ctx.recording_mutex) {
                vSemaphoreDelete(recorder_ctx.recording_mutex);
                recorder_ctx.recording_mutex = NULL;
            }
            return ret;
        }
        ESP_LOGI(TAG, "PCM encoder initialized");
    }

    // Initialize MP4 muxer
    ret = init_mp4_muxer();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize MP4 muxer: 0x%x", ret);
        if (recorder_ctx.recording_mutex) {
            vSemaphoreDelete(recorder_ctx.recording_mutex);
            recorder_ctx.recording_mutex = NULL;
        }
        return ret;
    }

    // Reset timing variables for new recording
    recorder_ctx.video_frame_count = 0;
    recorder_ctx.last_video_pts = 0;
    recorder_ctx.last_audio_pts = 0;
    recorder_ctx.last_frame_time = 0;
    recorder_ctx.actual_fps = 1500;  // Reset to 15fps (x100) initial estimate
    recorder_ctx.dynamic_audio_offset = 0;  // Reset dynamic audio offset
    memset(&recorder_ctx.audio_quality, 0, sizeof(audio_quality_t));  // Reset audio quality stats

    // Set recording flag
    recorder_ctx.recording = true;
    recorder_ctx.start_time = esp_timer_get_time() / 1000;
    
    ESP_LOGI(TAG, "Recording started at %lu", recorder_ctx.start_time);

    // Create audio capture and encode task
    xTaskCreatePinnedToCore(audio_capture_encode_task, "audio_cap_enc_task", 4096, NULL, 5, &recorder_ctx.audio_capture_task_handle, 0);

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

    // Close PCM encoder
    if (recorder_ctx.encoder != NULL) {
        esp_audio_enc_close(recorder_ctx.encoder);
        recorder_ctx.encoder = NULL;
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

    // Get shared photo buffer from video stream module (1280x720)
    app_video_stream_get_shared_photo_buf(&photo_buf, &photo_buf_size);
    if (photo_buf == NULL) {
        ESP_LOGE(TAG, "Failed to get shared photo buffer from video stream");
        return ESP_ERR_NO_MEM;
    }
    ESP_LOGI(TAG, "Using shared photo buffer: %lu bytes", photo_buf_size);

    // Initialize PDM codec
    ret = bsp_extra_pdm_codec_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize PDM codec: 0x%x", ret);
    }

    // Register PCM encoder
    ESP_ERROR_CHECK(esp_pcm_enc_register());

    // Configure PCM encoder
    esp_pcm_enc_config_t pcm_cfg = {
        .sample_rate = REC_AUDIO_SAMPLE_RATE,
        .channel = REC_AUDIO_CHANNEL,
        .bits_per_sample = REC_AUDIO_BITS_PER_SAMPLE,
        .frame_duration = 10,  // 10ms frame duration
    };

    esp_audio_enc_config_t enc_cfg = {
        .type = ESP_AUDIO_TYPE_PCM,
        .cfg = &pcm_cfg,
        .cfg_sz = sizeof(pcm_cfg)
    };

    ESP_ERROR_CHECK(esp_audio_enc_open(&enc_cfg, &recorder_ctx.encoder));

    return ret;
}

/**
 * @brief Deinitialize video recording module
 * 
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t app_video_record_deinit(void)
{
    // Make sure recording is stopped
    if (recorder_ctx.recording) {
        app_video_stream_stop_recording();
    }

    // Close and release PCM encoder if it's still open
    if (recorder_ctx.encoder != NULL) {
        esp_audio_enc_close(recorder_ctx.encoder);
        recorder_ctx.encoder = NULL;
    }

    // Reset photo buffer pointer (managed by video_stream module)
    photo_buf = NULL;
    photo_buf_size = 0;

    // Unregister PCM encoder
    esp_audio_enc_unregister(ESP_AUDIO_TYPE_PCM);

    ESP_LOGI(TAG, "Video recording module deinitialized");
    return ESP_OK;
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
        .codec = ESP_MUXER_ADEC_PCM,  // Use PCM encoding
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

    // Check if PCM encoder is valid
    if (recorder_ctx.encoder == NULL) {
        ESP_LOGE(TAG, "PCM encoder handle is NULL, cannot proceed with audio capture");
        vTaskDelete(NULL);
        return;
    }

    int pcm_frame_size = 0, output_frame_size = 0;
    esp_err_t ret = esp_audio_enc_get_frame_size(recorder_ctx.encoder, &pcm_frame_size, &output_frame_size);
    if (ret != ESP_OK || pcm_frame_size == 0 || output_frame_size == 0) {
        ESP_LOGE(TAG, "Failed to get PCM frame size: %x, pcm_frame_size: %d, output_frame_size: %d", 
                 ret, pcm_frame_size, output_frame_size);
        vTaskDelete(NULL);
        return;
    }
    ESP_LOGI(TAG, "PCM frame size: input=%d, output=%d", pcm_frame_size, output_frame_size);
    
    // For PCM encoding, allocate input buffer for PCM data
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
    
    // Initialize ring buffer for audio data
    size_t ring_buf_size = pcm_frame_size * 4;  // Buffer 4 frames for smoother processing
    uint8_t *ring_buffer = heap_caps_aligned_calloc(data_cache_line_size, 1, 
                                                  ring_buf_size, 
                                                  MALLOC_CAP_SPIRAM);
    if (ring_buffer == NULL) {
        ESP_LOGE(TAG, "Failed to allocate audio ring buffer");
        free(enc_data);
        free(sample_buffer);
        vTaskDelete(NULL);
        return;
    }
    
    size_t ring_buf_write_pos = 0;
    size_t data_in_ring = 0;

    while (recorder_ctx.recording) {
        // Verify PCM encoder is still valid
        if (recorder_ctx.encoder == NULL) {
            ESP_LOGE(TAG, "PCM encoder became invalid during recording");
            break;
        }

        // Capture PCM audio data with improved error handling
        size_t bytes_read = 0;
        ret = bsp_extra_pdm_i2s_read(sample_buffer, pcm_frame_size, &bytes_read, 100);  // Use 100ms timeout instead of portMAX_DELAY
        if (ret != ESP_OK || bytes_read == 0) {
            recorder_ctx.audio_quality.failed_frames++;
            ESP_LOGW(TAG, "I2S read failed: %x, bytes_read: %zu, continuing...", ret, bytes_read);
            vTaskDelay(pdMS_TO_TICKS(5));  // Reduced delay to 5ms for better responsiveness
            continue;
        }
        
        recorder_ctx.audio_quality.total_frames++;
        
        // Improved ring buffer management with better overflow handling
        if (bytes_read + data_in_ring <= ring_buf_size) {
            // Normal case: add data to ring buffer
            memcpy(ring_buffer + ring_buf_write_pos, sample_buffer, bytes_read);
            ring_buf_write_pos = (ring_buf_write_pos + bytes_read) % ring_buf_size;
            data_in_ring += bytes_read;
        } else {
            // Overflow handling: keep the newest data instead of discarding it
            recorder_ctx.audio_quality.overflow_count++;
            ESP_LOGW(TAG, "Audio ring buffer overflow, keeping newest data");
            
            // Calculate how much old data to discard to make room for new data
            size_t space_needed = bytes_read;
            size_t data_to_discard = space_needed;
            
            // Ensure we don't discard more than what's in the buffer
            if (data_to_discard > data_in_ring) {
                data_to_discard = data_in_ring;
            }
            
            // Shift existing data to make room (keep newer data)
            if (data_in_ring > data_to_discard) {
                memmove(ring_buffer, ring_buffer + data_to_discard, data_in_ring - data_to_discard);
            }
            data_in_ring -= data_to_discard;
            ring_buf_write_pos = data_in_ring;
            
            // Now add new data
            memcpy(ring_buffer + ring_buf_write_pos, sample_buffer, bytes_read);
            ring_buf_write_pos = (ring_buf_write_pos + bytes_read) % ring_buf_size;
            data_in_ring += bytes_read;
        }
        
        // Process audio data if we have enough in the buffer
        if (data_in_ring >= pcm_frame_size) {
            // Use PCM encoder to process audio data
            esp_audio_enc_in_frame_t in_frame = {
                .buffer = ring_buffer,
                .len = pcm_frame_size,
            };
            esp_audio_enc_out_frame_t out_frame = {
                .buffer = enc_data,
                .len = enc_size,
            };
            ret = esp_audio_enc_process(recorder_ctx.encoder, &in_frame, &out_frame);
            
            // Remove used data from ring buffer
            if (data_in_ring > pcm_frame_size) {
                memmove(ring_buffer, ring_buffer + pcm_frame_size, data_in_ring - pcm_frame_size);
            }
            data_in_ring -= pcm_frame_size;
            ring_buf_write_pos = data_in_ring;
            
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "PCM encoding failed: %x", ret);
                recorder_ctx.audio_quality.failed_frames++;
                continue;
            }
            
            // Only add audio packet when data is successfully encoded
            if (out_frame.encoded_bytes > 0) {
                // Generate audio timestamp with dynamic offset
                uint32_t real_audio_time = esp_timer_get_time() / 1000 - recorder_ctx.start_time;
                int32_t dynamic_audio_offset = recorder_ctx.dynamic_audio_offset;
                
                // Improved timestamp calculation with bounds checking
                int64_t calculated_timestamp = (int64_t)real_audio_time + dynamic_audio_offset;
                uint32_t audio_timestamp;
                
                // Ensure timestamp is within reasonable bounds
                if (calculated_timestamp < 0) {
                    audio_timestamp = 0;
                    recorder_ctx.audio_quality.timestamp_corrections++;
                } else if (calculated_timestamp > UINT32_MAX) {
                    audio_timestamp = real_audio_time; // Fallback to real time
                    recorder_ctx.audio_quality.timestamp_corrections++;
                } else {
                    audio_timestamp = (uint32_t)calculated_timestamp;
                }
                
                // Smooth timestamp progression to avoid jumps
                if (recorder_ctx.last_audio_pts > 0) {
                    uint32_t expected_min_timestamp = recorder_ctx.last_audio_pts + 5;  // Minimum 5ms progression
                    uint32_t expected_max_timestamp = recorder_ctx.last_audio_pts + 50; // Maximum 50ms progression
                    
                    if (audio_timestamp < expected_min_timestamp) {
                        audio_timestamp = expected_min_timestamp;
                        recorder_ctx.audio_quality.timestamp_corrections++;
                    } else if (audio_timestamp > expected_max_timestamp && recorder_ctx.last_audio_pts < UINT32_MAX - 50) {
                        // Only apply max limit if we won't cause overflow
                        audio_timestamp = expected_max_timestamp;
                        recorder_ctx.audio_quality.timestamp_corrections++;
                    }
                }
                
                recorder_ctx.last_audio_pts = audio_timestamp;
                ESP_LOGD(TAG, "audio_timestamp: %ld, real: %ld, offset: %ld, PCM encoded bytes: %ld", 
                         audio_timestamp, real_audio_time, dynamic_audio_offset, out_frame.encoded_bytes);
                
                // Update audio quality metrics and dynamic offset
                if (recorder_ctx.audio_quality.total_frames % AUDIO_QUALITY_CHECK_INTERVAL == 0) {
                    recorder_ctx.audio_quality.success_rate = (float)(recorder_ctx.audio_quality.total_frames - recorder_ctx.audio_quality.failed_frames) / recorder_ctx.audio_quality.total_frames;
                    
                    // Adjust dynamic offset based on audio quality
                    if (recorder_ctx.audio_quality.success_rate < 0.9f) {
                        // Poor audio quality, adjust offset to improve sync
                        int32_t offset_adjustment = (int32_t)((1.0f - recorder_ctx.audio_quality.success_rate) * 100);
                        recorder_ctx.dynamic_audio_offset = (int32_t)(AUDIO_OFFSET_SMOOTH_FACTOR * offset_adjustment + 
                                                                      (1 - AUDIO_OFFSET_SMOOTH_FACTOR) * recorder_ctx.dynamic_audio_offset);
                        
                        // Clamp offset to reasonable bounds
                        if (recorder_ctx.dynamic_audio_offset > MAX_AUDIO_OFFSET) {
                            recorder_ctx.dynamic_audio_offset = MAX_AUDIO_OFFSET;
                        } else if (recorder_ctx.dynamic_audio_offset < MIN_AUDIO_OFFSET) {
                            recorder_ctx.dynamic_audio_offset = MIN_AUDIO_OFFSET;
                        }
                        
                        ESP_LOGI(TAG, "Audio quality: %.2f%%, adjusting offset to %ld ms", 
                                recorder_ctx.audio_quality.success_rate * 100, recorder_ctx.dynamic_audio_offset);
                    }
                }
                
                // Add audio packet to muxer
                xSemaphoreTake(recorder_ctx.recording_mutex, portMAX_DELAY);
                if (recorder_ctx.recording) {
                    esp_muxer_audio_packet_t audio_packet = {
                        .data = enc_data,
                        .len = out_frame.encoded_bytes,
                        .pts = audio_timestamp,
                    };
                    
                    ret = esp_muxer_add_audio_packet(recorder_ctx.muxer, recorder_ctx.audio_stream_idx, &audio_packet);
                    if (ret != ESP_MUXER_ERR_OK) {
                        ESP_LOGE(TAG, "Failed to add audio packet, error: %x", ret);
                        recorder_ctx.audio_quality.failed_frames++;
                    }
                }
                xSemaphoreGive(recorder_ctx.recording_mutex);
            }
        }
    }
    
    // Free resources
    free(ring_buffer);
    free(enc_data);
    free(sample_buffer);
    
    // Log final audio quality statistics
    ESP_LOGI(TAG, "Audio recording completed. Quality stats:");
    ESP_LOGI(TAG, "  Total frames: %ld", recorder_ctx.audio_quality.total_frames);
    ESP_LOGI(TAG, "  Failed frames: %ld", recorder_ctx.audio_quality.failed_frames);
    ESP_LOGI(TAG, "  Buffer overflows: %ld", recorder_ctx.audio_quality.overflow_count);
    ESP_LOGI(TAG, "  Timestamp corrections: %ld", recorder_ctx.audio_quality.timestamp_corrections);
    if (recorder_ctx.audio_quality.total_frames > 0) {
        float final_success_rate = (float)(recorder_ctx.audio_quality.total_frames - recorder_ctx.audio_quality.failed_frames) / recorder_ctx.audio_quality.total_frames * 100.0f;
        ESP_LOGI(TAG, "  Success rate: %.2f%%", final_success_rate);
        ESP_LOGI(TAG, "  Final audio offset: %ld ms", recorder_ctx.dynamic_audio_offset);
    }
    
    ESP_LOGI(TAG, "Audio capture and encode task ended");
    vTaskDelete(NULL);
}