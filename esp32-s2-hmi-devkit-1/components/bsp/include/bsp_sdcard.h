/**
 * @file bsp_sdcard.h
 * @brief Header of bsp_sdcard.
 * @version 0.1
 * @date 2021-01-13
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

#include <stdio.h>

#include "driver/sdspi_host.h"
#include "driver/spi_common.h"

#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "sdkconfig.h"
#include "sdmmc_cmd.h"

#include "bsp_board.h"

#define MOUNT_POINT "/sdcard"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t bsp_sdcard_init(void);

esp_err_t bsp_sdcard_deinit(void);

#ifdef __cplusplus
}
#endif
