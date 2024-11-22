/**
 * @file app_music.c
 * @brief Music APP task
 * @version 0.1
 * @date 2021-03-04
 * 
 * @copyright Copyright 2021 Espressif Systems (Shanghai) Co. Ltd.
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *               http://www.apache.org/licenses/LICENSE-2.0
 * 
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#include "app_music.h"
#include "ui_main.h"

static const char *TAG = "app_music";

#define LOG_TRACE(...)  ESP_LOGI(TAG, ##__VA_ARGS__)
#define CHECK(a, str, ret_val) do { \
        if (!(a)) { \
            ESP_LOGE(TAG,"%s(%d): %s", __FUNCTION__, __LINE__, str); \
            return (ret_val); \
        } \
    } while(0)

static bool repeat_play = true;
static uint8_t *file_buffer = NULL;
static volatile size_t file_index = 0;
static const size_t file_buffer_size = 64 * 1024;

static QueueHandle_t music_event_queue = NULL;
static audio_file_info_list_t *s_audio_file_info_list_head = NULL;
static int32_t queue_buffer[8];
static StaticQueue_t s_t_audio_queue;

/**
 * @brief Task of music app
 * 
 * @param pvParam Not used.
 */
static void app_music_task(void *pvParam)
{
    /* Prevent warning */
    (void) pvParam;

    size_t file_count = 0;
    fs_file_t audio_file = NULL;
    size_t offset = 0, bytes_read = 0;
    char *audio_file_dir = "/spiffs" "/Music";
    music_event_t music_event = MUSIC_EVENT_NONE;
    audio_file_info_list_t *audio_file_info_list = NULL;
    
    /* Get audio files' info */
    app_music_update_music_info(audio_file_dir, &file_count);
    app_music_get_audio_info_link_head(&audio_file_info_list);
    LOG_TRACE("Find %d muisc files on storage", file_count);
    if (NULL != audio_file_info_list) {
        do {
            if (NULL != audio_file_info_list->file_name) {
                LOG_TRACE("File name : [%s], File size : [%zu]", audio_file_info_list->file_name, audio_file_info_list->file_size);
            }
            audio_file_info_list = audio_file_info_list->next;
        } while (NULL != audio_file_info_list);
    } else {
        ESP_LOGE(TAG, "No audio file list get");
        vTaskDelete(NULL);
    }

    /* Init audio hal component */
    audio_hal_config_t audio_hal_config = {
        .audio_freq = SAMPLE_RATE,
        .buffer_size = 8 * 1024,
        .malloc_caps = MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT
    };
    audio_hal_init(&audio_hal_config);

    /* Create a file buffer in PSRAM to save audio file data */
    file_buffer = heap_caps_malloc(file_buffer_size, MALLOC_CAP_SPIRAM);
    if (NULL == file_buffer) {
        ESP_LOGE(TAG, "Failed create file buffer, check PSRAM avaliable space");
        vTaskDelete(NULL);
    }

    /* Create music event queue */
    music_event_queue = xQueueCreateStatic(8, sizeof(uint32_t), (uint8_t *) &queue_buffer[0],&s_t_audio_queue);
    if (NULL == music_event_queue) {
        ESP_LOGE(TAG, "No mem for music event queue");
        free(file_buffer);
        audio_hal_deinit();
        vTaskDelete(NULL);
    }

    vTaskPrioritySet(NULL, configMAX_PRIORITIES - 2);

    do {            /* Controls music repeat playing */
        do {        /* Controls audio file increase */
            fs_get_dir_file(&audio_file, audio_file_dir, file_index);
            offset = 0;
            LOG_TRACE("Open file %d", file_index);
            do {    /* Controls data fetch from PSRAM to audio buffer */
                if (pdPASS == xQueueReceive(music_event_queue, &music_event, 0)) {
                    if (MUSIC_EVENT_NEXT == music_event) {
                        break;
                    } else if (MUSIC_EVENT_PREV == music_event) {
                        if (3 > file_count) {
                        } else {
                            if (0 == file_index) {
                                file_index = file_count - 2;
                            } else if (1 == file_count) {
                                file_index = file_count - 1;
                            } else {
                                file_index -= 2;
                            }
                        }
                        break;
                    }
                    music_event = MUSIC_EVENT_NONE;
                }
                fs_read_data(&audio_file, offset, file_buffer, file_buffer_size, &bytes_read);
                for (size_t i = 0; i < bytes_read; i++) {
                    while (ESP_OK != audio_hal_add_byte(file_buffer[i])) {
                        vTaskDelay(pdMS_TO_TICKS(1000 * audio_hal_config.buffer_size / audio_hal_config.audio_freq / 8));
                    }
                }
                offset += bytes_read;
            } while (0 != bytes_read);  /* Wait for no data to read */
            fs_close_file(&audio_file);
        } while (++file_index < file_count);

        /* Pause if not repeat playing */
        if (false == repeat_play) {
            audio_hal_pause();
        }

        /* Reset file index */
        file_index = 0;
    } while (1);

    free(file_buffer);

    vTaskDelete(NULL);    
}

esp_err_t app_music_start(void)
{
    CHECK(pdPASS == xTaskCreate(
        (TaskFunction_t)        app_music_task,
        (const char * const)    "APP Music Task",
        (const uint32_t)        4096,
        (void * const)          NULL,
        (UBaseType_t)           2,
        (TaskHandle_t * const)  NULL),
        "No mem for music task",
        ESP_ERR_NO_MEM);

    return ESP_OK;
}

/**
 * @brief Scan audio files with given path.
 * 
 * @param path The full path want to scan. Like "/sdcard/music" or "/spiffs/audio".
 * @param p_file_count pointer to count of files
 * @return esp_err_t Scan result
 */
esp_err_t app_music_update_music_info(const char *path, size_t *p_file_count)
{
    char file_name[sizeof(struct dirent)];        /* File names with path. Casue d_name is char[256], must be greater than 256 to prevent warning. */
    size_t file_size = 0;       /* Size of audio file in bytes. Can */
    size_t file_count = 0;
    fs_file_t fp = NULL;        /* Data type of file stream objects */
    fs_dir_t file_dir = NULL;   /* Data type of directory stream objects */

    /* lists of audio file info, struct defined in `audio_file_info_list_t` */
    audio_file_info_list_t *audio_file_info = NULL;
    audio_file_info_list_t *audio_file_info_prev = NULL;

    /* Check if already scaned */
    CHECK(NULL == s_audio_file_info_list_head, "Already scaned.", ESP_FAIL);

    /* Check NULL pointer */
    CHECK(p_file_count, "Invalid param of p_file_count", ESP_ERR_INVALID_ARG);

    /* Gave a default value of 0 */
    *p_file_count = 0;

    /* Open directory with given path. And check result */
    CHECK(ESP_OK == fs_open_dir(&file_dir, path), "Failed open dir", ESP_FAIL);
    CHECK(file_dir, "Failed open dir", ESP_FAIL);

    /* Scan the first file to see if any file exists */
    struct dirent *file = readdir(file_dir);
    CHECK(file, "File not found", ESP_FAIL);

    /* Scan files until NULL read */
    do {
        /* Get file name with path, fopen needs full file name as param */
        sprintf(file_name, "%s/%s", path, (char *)file->d_name);
        
        /* Try open file */
        fs_open_file(&fp, file_name);

        /* Check if file open status */
        if (NULL != fp) {
            /* Get size of file, used for calculate audio length by audio_file_size / audio_sample_rate */
            fs_file_seek(&fp, 0, SEEK_END);
            file_size = fs_file_tell(&fp);

            /* File must be closed after used. Max opened file is limited in struct esp_vfs_fat_sdmmc_mount_config_t.max_files */
            fs_close_file(&fp);

            /* Allocte mem for audio info. Try allocate in PSRAM */
            audio_file_info = heap_caps_malloc(sizeof(audio_file_info_list_t), MALLOC_CAP_SPIRAM);

            /* Check memory allocation result */
            if (NULL != audio_file_info) {
                /* Give head of list to p_audio_info_list_head */
                if (NULL == s_audio_file_info_list_head) {
                    s_audio_file_info_list_head = audio_file_info;
                }

                /* Connect previous link to current link */
                if (NULL != audio_file_info_prev) {
                    audio_file_info_prev->next = audio_file_info;
                }

                /* Save file info to `audio_file_info` */
                audio_file_info->prev = audio_file_info_prev;
                audio_file_info->next = NULL;
                audio_file_info->file_size = file_size;

                /* Audio file name in d_name will be free after file closed. */
                audio_file_info->file_name = heap_caps_malloc(strlen(file->d_name), MALLOC_CAP_SPIRAM);
                if (NULL != audio_file_info->file_name) {
                    strcpy(audio_file_info->file_name, file->d_name);
                    LOG_TRACE("File[%d] : %s, Size : %zu, time : %.3f", file_count, file->d_name, file_size, file_size / 44100.0f);
                } else {
                    /* Close file before function returned to prevent running out of file descriptor */
                    fs_close_file(&fp);
                    /* Close directory before function returned */
                    fs_close_dir(&file_dir);

                    /* Run out of mem, skip scanning process */
                    audio_file_info->next = NULL;
                    audio_file_info->prev = NULL;
                    audio_file_info->file_name = NULL;
                    audio_file_info->file_size = 0;
                    audio_file_info_prev->next = NULL;

                    free(audio_file_info);

                    ESP_LOGE(TAG, "No mem for file name");
                    
                    return ESP_ERR_NO_MEM;
                }

                audio_file_info_prev = audio_file_info;
            } else {
                /* Close file before function returned to prevent running out of file descriptor */
                ESP_LOGE(TAG, "No mem for audio_file_info");

                /* Close file before function returned to prevent running out of file descriptor */
                fs_close_file(&fp);
                
                /* Close directory before function returned */
                fs_close_dir(&file_dir);

                return ESP_ERR_NO_MEM;
            }
        } else {
            /* Error happed in file system. */
            ESP_LOGE(TAG, "Failed open : %s. Check file system.", file_name);

            /* Close directory before function returned */
            fs_close_dir(&file_dir);

            return ESP_FAIL;
        }

        /* Scan next file */
        file_count++;
        file = readdir(file_dir);
    } while (file != NULL);     /* NULL if previous file is the last file in given directory */

    *p_file_count = file_count;

    /* Close directory before function returned */
    fs_close_dir(&file_dir);

    /* Scan done and no error happened if run to here */
    return ESP_OK;
}

esp_err_t app_music_get_audio_info_link_head(audio_file_info_list_t **head)
{
    CHECK(head, "Invalid param of head", ESP_ERR_INVALID_ARG);
    *head = s_audio_file_info_list_head;
    return ESP_OK;
}

esp_err_t app_music_send_event(music_event_t event)
{
    if ((MUSIC_EVENT_PREV == event) || (MUSIC_EVENT_NEXT == event)) {
        audio_hal_pause();
        audio_hal_clear_data();
        if (NULL != file_buffer) {
            memset(file_buffer, 0, file_buffer_size);
        }
    }

    CHECK(pdPASS == xQueueSend(music_event_queue, &event, 0), "Failed send music event", ESP_FAIL);

    audio_hal_start();
    ui_music_update_btn();

    return ESP_OK;
}
