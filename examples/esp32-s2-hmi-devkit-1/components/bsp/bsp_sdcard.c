/**
 * @file bsp_sdcard.c
 * @brief SD card init and deinit on dev board.
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

#include "bsp_sdcard.h"

static const char *TAG = "bsp_sdcard";

#define LOG_TRACE(...)	ESP_LOGI(TAG, ##__VA_ARGS__)

static const char mount_point[] = MOUNT_POINT;
static sdmmc_card_t* card;
static sdmmc_host_t host = SDSPI_HOST_DEFAULT();

#define SPI_DMA_CHAN    host.slot

esp_err_t bsp_sdcard_init(void)
{
    esp_err_t ret;
    /*!< *************************************************************************
     * Options for mounting the filesystem.                                      *
     * If format_if_mount_failed is set to true, SD card will be partitioned and *
     * formatted in case when mounting fails.                                    *
     *****************************************************************************/
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 2,
        .allocation_unit_size = 16 * 1024
    };
    LOG_TRACE("Initializing SD card");

    /*!< **************************************************************************
     * Use settings defined above to initialize SD card and mount FAT filesystem. *
     * Note: esp_vfs_fat_sdmmc/sdspi_mount is all-in-one convenience functions.   *
     * Please check its source code and implement error recovery when developing  *
     * production applications.                                                   *
     *****************************************************************************/
    LOG_TRACE("Using SPI peripheral");
    
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = GPIO_SPI_MOSI,
        .miso_io_num = GPIO_SPI_MISO,
        .sclk_io_num = GPIO_SPI_SCLK,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .max_transfer_sz = 4 * 1024 * sizeof(uint8_t),
    };

    ret = spi_bus_initialize(host.slot, &bus_cfg, SPI_DMA_CHAN);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize bus.");
        return ret;
    }

    /*!< ************************************************************************************
     *  This initializes the slot without card detect (CD) and write protect (WP) signals.  *
     *  Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals. *
     ****************************************************************************************/
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = GPIO_SPI_CS;
    slot_config.host_id = host.slot;

    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                "If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return ESP_FAIL;
    }

    /*!< Card has been initialized, print its properties */
    sdmmc_card_print_info(stdout, card);

    return ESP_OK;
}

esp_err_t bsp_sdcard_deinit(void)
{
    esp_err_t ret;

    /*!< Unmount partition and disable SDMMC or SPI peripheral */
    ret = esp_vfs_fat_sdcard_unmount(mount_point, card);
    if (ESP_OK != ret) {
        ESP_LOGE(TAG, "Unmount sdcard fail!");
        return ret;
    } else {
        LOG_TRACE("Card unmounted");
    }

    /*!< deinitialize the bus after all devices are removed */
    ret = spi_bus_free(host.slot);
    if (ESP_OK != ret) {
        ESP_LOGE(TAG, "Free SPI bus fail!");
        return ret;
    }

    return ESP_OK;
}
