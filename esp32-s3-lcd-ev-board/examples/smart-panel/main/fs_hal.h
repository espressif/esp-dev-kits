/**
 * @file fs_hal.h
 * @brief Header of file system HAL.
 * @version 0.1
 * @date 2021-02-26
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

#pragma once

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>

#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "esp_spiffs.h"
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include "sdmmc_cmd.h"
#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef FILE    *fs_file_t;		/* Type of file */
typedef DIR     *fs_dir_t;		/* Type of directory */

/**
 * @brief Initialize file system hal layer.
 *        Create semaphore if 'CONFIG_FS_USE_SEM' is defined
 * 
 * @return ESP_OK on success, otherwise see esp_err_t.
 */
esp_err_t fs_hal_init(void);

/**
 * @brief Open file with given name.
 * 
 * @param fp Pointer of file, NULL if failed to open.
 * @param name File name.
 * @return ESP_OK on success, otherwise see esp_err_t.
 */
esp_err_t fs_open_file(fs_file_t *fp, const char *name);

/**
 * @brief Close file.
 * 
 * @param fp 
 * @return ESP_OK on success, otherwise see esp_err_t.
 */
esp_err_t fs_close_file(fs_file_t *fp);

/**
 * @brief 
 * 
 * @param dir 
 * @param path 
 * @return esp_err_t 
 */
esp_err_t fs_open_dir(fs_dir_t *dir, const char *path);

/**
 * @brief 
 * 
 * @param fp 
 * @param offset 
 * @param whence 
 * @return int 
 */
int fs_file_seek(fs_file_t *fp, int offset, int whence);

/**
 * @brief 
 * 
 * @param fp 
 * @return long 
 */
int fs_file_tell(fs_file_t *fp);

/**
 * @brief 
 * 
 * @param dir 
 * @return esp_err_t 
 */
esp_err_t fs_close_dir(fs_dir_t *dir);

/**
 * @brief Get file name of give index in given path.
 * 
 * @param path Path to search.
 * @param index File index of path.
 * @param file_name File name of given index. NULL if not found.
 * @return ESP_OK on success, otherwise see esp_err_t.
 */
esp_err_t fs_get_dir_file_name(const char *path, size_t index, char *file_name);

/**
 * @brief Get file count in given path.
 * 
 * @param path Path to get count of file.
 * @param file_num Number of files in path.
 * @return ESP_OK on success, otherwise see esp_err_t. 
 */
esp_err_t fs_get_dir_file_count(const char *path, size_t *file_num);

/**
 * @brief Open file in given path with given index. 
 * 
 * @param pfile Pointer to file. NULL if failed to open.
 * @param path Directory name.
 * @param index Index of file.
 * @return ESP_OK on success, otherwise see esp_err_t. 
 */
esp_err_t fs_get_dir_file(fs_file_t *pfile, const char *path, size_t index);

/**
 * @brief Read data from file.
 * 
 * @param file Pointer to file.
 * @param offset Offset of place to read.
 * @param dst Buffer to save the data read from file.
 * @param bytes_to_read Size of data to read in byte(s).
 * @param bytes_read Real size of data read from file in byte(s).
 * @return ESP_OK on success, otherwise see esp_err_t. 
 */
esp_err_t fs_read_data(fs_file_t *file, size_t offset, uint8_t *dst, size_t bytes_to_read, size_t *bytes_read);

#ifdef __cplusplus
}
#endif
