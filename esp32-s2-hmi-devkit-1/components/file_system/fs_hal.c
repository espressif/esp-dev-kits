/**
 * @file fs_hal.c
 * @brief File system hal.
 * @version 0.1
 * @date 2021-03-07
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

#include "fs_hal.h"

static const char *TAG = "fs_hal";

#define FS_LOG_TRACE(...)	ESP_LOGD(TAG, ##__VA_ARGS__)

#if CONFIG_FS_USE_SEM

static SemaphoreHandle_t fs_mutex = NULL;

esp_err_t fs_sem_take(void)
{
    return !xSemaphoreTake(fs_mutex, portMAX_DELAY);
}

esp_err_t fs_sem_give(void)
{
    return !xSemaphoreGive(fs_mutex);
}

#define FS_SEM_TAKE()     fs_sem_take()

#define FS_SEM_GIVE()     fs_sem_give()
#else

#define FS_SEM_TAKE
#define FS_SEM_GIVE

#endif

esp_err_t fs_hal_init(void)
{
#if CONFIG_FS_USE_SEM
    fs_mutex = xSemaphoreCreateMutex();
#endif

    return ESP_OK;
}

esp_err_t fs_open_file(fs_file_t *fp, const char *name)
{
    *fp = fopen(name, "rb");

    if (NULL == *fp) {
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t fs_close_file(fs_file_t *fp)
{
    return fclose(*fp);
}

int fs_file_seek(fs_file_t *fp, int offset, int whence)
{
    return fseek(*fp, offset, whence);
}

int fs_file_tell(fs_file_t *fp)
{
    return ftell(*fp);
}

esp_err_t fs_open_dir(fs_dir_t *dir, const char *path)
{
    *dir = opendir(path);

    if (NULL != *dir) {
        return ESP_OK;
    }

    return ESP_FAIL;
}

esp_err_t fs_close_dir(fs_dir_t *dir)
{
    return closedir(*dir);
}

esp_err_t fs_get_dir_file_count(const char *path, size_t *file_num)
{
    fs_dir_t file_dir = NULL;

    /* Open directory*/
    fs_open_dir(&file_dir, path);
    
    /* Check if directory opened successfully */
    if (NULL != file_dir) {
        FS_LOG_TRACE("Opened dir : [%s]", path);

        struct dirent *file = NULL;
        file = readdir(file_dir);
        while (file != NULL) {
            FS_LOG_TRACE(" | %s", file->d_name);
            file = readdir(file_dir);
        }
        *file_num = telldir(file_dir);
        if (closedir(file_dir)) {
            ESP_LOGE(TAG, "Failed close dir!");
            return ESP_FAIL;
        }
        FS_LOG_TRACE("Total file(s) : [%d]", *file_num);
    } else {
        FS_LOG_TRACE("Failed open dir : [%s]", path);
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t fs_get_dir_file_name(const char *path, size_t index, char *file_name)
{
    size_t file_num = 0;
    fs_dir_t file_dir = NULL;

    if (ESP_OK != fs_get_dir_file_count(path, &file_num)) {
        return ESP_FAIL;
    }

    char *file_path = heap_caps_malloc(64 * sizeof(char), MALLOC_CAP_SPIRAM);

    /*!< Open directory*/
    file_dir = opendir(path);

    /*!< Check if directory opened successfully */
    if (NULL != file_dir) {
        struct dirent *file = NULL;

        file = readdir(file_dir);

        /*!< Another way is using seekdir(), I think it has better performance */
        for (size_t i = 0; i < index; i++) {
            file = readdir(file_dir);
        }

        if (NULL == file) {
            ESP_LOGE(TAG, "File index out of bound!");
            return ESP_FAIL;
        } else {
            FS_LOG_TRACE("File found : [%s]", file->d_name);
            strcpy(file_name, file->d_name);
        }

        /*!< Close directory */
        if (closedir(file_dir)) {
            ESP_LOGE(TAG, "Failed close dir!");
            return ESP_FAIL;
        }

        free(file_path);
    } else {
        ESP_LOGE(TAG, "Failed open dir : [%s]", path);
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t fs_get_dir_file(fs_file_t *pfile, const char *path, size_t index)
{
    size_t file_num = 0;
    fs_dir_t file_dir = NULL;

    if (ESP_OK != fs_get_dir_file_count(path, &file_num)) {
        return ESP_FAIL;
    }

    char file_path[sizeof(struct dirent)];

    /* Open directory */
    file_dir = opendir(path);

    /* Check if directory opened successfully */
    if (NULL != file_dir) {
        struct dirent *file = NULL;

        file = readdir(file_dir);

        /* Another way is using seekdir(), I think it has better performance */
        for (size_t i = 0; i < index; i++) {
            file = readdir(file_dir);
        }

        if (NULL == file) {
            ESP_LOGE(TAG, "File index out of bound!");
            return ESP_FAIL;
        } else {
            FS_LOG_TRACE("File found : [%s]", file->d_name);
        }

        sprintf(file_path, "%s/%s", path, file->d_name);
        ESP_LOGI(TAG, "Open file : [%s]", file_path);

        fs_open_file(pfile, file_path);

        /* Close directory */
        if (closedir(file_dir)) {
            ESP_LOGE(TAG, "Failed close dir!");
            return ESP_FAIL;
        }
    } else {
        ESP_LOGE(TAG, "Failed open dir : [%s]", path);
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t fs_read_data(fs_file_t *file, size_t offset, uint8_t *dst, size_t bytes_to_read, size_t *bytes_read)
{
    *bytes_read = fread(dst, sizeof(uint8_t), bytes_to_read, *file);

    return ESP_OK;
}
