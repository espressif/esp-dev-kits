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

#define REPEAT_PLAY     (1)

static const char *TAG = "app_music";

static void app_music_task(void *pvParam);

esp_err_t app_music_start(void)
{
    if (pdPASS != xTaskCreate(
		(TaskFunction_t)        app_music_task,
		(const char * const)    "APP Music Task",
		(const uint32_t)        4096,
		(void * const)          NULL,
		(UBaseType_t)           configMAX_PRIORITIES - 2,
		(TaskHandle_t * const)  NULL)) {
        return ESP_ERR_NO_MEM;
    }

    return ESP_OK;
}

static void app_music_task(void *pvParam)
{
    (void) pvParam;

    size_t file_count = 0;
    size_t file_index = 0;
    fs_file_t audio_file = NULL;
    size_t offset = 0, bytes_read = 0;
    char *audio_file_dir = MOUNT_POINT "/Music";

    uint8_t *file_buffer = NULL;
    const size_t file_buffer_size = 64 * 1024;

    audio_hal_config_t audio_hal_config = {
        .audio_freq = 44100,
        .buffer_size = 8192,
        .malloc_caps = MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT
    };

    audio_hal_init(&audio_hal_config);

    fs_get_dir_file_count(audio_file_dir, &file_count);
    ESP_LOGI(TAG, "Find %d muisc files on SD card", file_count);

    /*!< Create a file buffer to save audio file */
    file_buffer = heap_caps_malloc(file_buffer_size, MALLOC_CAP_SPIRAM);
    if (NULL == file_buffer) {
        ESP_LOGE(TAG, "Failed create file buffer, check PSRAM avaliable space");
        vTaskDelete(NULL);
    }

    do {    /*!< Controls music repeat playing */
        do {    /*!< Controls audio file increase */
            fs_get_dir_file(&audio_file, audio_file_dir, file_index);
            offset = 0;
            ESP_LOGI(TAG, "Open file %d", file_index);
            do {    /*!< Controls data fetch from PSRAM to audio buffer */
                fs_read_data(&audio_file, offset, file_buffer, file_buffer_size, &bytes_read);
                for (size_t i = 0; i < bytes_read; i++) {
                    while (ESP_OK != audio_hal_add_byte(file_buffer[i])) {
                        vTaskDelay(pdMS_TO_TICKS(1000 * audio_hal_config.buffer_size / audio_hal_config.audio_freq / 8));
                    }
                }
                offset += bytes_read;
            } while (0 != bytes_read);  /*!< Wait for no data to read */
            fs_close_file(&audio_file);
        } while (++file_index < file_count);
        file_index = 0;
    } while (REPEAT_PLAY | (file_index = 0));

    // audio_hal_stop();
    free(file_buffer);

    vTaskDelete(NULL);    
}
