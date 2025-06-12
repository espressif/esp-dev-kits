#include <sys/stat.h> 
#include <dirent.h>
#include <stdio.h>
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_timer.h"
#include "esp_private/esp_cache_private.h"
#include "driver/ppa.h"
#include "driver/jpeg_encode.h"
#include "bsp/esp-bsp.h"

#include "ui_extra.h"
#include "app_video.h"
#include "app_storage.h"
#include "app_video_stream.h"
#include "app_album.h"
#include "app_video_utils.h"
#include "app_video_photo.h"

static const char *TAG = "app_video_photo";

/* Constants */
#define ALIGN_UP(num, align)    (((num) + ((align) - 1)) & ~((align) - 1))
#define DEBUG_MODE              0
#define CROP_PHOTO_WIDTH        1280
#define CROP_PHOTO_HEIGHT       960
#define JPEG_PHOTO_QUALITY      90            // JPEG quality setting

/* Static variables */
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

static int video_fd = -1;
static TaskHandle_t interval_sleep_task_handle = NULL;

/* Forward declarations */
static void enter_deep_sleep(uint16_t sleep_minutes);
static void interval_sleep_task(void *pvParameters);
static void interval_photo_complete_callback(void);

/* Public function implementations */

/**
 * @brief Initialize the photo module
 * 
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t app_video_photo_init(void)
{
    video_fd = app_video_stream_get_video_fd();

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

    return ESP_OK;
}

/**
 * @brief Set photo resolution
 * 
 * @param resolution Resolution enum value
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t app_video_stream_set_photo_resolution(photo_resolution_t resolution)
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
 * @brief Process and save a photo
 * 
 * @param camera_buf Camera buffer containing the image
 * @param width Image width
 * @param height Image height
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t take_and_save_photo(uint8_t *camera_buf, uint32_t width, uint32_t height)
{
    esp_err_t ret = ESP_OK;
    uint8_t *pic_buf = NULL;

    bsp_display_backlight_off();
    
    bsp_flashlight_set(app_video_stream_get_flash_light_state());

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

        // Use encapsulated interface for image scaling and cropping
        ret = app_image_process_scale_crop(
            pre_handle_buf, width, height,
            CROP_PHOTO_WIDTH, CROP_PHOTO_HEIGHT,
            photo_buf, photo_width, photo_height,
            ALIGN_UP(photo_width * photo_height * 2, data_cache_line_size),
            PPA_SRM_ROTATION_ANGLE_0
        );

        pic_buf = photo_buf;
    } else {
        if(magnification_factor > 1) {
            pic_buf = scaled_camera_buf;
        } else {
            pic_buf = camera_buf;
        }
    }

    // Perform JPEG encoding using the encapsulated function
    ret = app_image_encode_jpeg(
        pic_buf,
        photo_width,
        photo_height,
        JPEG_PHOTO_QUALITY,
        jpg_buf,
        rx_buffer_size,
        &jpg_size
    );
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "JPEG encoding failed: 0x%x", ret);
        goto cleanup;
    }

    // Save the picture
    ret = app_storage_save_picture(jpg_buf, jpg_size);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to save picture: 0x%x", ret);
    } else {
        ESP_LOGI(TAG, "Picture saved successfully");
    }

    if (ret == ESP_OK) {
        app_album_photo_saved();
    }

cleanup:
    // Note: photo_buf is pre-allocated and managed by init/deinit functions, don't free it here

    bsp_flashlight_set(false);
    bsp_display_backlight_on();

    // Handle interval photo
    if (app_video_stream_get_interval_photo_state() && ret == ESP_OK) {
        app_extra_set_saved_photo_count(app_extra_get_saved_photo_count() + 1);
        interval_photo_complete_callback();
    }

    return ret;
}

/**
 * @brief Deinitialize the photo module
 * 
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t app_video_photo_deinit(void)
{
    // Reset photo buffer pointer (managed by video_stream module)
    photo_buf = NULL;
    photo_buf_size = 0;

    ESP_LOGI(TAG, "Photo module deinitialized");
    return ESP_OK;
}

/* Private function implementations */

/**
 * @brief Enter deep sleep mode for interval photography
 * 
 * @param sleep_minutes Minutes to sleep before waking up
 */
static void enter_deep_sleep(uint16_t sleep_minutes)
{
    // Set wake-up time (microseconds)
#if DEBUG_MODE
    uint64_t sleep_time_us = sleep_minutes * 1000000ULL;
#else
    uint64_t sleep_time_us = sleep_minutes * 60 * 1000000ULL;
#endif
    
    ESP_LOGI(TAG, "Entering deep sleep for %d minutes", sleep_minutes);
    
    // Configure RTC wake-up timer
    esp_sleep_enable_timer_wakeup(sleep_time_us);
    
    // Initialize sleep IO
    bsp_enter_sleep_init();

    // Enter deep sleep
    esp_deep_sleep_start();
}

/**
 * @brief Interval photo sleep task, responsible for cleaning up resources and entering deep sleep
 * 
 * @param pvParameters Task parameters (unused)
 */
static void interval_sleep_task(void *pvParameters)
{
    uint16_t sleep_minutes = app_video_stream_get_current_interval_minutes();
    
    ESP_LOGI(TAG, "Preparing to enter deep sleep, cleaning up resources...");
    
    // Stop video stream task
    app_video_stream_task_stop(video_fd);
    app_video_wait_video_stop();
    app_video_close(video_fd);

    // Stop LVGL timer first to prevent triggering more LVGL events
    lvgl_port_stop();
    
    // Wait for a short time to ensure all LVGL tasks are completed
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // Acquire LVGL lock to ensure no other tasks are using LVGL
    if (lvgl_port_lock(1000)) {
        // Call deinit while holding the lock
        lvgl_port_deinit();
        // No need to unlock, because lvgl_port_deinit() has released the lock
    } else {
        ESP_LOGW(TAG, "Failed to acquire LVGL lock before deinit, proceeding anyway");
        lvgl_port_deinit();
    }
    bsp_display_del();
    
    // Enter deep sleep
    enter_deep_sleep(sleep_minutes);
    
    // The task should not reach here
    vTaskDelete(NULL);
}

/**
 * @brief Callback when interval photo is completed
 */
static void interval_photo_complete_callback(void)
{   
    // If interval photo is still active, create a new task to handle resource release before sleep
    if (app_video_stream_get_interval_photo_state()) {
        // Create a new task to handle resource release before sleep
        if (interval_sleep_task_handle != NULL) {
            vTaskDelete(interval_sleep_task_handle);
            interval_sleep_task_handle = NULL;
        }
        
        xTaskCreate(interval_sleep_task, "interval_sleep", 4096, NULL, 5, &interval_sleep_task_handle);
    }
}