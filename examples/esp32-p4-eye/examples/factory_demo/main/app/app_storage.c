#include <stdio.h>
#include <dirent.h> 
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_timer.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/jpeg_decode.h"
#include "bsp/esp-bsp.h"
#include "nvs_flash.h"
#include "nvs.h"

#include "tinyusb.h"
#include "diskio.h"
#include "ff.h"

#include "ui_extra.h"
#include "app_album.h"
#include "app_video_stream.h"
#include "app_storage.h"

/* Constants and definitions */
#define PIC_FOLDER_NAME "esp32_p4_pic_save"
#define NVS_NAMESPACE "p4_eye_cfg"
#define NVS_KEY_OD "od"
#define NVS_KEY_RESOLUTION "resolution"
#define NVS_KEY_FLASH "flash"
#define NVS_KEY_INTERVAL_TIME "int_time"
#define NVS_KEY_MAGNIFICATION "magnify"
#define NVS_KEY_INTERVAL_ACTIVE "int_active"  // timed shooting flag
#define NVS_KEY_NEXT_WAKE_TIME "wake_time"    // next wake time
#define NVS_KEY_PHOTO_COUNT "photo_count"     // photo count
#define NVS_KEY_CONTRAST "contrast"
#define NVS_KEY_SATURATION "saturation"
#define NVS_KEY_BRIGHTNESS "brightness"
#define NVS_KEY_HUE "hue"
#define NVS_KEY_GYROSCOPE "gyroscope"
#define LOGICAL_DISK_NUM 1

/* Static variables */
static const char *TAG = "app_storage";
static uint32_t pic_num = 0;
static uint8_t s_pdrv = 0;
static int s_disk_block_size = 0;
static bool ejected[LOGICAL_DISK_NUM] = {true};

/* Forward declarations for static functions */
static void app_storage_find_max_pic_num(void);
static void app_storage_check_sd_card_task(void *pvParameters);
static bool _logical_disk_ejected(void);

/* NVS functions */

/**
 * @brief Save photo count to NVS
 */
esp_err_t app_storage_save_photo_count(uint16_t count)
{
    nvs_handle_t nvs_handle;
    esp_err_t err;
    
    // Open NVS namespace
    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
        return err;
    }
    
    // Save photo count
    err = nvs_set_u16(nvs_handle, NVS_KEY_PHOTO_COUNT, count);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error saving photo count: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }
    
    // Commit changes
    err = nvs_commit(nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error committing NVS: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }
    
    // Close NVS handle
    nvs_close(nvs_handle);
    ESP_LOGI(TAG, "Photo count saved: %d", count);
    
    return ESP_OK;
}

/**
 * @brief Get photo count from NVS
 */
esp_err_t app_storage_get_photo_count(uint16_t *count)
{
    nvs_handle_t nvs_handle;
    esp_err_t err;
    
    // Open NVS namespace
    err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
        return err;
    }
    
    // Get photo count
    err = nvs_get_u16(nvs_handle, NVS_KEY_PHOTO_COUNT, count);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        *count = 0;  // If not exist, set to 0
        err = ESP_OK;
    } else if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error getting photo count: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }
    
    // Close NVS handle
    nvs_close(nvs_handle);
    ESP_LOGI(TAG, "Photo count loaded: %d", *count);
    
    return ESP_OK;
}

/**
 * @brief Save interval shooting state to NVS
 */
esp_err_t app_storage_save_interval_state(bool is_active, uint32_t next_wake_time)
{
    nvs_handle_t nvs_handle;
    esp_err_t err;
    
    // Open NVS namespace
    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
        return err;
    }
    
    // Save timed shooting flag
    err = nvs_set_u8(nvs_handle, NVS_KEY_INTERVAL_ACTIVE, is_active ? 1 : 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error saving interval active flag: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }
    
    // Save next wake time
    err = nvs_set_u32(nvs_handle, NVS_KEY_NEXT_WAKE_TIME, next_wake_time);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error saving next wake time: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }
    
    // Commit changes
    err = nvs_commit(nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error committing NVS: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }
    
    // Close NVS handle
    nvs_close(nvs_handle);
    ESP_LOGI(TAG, "Interval state saved: active=%d, next_wake=%lu", is_active, next_wake_time);
    
    return ESP_OK;
}

/**
 * @brief Get interval shooting state from NVS
 */
esp_err_t app_storage_get_interval_state(bool *is_active, uint32_t *next_wake_time)
{
    nvs_handle_t nvs_handle;
    esp_err_t err;
    
    // Open NVS namespace
    err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
        return err;
    }
    
    // Get timed shooting flag
    uint8_t active_flag = 0;
    err = nvs_get_u8(nvs_handle, NVS_KEY_INTERVAL_ACTIVE, &active_flag);
    if (err == ESP_OK) {
        *is_active = (active_flag == 1);
    } else if (err == ESP_ERR_NVS_NOT_FOUND) {
        *is_active = false;
        err = ESP_OK;
    } else {
        ESP_LOGE(TAG, "Error getting interval active flag: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }
    
    // Get next wake time
    err = nvs_get_u32(nvs_handle, NVS_KEY_NEXT_WAKE_TIME, next_wake_time);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        *next_wake_time = 0;
        err = ESP_OK;
    } else if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error getting next wake time: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }
    
    // Close NVS handle
    nvs_close(nvs_handle);
    ESP_LOGI(TAG, "Interval state loaded: active=%d, next_wake=%lu", *is_active, *next_wake_time);
    
    return ESP_OK;
}

/**
 * @brief Save application settings to NVS
 */
esp_err_t app_storage_save_settings(settings_info_t *settings, uint16_t interval_time, uint16_t magnification)
{
    nvs_handle_t nvs_handle;
    esp_err_t err;
    
    // Open NVS namespace
    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
        return err;
    }
    
    // Save language settings
    uint8_t od_idx = 0;
    if (strcmp(settings->od, "On") == 0) {
        od_idx = 1;
    }
    err = nvs_set_u8(nvs_handle, NVS_KEY_OD, od_idx);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error saving od: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }
    
    // Save resolution settings
    uint8_t resolution_idx = 0;
    if (strcmp(settings->resolution, "1080P") == 0) {
        resolution_idx = 1;
    } else if (strcmp(settings->resolution, "480P") == 0) {
        resolution_idx = 2;
    }
    err = nvs_set_u8(nvs_handle, NVS_KEY_RESOLUTION, resolution_idx);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error saving resolution: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }
    
    // Save flash settings
    uint8_t flash_idx = 0;
    if (strcmp(settings->flash, "On") == 0) {
        flash_idx = 1;
    }
    err = nvs_set_u8(nvs_handle, NVS_KEY_FLASH, flash_idx);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error saving flash: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }
    
    // Save timed time
    err = nvs_set_u16(nvs_handle, NVS_KEY_INTERVAL_TIME, interval_time);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error saving interval time: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }
    
    // Save magnification
    err = nvs_set_u16(nvs_handle, NVS_KEY_MAGNIFICATION, magnification);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error saving magnification: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }
    
    // Save gyroscope settings
    uint8_t gyroscope_idx = 0;
    if (strcmp(settings->gyroscope, "On") == 0) {
        gyroscope_idx = 1;
    }
    err = nvs_set_u8(nvs_handle, NVS_KEY_GYROSCOPE, gyroscope_idx);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error saving gyroscope: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }
    
    // Commit changes
    err = nvs_commit(nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error committing NVS: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }
    
    // Close NVS handle
    nvs_close(nvs_handle);
    ESP_LOGI(TAG, "Settings saved to NVS successfully");
    
    return ESP_OK;
}

/**
 * @brief Load application settings from NVS
 */
esp_err_t app_storage_load_settings(settings_info_t *settings, uint16_t *interval_time, uint16_t *magnification)
{
    nvs_handle_t nvs_handle;
    esp_err_t err;
    
    // Open NVS namespace
    err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
        return err;
    }
    
    // Load gyroscope settings
    uint8_t gyroscope_idx = 0;
    err = nvs_get_u8(nvs_handle, NVS_KEY_GYROSCOPE, &gyroscope_idx);
    if (err == ESP_OK) {
        settings->gyroscope = (gyroscope_idx == 1) ? "On" : "Off";
    } else if (err != ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE(TAG, "Error loading gyroscope: %s", esp_err_to_name(err));
    }
    
    // Load od settings
    uint8_t od_idx = 0;
    err = nvs_get_u8(nvs_handle, NVS_KEY_OD, &od_idx);
    if (err == ESP_OK) {
        settings->od = (od_idx == 1) ? "On" : "Off";
    } else if (err != ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE(TAG, "Error loading od: %s", esp_err_to_name(err));
    }
    
    // Load resolution settings
    uint8_t resolution_idx = 0;
    err = nvs_get_u8(nvs_handle, NVS_KEY_RESOLUTION, &resolution_idx);
    if (err == ESP_OK) {
        if (resolution_idx == 0) {
            settings->resolution = "720P";
        } else if (resolution_idx == 1) {
            settings->resolution = "1080P";
        } else {
            settings->resolution = "480P";
        }
    } else if (err != ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE(TAG, "Error loading resolution: %s", esp_err_to_name(err));
    }
    
    // Load flash settings
    uint8_t flash_idx = 0;
    err = nvs_get_u8(nvs_handle, NVS_KEY_FLASH, &flash_idx);
    if (err == ESP_OK) {
        settings->flash = (flash_idx == 1) ? "On" : "Off";
    } else if (err != ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE(TAG, "Error loading flash: %s", esp_err_to_name(err));
    }
    
    // Load timed time
    err = nvs_get_u16(nvs_handle, NVS_KEY_INTERVAL_TIME, interval_time);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE(TAG, "Error loading interval time: %s", esp_err_to_name(err));
    }
    
    // Load magnification
    err = nvs_get_u16(nvs_handle, NVS_KEY_MAGNIFICATION, magnification);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE(TAG, "Error loading magnification: %s", esp_err_to_name(err));
    }
    
    // Close NVS handle
    nvs_close(nvs_handle);
    ESP_LOGI(TAG, "Settings loaded from NVS successfully");
    
    return ESP_OK;
}

/**
 * @brief save camera settings to nvs
 */
esp_err_t app_storage_save_camera_settings(uint32_t contrast, uint32_t saturation, 
                                          uint32_t brightness, uint32_t hue)
{
    nvs_handle_t nvs_handle;
    esp_err_t err;
    
    // open nvs namespace
    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
        return err;
    }
    
    // save contrast
    err = nvs_set_u32(nvs_handle, NVS_KEY_CONTRAST, contrast);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error saving contrast: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }
    
    // save saturation
    err = nvs_set_u32(nvs_handle, NVS_KEY_SATURATION, saturation);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error saving saturation: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }
    
    // save brightness
    err = nvs_set_u32(nvs_handle, NVS_KEY_BRIGHTNESS, brightness);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error saving brightness: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }
    
    // save hue
    err = nvs_set_u32(nvs_handle, NVS_KEY_HUE, hue);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error saving hue: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }
    
    // commit changes
    err = nvs_commit(nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error committing NVS: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }
    
    // close nvs handle
    nvs_close(nvs_handle);
    ESP_LOGI(TAG, "Camera settings saved to NVS successfully");
    
    return ESP_OK;
}

/**
 * @brief load camera settings from nvs
 */
esp_err_t app_storage_load_camera_settings(uint32_t *contrast, uint32_t *saturation, 
                                          uint32_t *brightness, uint32_t *hue)
{
    nvs_handle_t nvs_handle;
    esp_err_t err;
    
    // open nvs namespace
    err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
        return err;
    }
    
    // load contrast
    err = nvs_get_u32(nvs_handle, NVS_KEY_CONTRAST, contrast);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE(TAG, "Error loading contrast: %s", esp_err_to_name(err));
    }
    
    // load saturation
    err = nvs_get_u32(nvs_handle, NVS_KEY_SATURATION, saturation);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE(TAG, "Error loading saturation: %s", esp_err_to_name(err));
    }
    
    // load brightness
    err = nvs_get_u32(nvs_handle, NVS_KEY_BRIGHTNESS, brightness);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE(TAG, "Error loading brightness: %s", esp_err_to_name(err));
    }
    
    // load hue
    err = nvs_get_u32(nvs_handle, NVS_KEY_HUE, hue);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE(TAG, "Error loading hue: %s", esp_err_to_name(err));
    }
    
    // close nvs handle
    nvs_close(nvs_handle);
    ESP_LOGI(TAG, "Camera settings loaded from NVS successfully");
    
    return ESP_OK;
}

/**
 * @brief Save gyroscope setting to NVS
 */
esp_err_t app_storage_save_gyroscope_setting(bool enabled)
{
    nvs_handle_t nvs_handle;
    esp_err_t err;
    
    // Open NVS namespace
    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
        return err;
    }
    
    // Save gyroscope setting
    err = nvs_set_u8(nvs_handle, NVS_KEY_GYROSCOPE, enabled ? 1 : 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error saving gyroscope setting: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }
    
    // Commit changes
    err = nvs_commit(nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error committing NVS: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }
    
    // Close NVS handle
    nvs_close(nvs_handle);
    ESP_LOGI(TAG, "Gyroscope setting saved: %s", enabled ? "On" : "Off");
    
    return ESP_OK;
}

/**
 * @brief Load gyroscope setting from NVS
 */
esp_err_t app_storage_load_gyroscope_setting(bool *enabled)
{
    nvs_handle_t nvs_handle;
    esp_err_t err;
    
    // Open NVS namespace
    err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
        return err;
    }
    
    // Get gyroscope setting
    uint8_t gyroscope_flag = 0;
    err = nvs_get_u8(nvs_handle, NVS_KEY_GYROSCOPE, &gyroscope_flag);
    if (err == ESP_OK) {
        *enabled = (gyroscope_flag == 1);
    } else if (err == ESP_ERR_NVS_NOT_FOUND) {
        *enabled = false;  // Default to false if not found
        err = ESP_OK;
    } else {
        ESP_LOGE(TAG, "Error getting gyroscope setting: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }
    
    // Close NVS handle
    nvs_close(nvs_handle);
    ESP_LOGI(TAG, "Gyroscope setting loaded: %s", *enabled ? "On" : "Off");
    
    return ESP_OK;
}

/* SD Card and file operations */

/**
 * @brief Find the highest picture number in the directory
 */
static void app_storage_find_max_pic_num(void) 
{
    DIR *dir = opendir(BSP_SD_MOUNT_POINT"/"PIC_FOLDER_NAME);
    if (!dir) {
        ESP_LOGE(TAG, "Failed to open directory %s/%s", BSP_SD_MOUNT_POINT, PIC_FOLDER_NAME);
        return;
    }

    struct dirent *entry;
    uint32_t max_num = 0;  

    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, "pic_") && strstr(entry->d_name, ".jpg")) {
            unsigned int index;
            if (sscanf(entry->d_name, "pic_%u.jpg", &index) == 1) {
                if (index > max_num) {
                    max_num = index;  
                }
            }
        }
    }

    closedir(dir);
    pic_num = max_num + 1;  
    ESP_LOGI(TAG, "Next picture number will be: %lu", pic_num);
}

/**
 * @brief Save picture to SD card
 */
esp_err_t app_storage_save_picture(const uint8_t *data, size_t len) 
{
    if (data == NULL || len == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Verify JPEG integrity before saving
    jpeg_decode_picture_info_t header_info;
    esp_err_t ret = jpeg_decoder_get_info(data, len, &header_info);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to verify JPEG integrity: error %d", ret);
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "JPEG verified: %"PRId32"x%"PRId32, header_info.width, header_info.height);
    
    char filename[64];
    sprintf(filename, "%s/%s/pic_%04lu.jpg", BSP_SD_MOUNT_POINT, PIC_FOLDER_NAME, pic_num);
    
    FILE *file = fopen(filename, "wb");
    if (!file) {
        // If file opening failed, try to create the directory first
        char folder_path[64];
        sprintf(folder_path, "%s/%s", BSP_SD_MOUNT_POINT, PIC_FOLDER_NAME);
        
        // Check if directory exists
        DIR *dir = opendir(folder_path);
        if (!dir) {
            // Directory doesn't exist, create it
            ESP_LOGW(TAG, "Directory %s doesn't exist, creating it", folder_path);
            if (mkdir(folder_path, 0755) != 0) {
                ESP_LOGE(TAG, "Failed to create directory %s", folder_path);
                return ESP_FAIL;
            }
            ESP_LOGI(TAG, "Created directory: %s", folder_path);
        } else {
            closedir(dir);
        }
        
        // Try to open the file again after ensuring directory exists
        file = fopen(filename, "wb");
        if (!file) {
            ESP_LOGE(TAG, "Failed to open file for writing even after creating directory: %s", filename);
            return ESP_FAIL;
        }
    }
    
    size_t bytes_written = fwrite(data, 1, len, file);
    if (bytes_written != len) {
        ESP_LOGE(TAG, "Failed to write to file: %s (written: %u/%u)", 
                 filename, bytes_written, len);
        fclose(file);
        return ESP_FAIL;
    }
    
    fclose(file);
    ESP_LOGI(TAG, "Picture saved as %s (%u bytes)", filename, len);
    
    // Increment picture number for next image
    pic_num++;
    
    return ESP_OK;
}

static void app_storage_check_sd_card_task(void *pvParameters)
{
    bsp_sdcard_detect_init();
    bool is_sd_card_mounted = false;
    bool usb_msc_initialized = false;
    bool album_initialized = false;
    bool directory_checked = false;

    while(1) {
        bool current_is_sd_card_mounted = bsp_sdcard_is_present();
        if(current_is_sd_card_mounted != is_sd_card_mounted) {
            is_sd_card_mounted = current_is_sd_card_mounted;
            if(is_sd_card_mounted) {
                ESP_LOGI(TAG, "SD card mounted");
                bsp_sdcard_mount();

                bsp_display_lock(0);
                ui_extra_set_sd_card_mounted(true);
                bsp_display_unlock();

                if (!album_initialized) {
                    app_album_init(ui_ImageScreenAlbum);
                    album_initialized = true;
                    ESP_LOGI(TAG, "Album initialized");
                } else {
                    ESP_LOGI(TAG, "Album already initialized");
                }

                if (!directory_checked) {
                    // Create directory for saving pictures if it doesn't exist
                    char folder_path[64];
                    sprintf(folder_path, "%s/%s", BSP_SD_MOUNT_POINT, PIC_FOLDER_NAME);
                    
                    DIR *dir = opendir(folder_path);
                    if (dir) {
                        // Directory exists
                        closedir(dir);
                        ESP_LOGI(TAG, "Directory %s already exists", folder_path);
                        
                        // Find the highest picture number to continue incrementing
                        app_storage_find_max_pic_num();
                    } else {
                        // Directory doesn't exist, create it
                        if (mkdir(folder_path, 0755) != 0) {
                            ESP_LOGE(TAG, "Failed to create directory %s", folder_path);
                        } else {
                            ESP_LOGI(TAG, "Created directory: %s", folder_path);
                            pic_num = 1;  // Start with 1 for a new directory
                        }
                    }
                    directory_checked = true;
                    ESP_LOGI(TAG, "Directory check completed");
                } else {
                    ESP_LOGI(TAG, "Directory already checked");
                }

                if (!usb_msc_initialized) {
                    ESP_LOGI(TAG, "USB MSC initialization");
                    
                    const tinyusb_config_t tusb_cfg = {0};
                    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));
                    
                    usb_msc_initialized = true;
                    ESP_LOGI(TAG, "USB MSC initialization DONE");
                } else {
                    ESP_LOGI(TAG, "USB MSC already initialized");
                }
            } else {
                ESP_LOGI(TAG, "SD card unmounted");
                bsp_sdcard_unmount();

                bsp_display_lock(0);
                ui_extra_set_sd_card_mounted(false);
                bsp_display_unlock();
            }
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }

    vTaskDelete(NULL);
}

/**
 * @brief Initialize storage subsystem
 */
esp_err_t app_storage_init(void) {
    esp_err_t ret = ESP_OK;
    
    bool is_interval_active = false;
    uint32_t next_wake_time = 0;
    uint16_t interval_time = 0;
    settings_info_t settings;
    uint16_t magnification;

    if(!(esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER)) {
        ESP_LOGI(TAG, "other wake up");

        app_storage_get_interval_state(&is_interval_active, &next_wake_time);
        if(is_interval_active) {
            ui_extra_goto_page(UI_PAGE_INTERVAL_CAM);
            ui_extra_clear_page();
            ui_extra_popup_interval_timer_warning();
        }

        app_video_stream_stop_interval_photo();
        app_extra_set_saved_photo_count(0);
    } else {
        ESP_LOGI(TAG, "timer wake up");

        ret = app_storage_get_interval_state(&is_interval_active, &next_wake_time);
        if (ret == ESP_OK && is_interval_active) {
            // load settings
            ret = app_storage_load_settings(&settings, &interval_time, &magnification);
            if (ret == ESP_OK) {
                // use interval time
                ui_extra_goto_page(UI_PAGE_INTERVAL_CAM);
                ui_extra_clear_page();
                app_video_stream_start_interval_photo(interval_time);
                ESP_LOGI(TAG, "Device woke up for interval photography, interval time: %u minutes", interval_time);
            } else {
                ESP_LOGE(TAG, "Failed to load interval time settings");
            }
        }
    }

    xTaskCreate(app_storage_check_sd_card_task, "app_storage_check_sd_card_task", 1024 * 6, NULL, 5, NULL);

    return ESP_OK;
}

/* USB MSC helper functions */
static bool _logical_disk_ejected(void)
{
    bool all_ejected = true;

    for (uint8_t i = 0; i < LOGICAL_DISK_NUM; i++) {
        all_ejected &= ejected[i];
    }

    return all_ejected;
}

/* TinyUSB callbacks */

// Invoked when device is mounted
void tud_mount_cb(void)
{
    // Reset the ejection tracking every time we're plugged into USB. This allows for us to battery
    // power the device, eject, unplug and plug it back in to get the drive.
    for (uint8_t i = 0; i < LOGICAL_DISK_NUM; i++) {
        ejected[i] = false;
    }

    ESP_LOGI(TAG, "USB MSC mounted");
    bsp_display_lock(0);
    ui_extra_set_usb_disk_mounted(true);
    bsp_display_unlock();
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
    ESP_LOGW(TAG, "USB MSC unmounted");
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allows us to perform remote wakeup
// USB Specs: Within 7ms, device must draw an average current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
    ESP_LOGI(TAG, "USB MSC suspended");
    bsp_display_lock(0);
    ui_extra_set_usb_disk_mounted(false);
    bsp_display_unlock();
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
    ESP_LOGW(__func__, "");
}

// Callback invoked when WRITE10 command is completed (status received and accepted by host).
// used to flush any pending cache.
void tud_msc_write10_complete_cb(uint8_t lun)
{
    if (lun >= LOGICAL_DISK_NUM) {
        ESP_LOGE(__func__, "invalid lun number %u", lun);
        return;
    }

    // This write is complete, start the auto reload clock.
    ESP_LOGD(__func__, "");
}

// Invoked when received SCSI_CMD_INQUIRY
// Application fill vendor id, product id and revision with string up to 8, 16, 4 characters respectively
void tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4])
{
    ESP_LOGD(__func__, "");

    if (lun >= LOGICAL_DISK_NUM) {
        ESP_LOGE(__func__, "invalid lun number %u", lun);
        return;
    }

    const char vid[] = "ESP";
    const char pid[] = "Mass Storage";
    const char rev[] = "1.0";

    memcpy(vendor_id, vid, strlen(vid));
    memcpy(product_id, pid, strlen(pid));
    memcpy(product_rev, rev, strlen(rev));
}

// Invoked when received Test Unit Ready command.
// return true allowing host to read/write this LUN e.g SD card inserted
bool tud_msc_test_unit_ready_cb(uint8_t lun)
{
    ESP_LOGD(__func__, "");

    if (lun >= LOGICAL_DISK_NUM) {
        ESP_LOGE(__func__, "invalid lun number %u", lun);
        return false;
    }

    if (_logical_disk_ejected()) {
        // Set 0x3a for media not present.
        tud_msc_set_sense(lun, SCSI_SENSE_NOT_READY, 0x3A, 0x00);
        return false;
    }

    return true;
}

// Invoked when received SCSI_CMD_READ_CAPACITY_10 and SCSI_CMD_READ_FORMAT_CAPACITY to determine the disk size
// Application update block count and block size
void tud_msc_capacity_cb(uint8_t lun, uint32_t *block_count, uint16_t *block_size)
{
    ESP_LOGD(__func__, "");

    if (lun >= LOGICAL_DISK_NUM) {
        ESP_LOGE(__func__, "invalid lun number %u", lun);
        return;
    }

    disk_ioctl(s_pdrv, GET_SECTOR_COUNT, block_count);
    disk_ioctl(s_pdrv, GET_SECTOR_SIZE, block_size);
    s_disk_block_size = *block_size;
    ESP_LOGD(__func__, "GET_SECTOR_COUNT = %"PRIu32"，GET_SECTOR_SIZE = %d", *block_count, *block_size);
}

bool tud_msc_is_writable_cb(uint8_t lun)
{
    ESP_LOGD(__func__, "");

    if (lun >= LOGICAL_DISK_NUM) {
        ESP_LOGE(__func__, "invalid lun number %u", lun);
        return false;
    }

    return true;
}

// Invoked when received Start Stop Unit command
// - Start = 0 : stopped power mode, if load_eject = 1 : unload disk storage
// - Start = 1 : active mode, if load_eject = 1 : load disk storage
bool tud_msc_start_stop_cb(uint8_t lun, uint8_t power_condition, bool start, bool load_eject)
{
    (void) power_condition;

    if (lun >= LOGICAL_DISK_NUM) {
        ESP_LOGE(__func__, "invalid lun number %u", lun);
        return false;
    }

    if (load_eject) {
        if (!start) {
            // Eject but first flush.
            if (disk_ioctl(s_pdrv, CTRL_SYNC, NULL) != RES_OK) {
                return false;
            } else {
                ejected[lun] = true;
            }
        } else {
            // We can only load if it hasn't been ejected.
            return !ejected[lun];
        }
    } else {
        if (!start) {
            // Stop the unit but don't eject.
            if (disk_ioctl(s_pdrv, CTRL_SYNC, NULL) != RES_OK) {
                return false;
            }
        }

        // Always start the unit, even if ejected. Whether media is present is a separate check.
    }

    ESP_LOGI(TAG, "USB MSC suspended");
    bsp_display_lock(0);
    ui_extra_set_usb_disk_mounted(false);
    bsp_display_unlock();

    return true;
}

// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and return number of copied bytes.
int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize)
{
    ESP_LOGD(__func__, "");

    if (lun >= LOGICAL_DISK_NUM) {
        ESP_LOGE(__func__, "invalid lun number %u", lun);
        return 0;
    }

    const uint32_t block_count = bufsize / s_disk_block_size;
    disk_read(s_pdrv, buffer, lba, block_count);
    return block_count * s_disk_block_size;
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and return number of written bytes
int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize)
{
    ESP_LOGD(__func__, "");
    (void) offset;

    if (lun >= LOGICAL_DISK_NUM) {
        ESP_LOGE(__func__, "invalid lun number %u", lun);
        return 0;
    }

    const uint32_t block_count = bufsize / s_disk_block_size;
    disk_write(s_pdrv, buffer, lba, block_count);
    return block_count * s_disk_block_size;
}

// Callback invoked when received an SCSI command not in built-in list below
// - READ_CAPACITY10, READ_FORMAT_CAPACITY, INQUIRY, MODE_SENSE6, REQUEST_SENSE
// - READ10 and WRITE10 has their own callbacks
int32_t tud_msc_scsi_cb(uint8_t lun, uint8_t const scsi_cmd[16], void *buffer, uint16_t bufsize)
{
    // read10 & write10 has their own callback and MUST not be handled here
    ESP_LOGD(__func__, "");

    if (lun >= LOGICAL_DISK_NUM) {
        ESP_LOGE(__func__, "invalid lun number %u", lun);
        return 0;
    }

    void const *response = NULL;
    uint16_t resplen = 0;

    // most scsi handled is input
    bool in_xfer = true;

    switch (scsi_cmd[0]) {
    case SCSI_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL:
        // Host is about to read/write etc ... better not to disconnect disk
        resplen = 0;
        break;

    default:
        // Set Sense = Invalid Command Operation
        tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x20, 0x00);

        // negative means error -> tinyusb could stall and/or response with failed status
        resplen = -1;
        break;
    }

    // return resplen must not larger than bufsize
    if (resplen > bufsize) {
        resplen = bufsize;
    }

    if (response && (resplen > 0)) {
        if (in_xfer) {
            memcpy(buffer, response, resplen);
        } else {
            // SCSI output
        }
    }

    return resplen;
}