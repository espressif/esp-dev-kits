#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "bsp/esp-bsp.h"
#include "lvgl.h"
#include "driver/jpeg_decode.h"
#include "driver/ppa.h"
#include "esp_private/esp_cache_private.h"

#include "ui_extra.h"

#include "app_storage.h"
#include "app_video.h"
#include "app_video_stream.h"
#include "app_video_utils.h"

#include "app_ai_detect.h"

static const char *TAG = "app_album";

#define ALIGN_UP(num, align)    (((num) + ((align) - 1)) & ~((align) - 1))

#define MAX_PATH_LEN 300
#define PIC_FOLDER_NAME "esp32_p4_pic_save"

typedef struct {
    char **filenames; 
    int count;
    int current_index;
    lv_obj_t *canvas;
    void *img_buffer;       // Buffer for JPEG file
    void *ppa_buffer;
    size_t buffer_size;
    void *canvas_buffer;    // Buffer for decoded RGB565 image
    int canvas_width;
    int canvas_height;
    jpeg_decoder_handle_t jpeg_handle;
    ppa_client_handle_t ppa_handle;
    int max_images;         // Dynamically adjusted maximum image count
} album_context_t;

static album_context_t album_ctx;
static size_t data_cache_line_size = 0;
static size_t tx_buffer_size = 0;

static uint64_t last_free_space_check_time = 0;
static uint64_t last_known_free_space = 0;
static uint32_t photos_taken_since_check = 0;
static const uint32_t CHECK_INTERVAL_MS = 10000; // 10 seconds
static const uint32_t ESTIMATED_PHOTO_SIZE = 500 * 1024; // 500KB
static const uint32_t MIN_FREE_SPACE = 5 * 1024 * 1024; // 5MB

static const uint32_t album_res[PHOTO_RESOLUTION_MAX] = {480, 640, 960};
static photo_resolution_t current_album_resolution = PHOTO_RESOLUTION_1080P; // default 1080P

static bool enable_coco_od = false;

static bool is_valid_image_file(const char *filename) {
    // check file extension
    const char *ext = strrchr(filename, '.');
    if (!ext || (strcasecmp(ext, ".jpg") != 0 && strcasecmp(ext, ".jpeg") != 0)) {
        return false;
    }
    
    // check if it is a temporary file or system file
    if (filename[0] == '.' || filename[0] == '#' || 
        filename[strlen(filename)-1] == '~' ||
        strncmp(filename, "____~", 5) == 0 ||
        strstr(filename, ".swp") != NULL) {
        return false;
    }
    
    return true;
}

// Comparison function for sorting filenames in descending order
static int compare_filenames_desc(const void *a, const void *b) {
    return strcmp(*(const char **)b, *(const char **)a);
}

// get sd card free space
static float app_album_get_sd_free_space(void)
{
    FATFS *fs;
    DWORD free_clusters;  
    DWORD sector_size;    
    
    // get file system information
    if (f_getfree(BSP_SD_MOUNT_POINT, &free_clusters, &fs) != FR_OK) {
        ESP_LOGE(TAG, "Failed to get SD card free space");
        return -1.0f;
    }
    
    // calculate total clusters and sector size
    sector_size = fs->ssize;
    
    // calculate free space (bytes)
    uint64_t free_bytes = (uint64_t)free_clusters * fs->csize * sector_size;
    
    // convert to MB
    float free_mb = (float)free_bytes / (1024 * 1024);
    
    ESP_LOGI(TAG, "SD card free space: %.2f MB", free_mb);
    
    return free_mb;
}

// get SD card total space (in MB)
static float app_album_get_sd_total_space(void)
{
    FATFS *fs;
    DWORD free_clusters;  
    DWORD total_clusters; 
    DWORD sector_size;    
    
    // get file system information
    if (f_getfree(BSP_SD_MOUNT_POINT, &free_clusters, &fs) != FR_OK) {
        ESP_LOGE(TAG, "Failed to get SD card information");
        return -1.0f;
    }
    
    // calculate total clusters and sector size
    total_clusters = fs->n_fatent - 2;
    sector_size = fs->ssize;
    
    // calculate total space (bytes)
    uint64_t total_bytes = (uint64_t)total_clusters * fs->csize * sector_size;
    
    // convert to MB
    float total_mb = (float)total_bytes / (1024 * 1024);
    
    ESP_LOGI(TAG, "SD card total space: %.2f MB", total_mb);
    
    return total_mb;
}

// Calculate and set the maximum number of images based on SD card capacity
static void app_album_calculate_max_images(void)
{
    // Local constants for this function only
    const int DEFAULT_MAX_IMAGES = 20000;    // Default maximum number of images
    const int AVG_IMAGE_SIZE_KB = 100;       // Assume average image size is 100KB
    const int SD_STORAGE_PERCENT = 50;       // Percentage of SD card capacity used for storing images
    
    float total_mb = app_album_get_sd_total_space();
    if (total_mb <= 0) {
        // If failed to get total capacity, use default value
        album_ctx.max_images = DEFAULT_MAX_IMAGES;
        ESP_LOGW(TAG, "Failed to get SD card total space, using default MAX_IMAGES: %d", DEFAULT_MAX_IMAGES);
        return;
    }
    
    // Calculate space available for storing images (MB)
    float storage_mb = total_mb * SD_STORAGE_PERCENT / 100.0f;
    
    // Calculate number of images that can be stored (assuming 100KB per image)
    int max_images = (int)(storage_mb * 1024 / AVG_IMAGE_SIZE_KB);
    
    // Limit maximum value to DEFAULT_MAX_IMAGES
    if (max_images > DEFAULT_MAX_IMAGES) {
        max_images = DEFAULT_MAX_IMAGES;
    }
    
    album_ctx.max_images = max_images;
    ESP_LOGI(TAG, "Calculated MAX_IMAGES: %d (based on %.2f MB total, %.2f MB allocated for storage)", 
             max_images, total_mb, storage_mb);
}

// Check if SD card has enough space to store a new image
bool app_album_can_store_new_image(void) 
{
    uint64_t current_time = esp_timer_get_time() / 1000; // 转换为毫秒
    
    // Check if SD card is mounted
    if (!ui_extra_get_sd_card_mounted()) {
        return false;
    }
    
    // If it's the first check or the time since the last check has passed, or a certain number of photos have been taken
    if (last_free_space_check_time == 0 || 
        (current_time - last_free_space_check_time) > CHECK_INTERVAL_MS ||
        photos_taken_since_check >= 5) {
        
        // Get SD card information
        FATFS *fs;
        DWORD free_clusters;
        FATFS **fsptr = &fs;
        
        f_getfree(BSP_SD_MOUNT_POINT, &free_clusters, fsptr);
        
        uint32_t sector_size = (*fsptr)->ssize;
        uint32_t cluster_size = (*fsptr)->csize;
        uint64_t free_space = (uint64_t)free_clusters * cluster_size * sector_size;
        
        // Update cache information
        last_free_space_check_time = current_time;
        last_known_free_space = free_space;
        photos_taken_since_check = 0;
        
        ESP_LOGI(TAG, "SD card free space: %llu bytes", free_space);
        
        // Check if there is enough space
        return (free_space > MIN_FREE_SPACE);
    } else {
        // Use estimated value: last known free space minus estimated photo size
        uint64_t estimated_free_space = last_known_free_space - 
                                       (photos_taken_since_check * ESTIMATED_PHOTO_SIZE);
        
        // If estimated space is enough, increase count and return true
        if (estimated_free_space > MIN_FREE_SPACE) {
            photos_taken_since_check++;
            return true;
        }
        
        // If estimated space is not enough, force a re-check
        last_free_space_check_time = 0;
        return app_album_can_store_new_image(); // Recursively call to perform actual check
    }
}


void app_album_photo_saved(void)
{
    photos_taken_since_check++;
}

/**
 * @brief Check if SD card has enough space to store a new MP4 video
 * 
 * This function checks if the SD card has sufficient space to store a new MP4 video file.
 * It checks both the remaining space percentage and the maximum video file count limit.
 * 
 * @param estimated_size_mb Estimated video size in MB, uses default estimate if 0
 * @return true if enough space is available, false otherwise
 */
bool app_video_stream_can_store_new_mp4(float estimated_size_mb)
{
    // Default video size estimate (MB/minute)
    const float DEFAULT_VIDEO_SIZE_PER_MIN = 15.0f;
    // Default video length (minutes)
    const float DEFAULT_VIDEO_LENGTH = 5.0f;
    // Video storage space threshold percentage
    const float MP4_LOW_SPACE_PERCENT = 15.0f;
    // Maximum number of video files
    const int MAX_MP4_FILES = 500;
    
    // Get video folder path
    char dir_path[64];
    snprintf(dir_path, sizeof(dir_path), "%s/esp32_p4_mp4_save", BSP_SD_MOUNT_POINT);
    
    // Check video file count
    DIR *dir = opendir(dir_path);
    if (dir != NULL) {
        int file_count = 0;
        struct dirent *entry;
        
        // Count existing MP4 files
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_REG && strstr(entry->d_name, ".mp4") != NULL) {
                file_count++;
            }
        }
        closedir(dir);
        
        // Check if maximum file count limit is reached
        if (file_count >= MAX_MP4_FILES) {
            ESP_LOGW(TAG, "Cannot store more videos: reached maximum file count (%d/%d)", 
                     file_count, MAX_MP4_FILES);
            return false;
        }
    }
    
    // Check remaining space
    float free_mb = app_album_get_sd_free_space();
    float total_mb = app_album_get_sd_total_space();
    
    if (free_mb <= 0 || total_mb <= 0) {
        ESP_LOGE(TAG, "Failed to get SD card space information");
        return false;
    }
    
    // Calculate remaining space percentage
    float free_percent = (free_mb / total_mb) * 100.0f;
    
    // If estimated size not specified, use default estimate
    if (estimated_size_mb <= 0) {
        estimated_size_mb = DEFAULT_VIDEO_SIZE_PER_MIN * DEFAULT_VIDEO_LENGTH;
    }
    
    // Check if remaining space is below threshold
    if (free_percent < MP4_LOW_SPACE_PERCENT) {
        ESP_LOGW(TAG, "Cannot store more videos: SD card low on space (%.2f%% free, %.2f MB)", 
                 free_percent, free_mb);
        return false;
    }
    
    // Check if there's enough space for the estimated video size
    if (free_mb < estimated_size_mb) {
        ESP_LOGW(TAG, "Cannot store more videos: Not enough space (%.2f MB available, %.2f MB needed)", 
                 free_mb, estimated_size_mb);
        return false;
    }
    
    return true;
}

// Scan images from SD card
static esp_err_t app_album_scan_images(void) {
    DIR *dir = opendir(BSP_SD_MOUNT_POINT"/"PIC_FOLDER_NAME);
    if (!dir) {
        ESP_LOGE(TAG, "Failed to open directory %s/%s", BSP_SD_MOUNT_POINT, PIC_FOLDER_NAME);
        return ESP_FAIL;
    }

    struct dirent *entry;
    char **filenames_temp = NULL;
    int count = 0;
    int capacity = 32;  

    // Free previous filenames array
    if (album_ctx.filenames) {
        for (int i = 0; i < album_ctx.count; i++) {
            if (album_ctx.filenames[i]) {
                free(album_ctx.filenames[i]);
            }
        }
        free(album_ctx.filenames);
        album_ctx.filenames = NULL;
    }
    album_ctx.count = 0;

    // Allocate initial capacity for filenames array
    filenames_temp = heap_caps_malloc(capacity * sizeof(char*), MALLOC_CAP_SPIRAM);
    if (!filenames_temp) {
        ESP_LOGE(TAG, "Failed to allocate memory for filenames array");
        closedir(dir);
        return ESP_FAIL;
    }

    // Scan all jpg files in the directory
    while ((entry = readdir(dir)) != NULL && count < album_ctx.max_images) {
        if (is_valid_image_file(entry->d_name)) {
            // If we need to expand the array capacity
            if (count >= capacity) {
                capacity *= 2;
                char **new_array = heap_caps_realloc(filenames_temp, capacity * sizeof(char*), MALLOC_CAP_SPIRAM);
                if (!new_array) {
                    ESP_LOGE(TAG, "Failed to reallocate memory for filenames array");
                    for (int i = 0; i < count; i++) {
                        free(filenames_temp[i]);
                    }
                    free(filenames_temp);
                    closedir(dir);
                    return ESP_FAIL;
                }
                filenames_temp = new_array;
            }
            
            // Allocate memory for filename
            filenames_temp[count] = heap_caps_malloc(MAX_PATH_LEN, MALLOC_CAP_SPIRAM);
            if (filenames_temp[count] == NULL) {
                ESP_LOGE(TAG, "Failed to allocate memory for filename");
                for (int i = 0; i < count; i++) {
                    free(filenames_temp[i]);
                }
                free(filenames_temp);
                closedir(dir);
                return ESP_FAIL;
            }
            
            snprintf(filenames_temp[count], MAX_PATH_LEN, 
                    "%s/%s/%s", BSP_SD_MOUNT_POINT, PIC_FOLDER_NAME, entry->d_name);
            count++;
        }
    }

    closedir(dir);
    
    if (count == 0) {
        ESP_LOGW(TAG, "No images found in %s/%s", BSP_SD_MOUNT_POINT, PIC_FOLDER_NAME);
        free(filenames_temp);
        return ESP_FAIL;
    }
    
    // Sort filenames in descending order (newest first, assuming sequential numbering)
    qsort(filenames_temp, count, sizeof(char *), compare_filenames_desc);
    
    // Save sorted filenames array
    album_ctx.filenames = filenames_temp;
    album_ctx.count = count;
    album_ctx.current_index = 0;
    
    ESP_LOGI(TAG, "Found %d images in %s/%s (sorted by filename in descending order)", 
             count, BSP_SD_MOUNT_POINT, PIC_FOLDER_NAME);
    
    return ESP_OK;
}

// Load current image into buffer and decode it
static esp_err_t app_album_load_current_image(void) {
    if (album_ctx.count == 0) {
        ESP_LOGE(TAG, "No images available");
        return ESP_FAIL;
    }
    
    // Free previous image buffer
    if (album_ctx.img_buffer) {
        free(album_ctx.img_buffer);
        album_ctx.img_buffer = NULL;
    }
    
    // Open file
    FILE *f = fopen(album_ctx.filenames[album_ctx.current_index], "rb");
    if (!f) {
        ESP_LOGE(TAG, "Failed to open file: %s", album_ctx.filenames[album_ctx.current_index]);
        return ESP_FAIL;
    }
    
    // Get file size
    fseek(f, 0, SEEK_END);
    size_t file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    // Allocate memory for JPEG data
    album_ctx.img_buffer = heap_caps_malloc(file_size, MALLOC_CAP_SPIRAM);
    if (!album_ctx.img_buffer) {
        ESP_LOGE(TAG, "Failed to allocate memory for image");
        fclose(f);
        return ESP_FAIL;
    }
    
    // Read file content
    size_t bytes_read = fread(album_ctx.img_buffer, 1, file_size, f);
    fclose(f);
    
    if (bytes_read != file_size) {
        ESP_LOGE(TAG, "Failed to read file: %s", album_ctx.filenames[album_ctx.current_index]);
        free(album_ctx.img_buffer);
        album_ctx.img_buffer = NULL;
        return ESP_FAIL;
    }
    
    album_ctx.buffer_size = file_size;
    ESP_LOGI(TAG, "Loaded image: %s (%u bytes)", album_ctx.filenames[album_ctx.current_index], file_size);
    
    uint32_t out_size = 0;
    jpeg_decode_picture_info_t header_info;

    jpeg_decode_cfg_t decode_cfg_rgb = {
        .output_format = JPEG_DECODE_OUT_FORMAT_RGB565,
        .rgb_order = JPEG_DEC_RGB_ELEMENT_ORDER_BGR,
    };
    esp_err_t ret = jpeg_decoder_get_info(album_ctx.img_buffer, album_ctx.buffer_size, &header_info);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to parse JPEG header: %s (error %d)", album_ctx.filenames[album_ctx.current_index], ret);
        free(album_ctx.img_buffer);
        album_ctx.img_buffer = NULL;
        
        // Delete corrupted image
        char current_file[MAX_PATH_LEN];
        strncpy(current_file, album_ctx.filenames[album_ctx.current_index], MAX_PATH_LEN - 1);
        ESP_LOGW(TAG, "Deleting corrupted image: %s", current_file);
        unlink(current_file);
        
        // Update album data
        free(album_ctx.filenames[album_ctx.current_index]);
        for (int i = album_ctx.current_index; i < album_ctx.count - 1; i++) {
            album_ctx.filenames[i] = album_ctx.filenames[i + 1];
        }
        album_ctx.count--;
        
        // Return failure if no images left
        if (album_ctx.count == 0) {
            return ESP_FAIL;
        }
        
        // Adjust index if out of range
        if (album_ctx.current_index >= album_ctx.count) {
            album_ctx.current_index = 0;
        }
        
        // Recursively try to load next image
        return app_album_load_current_image();
    }
    
    ESP_LOGD(TAG, "header parsed, width is %" PRId32 ", height is %" PRId32, header_info.width, header_info.height);
        
    ret = jpeg_decoder_process(album_ctx.jpeg_handle, &decode_cfg_rgb, album_ctx.img_buffer, album_ctx.buffer_size, album_ctx.ppa_buffer, tx_buffer_size, &out_size);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to decode JPEG: %s (error %d)", album_ctx.filenames[album_ctx.current_index], ret);
        free(album_ctx.img_buffer);
        album_ctx.img_buffer = NULL;
        
        // Delete corrupted image
        char current_file[MAX_PATH_LEN];
        strncpy(current_file, album_ctx.filenames[album_ctx.current_index], MAX_PATH_LEN - 1);
        ESP_LOGW(TAG, "Deleting corrupted image: %s", current_file);
        unlink(current_file);
        
        // Update album data
        free(album_ctx.filenames[album_ctx.current_index]);
        for (int i = album_ctx.current_index; i < album_ctx.count - 1; i++) {
            album_ctx.filenames[i] = album_ctx.filenames[i + 1];
        }
        album_ctx.count--;
        
        // Return failure if no images left
        if (album_ctx.count == 0) {
            return ESP_FAIL;
        }
        
        // Adjust index if out of range
        if (album_ctx.current_index >= album_ctx.count) {
            album_ctx.current_index = 0;
        }
        
        // Recursively try to load next image
        return app_album_load_current_image();
    }
    
    if(header_info.width == 1920 && header_info.height == 1080) {
        current_album_resolution = PHOTO_RESOLUTION_1080P;
    } else if(header_info.width == 1280 && header_info.height == 720) {
        current_album_resolution = PHOTO_RESOLUTION_720P;
    } else if(header_info.width == 640 && header_info.height == 480) {
        current_album_resolution = PHOTO_RESOLUTION_480P;
    } else {
        ESP_LOGE(TAG, "Not supported image resolution: %"PRId32"x%"PRId32", skip this image", 
                 header_info.width, header_info.height);
        free(album_ctx.img_buffer);
        album_ctx.img_buffer = NULL;
        return ESP_ERR_NOT_SUPPORTED;
    }

    ppa_srm_oper_config_t srm_config = {
        .in.buffer = album_ctx.ppa_buffer,
        .in.pic_w = header_info.width,
        .in.pic_h = header_info.height,
        .in.block_w = album_res[current_album_resolution],
        .in.block_h = album_res[current_album_resolution],
        .in.block_offset_x = (header_info.width - album_res[current_album_resolution]) / 2,
        .in.block_offset_y = (header_info.height - album_res[current_album_resolution]) / 2,
        .in.srm_cm = PPA_SRM_COLOR_MODE_RGB565,
        .out.buffer = album_ctx.canvas_buffer,
        .out.buffer_size = ALIGN_UP(BSP_LCD_H_RES * BSP_LCD_V_RES * 2, data_cache_line_size),
        .out.pic_w = BSP_LCD_H_RES,
        .out.pic_h = BSP_LCD_V_RES,
        .out.block_offset_x = 0,
        .out.block_offset_y = 0,
        .out.srm_cm = PPA_SRM_COLOR_MODE_RGB565,
        .rotation_angle = PPA_SRM_ROTATION_ANGLE_0,
        .scale_x = (float)BSP_LCD_H_RES / album_res[current_album_resolution],
        .scale_y = (float)BSP_LCD_V_RES / album_res[current_album_resolution],
        .rgb_swap = 0,
        .byte_swap = 0,
        .mode = PPA_TRANS_MODE_BLOCKING,
    };

    ESP_ERROR_CHECK(ppa_do_scale_rotate_mirror(album_ctx.ppa_handle, &srm_config));

    swap_rgb565_bytes(album_ctx.canvas_buffer, BSP_LCD_H_RES * BSP_LCD_V_RES);

    return ESP_OK;
}

// Display current image on canvas
static esp_err_t app_album_display_current_image(void) {
    if (!album_ctx.canvas_buffer || !album_ctx.canvas) {
        ESP_LOGE(TAG, "Canvas buffer or canvas object not initialized");
        return ESP_FAIL;
    }
    
    // Set canvas buffer with decoded image
    if (enable_coco_od) {
        app_coco_od_detect(album_ctx.canvas_buffer, album_ctx.canvas_width, album_ctx.canvas_height);
    }
    bsp_display_lock(0);
    lv_canvas_set_buffer(album_ctx.canvas, album_ctx.canvas_buffer, 
                         album_ctx.canvas_width, album_ctx.canvas_height, 
                         LV_IMG_CF_TRUE_COLOR);
    
    // Add photo index indicator
    if (album_ctx.count > 0) {
        // Create index text with larger buffer to avoid truncation warnings
        char index_text[32];  // Increased buffer size from 20 to 32
        snprintf(index_text, sizeof(index_text), "%d/%d", album_ctx.current_index + 1, album_ctx.count);
        
        // Setup text style
        lv_draw_label_dsc_t label_dsc;
        lv_draw_label_dsc_init(&label_dsc);
        label_dsc.color = lv_color_white();
        label_dsc.font = &ui_font_FontKoHoMiniBold24;
        
        // Create background rectangle
        lv_draw_rect_dsc_t rect_dsc;
        lv_draw_rect_dsc_init(&rect_dsc);
        rect_dsc.bg_color = lv_color_black();
        rect_dsc.bg_opa = LV_OPA_50;  // Semi-transparent background
        rect_dsc.radius = 5;
        
        // Calculate text width
        lv_point_t text_size;
        lv_txt_get_size(&text_size, index_text, label_dsc.font, 0, 0, LV_COORD_MAX, 0);
        
        // Calculate rectangle position and size (top-right corner with padding)
        int padding = 5;
        lv_area_t rect_area;
        rect_area.x1 = album_ctx.canvas_width - text_size.x - padding * 2 - 10;
        rect_area.y1 = 10;
        rect_area.x2 = album_ctx.canvas_width - 10;
        rect_area.y2 = text_size.y + padding * 2 + 10;
        
        // Draw background rectangle
        lv_canvas_draw_rect(album_ctx.canvas, rect_area.x1, rect_area.y1, 
                           rect_area.x2 - rect_area.x1, rect_area.y2 - rect_area.y1, 
                           &rect_dsc);
        
        // Draw text
        lv_point_t label_pos;
        label_pos.x = rect_area.x1 + padding;
        label_pos.y = rect_area.y1 + padding;
        lv_canvas_draw_text(album_ctx.canvas, label_pos.x, label_pos.y, 
                           text_size.x + 5, &label_dsc, index_text);
    }

    // Force refresh
    lv_obj_invalidate(album_ctx.canvas);
    bsp_display_unlock();
    
    return ESP_OK;
}

// Switch to next image
esp_err_t app_album_next_image(void) {
    if (album_ctx.count == 0) {
        ESP_LOGE(TAG, "No images available");
        return ESP_FAIL;
    }
    
    album_ctx.current_index = (album_ctx.current_index + 1) % album_ctx.count;
    ESP_LOGI(TAG, "Switching to next image: %d/%d", album_ctx.current_index + 1, album_ctx.count);
    
    if (app_album_load_current_image() != ESP_OK) {
        return ESP_FAIL;
    }
    
    return app_album_display_current_image();
}

// Switch to previous image
esp_err_t app_album_prev_image(void) {
    if (album_ctx.count == 0) {
        ESP_LOGE(TAG, "No images available");
        return ESP_FAIL;
    }
    
    album_ctx.current_index = (album_ctx.current_index + album_ctx.count - 1) % album_ctx.count;
    ESP_LOGI(TAG, "Switching to previous image: %d/%d", album_ctx.current_index + 1, album_ctx.count);
    
    if (app_album_load_current_image() != ESP_OK) {
        return ESP_FAIL;
    }
    
    return app_album_display_current_image();
}

static void app_album_show_no_data_message(void) {
    if (!ui_extra_get_sd_card_mounted()) {
        ESP_LOGW(TAG, "SD card not mounted, not showing any message");
        return;  // If SD card is not mounted, return immediately, do not display any message
    }    
    
    if (!album_ctx.canvas_buffer || !album_ctx.canvas) {
        ESP_LOGE(TAG, "Canvas buffer or canvas object not initialized");
        return;
    }
    
    // Set canvas 
    uint16_t *buf = (uint16_t *)album_ctx.canvas_buffer;
    for (int i = 0; i < album_ctx.canvas_width * album_ctx.canvas_height; i++) {
        buf[i] = 0xFFFF; 
    }
    
    bsp_display_lock(0);
    
    // Set canvas buffer
    lv_canvas_set_buffer(album_ctx.canvas, album_ctx.canvas_buffer, 
                         album_ctx.canvas_width, album_ctx.canvas_height, 
                         LV_IMG_CF_TRUE_COLOR);
    
    // Create "No data found" text
    lv_draw_label_dsc_t label_dsc;
    lv_draw_label_dsc_init(&label_dsc);
    label_dsc.color = lv_color_black(); 
    label_dsc.font = &ui_font_FontKoHoMiniBold24;
    
    // Calculate text position (move to the right)
    lv_point_t label_pos;
    label_pos.x = album_ctx.canvas_width / 2 - 90; 
    label_pos.y = album_ctx.canvas_height / 2 - 12; 
    
    // Draw text on canvas
    lv_canvas_draw_text(album_ctx.canvas, label_pos.x, label_pos.y, 200, &label_dsc, "No data found");
    
    // Force refresh
    lv_obj_invalidate(album_ctx.canvas);
    
    bsp_display_unlock();
    
    ESP_LOGI(TAG, "Displayed 'No data found' message");
}

// Delete current image and load next one
esp_err_t app_album_delete_current_image(void) {
    if (album_ctx.count == 0) {
        ESP_LOGE(TAG, "No images available");
        return ESP_FAIL;
    }
    
    // Get current filename
    char current_file[MAX_PATH_LEN];
    strncpy(current_file, album_ctx.filenames[album_ctx.current_index], MAX_PATH_LEN - 1);
    
    ESP_LOGI(TAG, "Deleting image: %s", current_file);
    
    // Delete the file
    if (unlink(current_file) != 0) {
        ESP_LOGE(TAG, "Failed to delete file: %s", current_file);
        return ESP_FAIL;
    }
    
    // free the memory of the filename
    free(album_ctx.filenames[album_ctx.current_index]);
    
    // Update filenames array by shifting elements
    for (int i = album_ctx.current_index; i < album_ctx.count - 1; i++) {
        album_ctx.filenames[i] = album_ctx.filenames[i + 1];
    }
    
    // Decrease count
    album_ctx.count--;
    
    ESP_LOGI(TAG, "Image deleted successfully, remaining images: %d", album_ctx.count);
    
    // If no images left
    if (album_ctx.count == 0) {
        // Clear canvas
        if (album_ctx.canvas_buffer) {
            memset(album_ctx.canvas_buffer, 0, album_ctx.canvas_width * album_ctx.canvas_height * 2);
            bsp_display_lock(0);
            lv_canvas_set_buffer(album_ctx.canvas, album_ctx.canvas_buffer, 
                                album_ctx.canvas_width, album_ctx.canvas_height, 
                                LV_IMG_CF_TRUE_COLOR);
            lv_obj_invalidate(album_ctx.canvas);
            bsp_display_unlock();
        }
        
        // Show "No data found" message
        app_album_show_no_data_message();

        // Free image buffer
        if (album_ctx.img_buffer) {
            free(album_ctx.img_buffer);
            album_ctx.img_buffer = NULL;
        }
        
        return ESP_OK;
    }
    
    // Adjust current index if needed
    if (album_ctx.current_index >= album_ctx.count) {
        album_ctx.current_index = album_ctx.count - 1;
    }
    
    // Load and display next image
    if (app_album_load_current_image() != ESP_OK) {
        // If failed to load current image, try next one
        if (album_ctx.count > 1) {
            album_ctx.current_index = (album_ctx.current_index + 1) % album_ctx.count;
            return app_album_load_current_image() && app_album_display_current_image();
        }
        return ESP_FAIL;
    }
    
    return app_album_display_current_image();
}

esp_err_t app_album_enable_coco_od(bool enable) {
    enable_coco_od = enable;
    return ESP_OK;
}

// Initialize album functionality
esp_err_t app_album_init(lv_obj_t *parent) {
    // Initialize context
    memset(&album_ctx, 0, sizeof(album_ctx));
    
    // Calculate and set maximum image count
    app_album_calculate_max_images();

    // Set canvas dimensions
    album_ctx.canvas_width = BSP_LCD_H_RES;
    album_ctx.canvas_height = BSP_LCD_V_RES;
    
    // Create canvas object
    album_ctx.canvas = parent;
    if (!album_ctx.canvas) {
        ESP_LOGE(TAG, "Failed to create LVGL canvas object");
        return ESP_FAIL;
    }

    ESP_ERROR_CHECK(esp_cache_get_alignment(MALLOC_CAP_SPIRAM, &data_cache_line_size));
    
    // Allocate canvas buffer (RGB565 format: 2 bytes per pixel)
    size_t canvas_buf_size = album_ctx.canvas_width * album_ctx.canvas_height * 2;
    album_ctx.canvas_buffer = heap_caps_aligned_calloc(data_cache_line_size, 1, canvas_buf_size, MALLOC_CAP_SPIRAM);
    if (!album_ctx.canvas_buffer) {
        ESP_LOGE(TAG, "Failed to allocate canvas buffer");
        return ESP_FAIL;
    }

    //PPA client config
    ppa_client_config_t ppa_srm_config = {
        .oper_type = PPA_OPERATION_SRM,
    };
    ESP_ERROR_CHECK(ppa_register_client(&ppa_srm_config, &album_ctx.ppa_handle));

    //jpeg decode memory alloc config
    jpeg_decode_memory_alloc_cfg_t tx_mem_cfg = {
        .buffer_direction = JPEG_DEC_ALLOC_OUTPUT_BUFFER,
    };

    album_ctx.ppa_buffer = jpeg_alloc_decoder_mem(1920 * 1088 * 3, &tx_mem_cfg, &tx_buffer_size);
    if (!album_ctx.ppa_buffer) {
        ESP_LOGE(TAG, "Failed to allocate PPA buffer");
        return ESP_FAIL;
    }

    jpeg_decode_engine_cfg_t decode_eng_cfg = {
        .timeout_ms = 40,
    };
    ESP_ERROR_CHECK(jpeg_new_decoder_engine(&decode_eng_cfg, &album_ctx.jpeg_handle));
    
    // Set initial canvas buffer (black screen)
    bsp_display_lock(0);
    lv_canvas_set_buffer(album_ctx.canvas, album_ctx.canvas_buffer, 
                         album_ctx.canvas_width, album_ctx.canvas_height, 
                         LV_IMG_CF_TRUE_COLOR);
    bsp_display_unlock();
    
    // Scan images from SD card
    esp_err_t ret = app_album_scan_images();
    if (ret != ESP_OK) {
        app_album_show_no_data_message();
        return ESP_OK; 
    }
    
    // Load first image
    ret = app_album_load_current_image();
    if (ret != ESP_OK) {
        app_album_show_no_data_message();
        return ESP_OK;
    }
    
    return ret;
}

esp_err_t app_album_refresh(void) {
    // Free previous image buffer
    if (album_ctx.img_buffer) {
        free(album_ctx.img_buffer);
        album_ctx.img_buffer = NULL;
    }
    
    // Rescan images from SD card
    esp_err_t ret = app_album_scan_images();
    if (ret != ESP_OK) {
        app_album_show_no_data_message();
        return ESP_OK; 
    }
    
    // Load first image
    ret = app_album_load_current_image();
    if (ret != ESP_OK) {
        // If failed to load image, show "No data found"
        app_album_show_no_data_message();
        return ESP_OK;
    }
    
    // Display first image
    ret = app_album_display_current_image();
    
    return ret;
}

// Clean up album resources
void app_album_deinit(void) {
    if (album_ctx.img_buffer) {
        free(album_ctx.img_buffer);
        album_ctx.img_buffer = NULL;
    }
    
    // free the memory of the filenames array
    if (album_ctx.filenames) {
        for (int i = 0; i < album_ctx.count; i++) {
            if (album_ctx.filenames[i]) {
                free(album_ctx.filenames[i]);
            }
        }
        free(album_ctx.filenames);
        album_ctx.filenames = NULL;
    }
    album_ctx.count = 0;

    if (album_ctx.canvas_buffer) {
        free(album_ctx.canvas_buffer);
        album_ctx.canvas_buffer = NULL;
    }
    
    if (album_ctx.jpeg_handle) {
        jpeg_del_decoder_engine(album_ctx.jpeg_handle);
        album_ctx.jpeg_handle = NULL;
    }
    
    // LVGL objects will be deleted with their parent
}