// Copyright 2020-2021 Espressif Systems (Shanghai) Co. Ltd.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "esp_err.h"
#include "esp_log.h"
#include "ffconf.h"
#include "ff.h"
#include "diskio.h"
#include "tusb_msc.h"
#include "esp_attr.h"

#define LOGICAL_DISK_NUM 1

static uint8_t s_pdrv[LOGICAL_DISK_NUM] = {0};
static tusb_msc_callback_t cb_mount[LOGICAL_DISK_NUM] = {NULL};
static tusb_msc_callback_t cb_unmount[LOGICAL_DISK_NUM] = {NULL};
static int s_disk_block_size[LOGICAL_DISK_NUM] = {0};
static bool s_ejected[LOGICAL_DISK_NUM] = {true};

esp_err_t tusb_msc_init(const tinyusb_config_msc_t *cfg)
{
    if (cfg == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    cb_mount[0] = cfg->cb_mount;
    cb_unmount[0] = cfg->cb_unmount;
    s_pdrv[0] = cfg->pdrv;
    //s_pdrv[1] = 1;
    return ESP_OK;
}

//--------------------------------------------------------------------+
// tinyusb callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
    // Reset the ejection tracking every time we're plugged into USB. This allows for us to battery
    // power the device, eject, unplug and plug it back in to get the drive.
    for (uint8_t i = 0; i < LOGICAL_DISK_NUM; i++) {
        s_ejected[i] = false;
    }

    if (cb_mount[0])
    {
        cb_mount[0](0, NULL);
    }
    
    ESP_LOGI(__func__, "");
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
    if (cb_unmount[0])
    {
        cb_unmount[0](0, NULL);
    }

    ESP_LOGW(__func__, "");
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allows us to perform remote wakeup
// USB Specs: Within 7ms, device must draw an average current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
    if (cb_unmount[0])
    {
        cb_unmount[0](0, NULL);
    }
    ESP_LOGW(__func__, "");
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
    ESP_LOGW(__func__, "");
}

// Invoked to determine max LUN
uint8_t tud_msc_get_maxlun_cb(void)
{
  return LOGICAL_DISK_NUM; // dual LUN
}

// Callback invoked when WRITE10 command is completed (status received and accepted by host).
// used to flush any pending cache.
void tud_msc_write10_complete_cb(uint8_t lun)
{
    if (unlikely(lun >= LOGICAL_DISK_NUM)) {
        ESP_LOGE(__func__, "invalid lun number %u", lun);
        return;
    }

    // This write is complete, start the autoreload clock.
    ESP_LOGD(__func__, "");
}

static bool _logical_disk_ejected(void)
{
    bool all_ejected = true;

    for (uint8_t i = 0; i < LOGICAL_DISK_NUM; i++) {
        all_ejected &= s_ejected[i];
    }

    return all_ejected;
}

// Invoked when received SCSI_CMD_INQUIRY
// Application fill vendor id, product id and revision with string up to 8, 16, 4 characters respectively
void tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4])
{
    ESP_LOGD(__func__, "");

    if (unlikely(lun >= LOGICAL_DISK_NUM)) {
        ESP_LOGE(__func__, "invalid lun number %u", lun);
        return;
    }

    const char vid[] = "Espressif";
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

    if (unlikely(lun >= LOGICAL_DISK_NUM)) {
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

    if (unlikely(lun >= LOGICAL_DISK_NUM)) {
        ESP_LOGE(__func__, "invalid lun number %u", lun);
        return;
    }

    disk_ioctl(s_pdrv[lun], GET_SECTOR_COUNT, block_count);
    disk_ioctl(s_pdrv[lun], GET_SECTOR_SIZE, block_size);
    s_disk_block_size[lun] = *block_size;
    ESP_LOGD(__func__, "lun = %u GET_SECTOR_COUNT = %d，GET_SECTOR_SIZE = %d",lun, *block_count, *block_size);
}

bool tud_msc_is_writable_cb(uint8_t lun)
{
    ESP_LOGD(__func__, "");

    if (unlikely(lun >= LOGICAL_DISK_NUM)) {
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
    ESP_LOGI(__func__, "");
    (void) power_condition;

    if (unlikely(lun >= LOGICAL_DISK_NUM)) {
        ESP_LOGE(__func__, "invalid lun number %u", lun);
        return false;
    }

    if (load_eject) {
        if (!start) {
            // Eject but first flush.
            if (disk_ioctl(s_pdrv[lun], CTRL_SYNC, NULL) != RES_OK) {
                return false;
            } else {
                s_ejected[lun] = true;
            }
        } else {
            // We can only load if it hasn't been ejected.
            return !s_ejected[lun];
        }
    } else {
        if (!start) {
            // Stop the unit but don't eject.
            if (disk_ioctl(s_pdrv[lun], CTRL_SYNC, NULL) != RES_OK) {
                return false;
            }
        }

        // Always start the unit, even if ejected. Whether media is present is a separate check.
    }

    return true;
}

// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and return number of copied bytes.
IRAM_ATTR int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize)
{
    ESP_LOGD(__func__, "");

    if (unlikely(lun >= LOGICAL_DISK_NUM)) {
        ESP_LOGE(__func__, "invalid lun number %u", lun);
        return 0;
    }

    const uint32_t block_count = bufsize / s_disk_block_size[lun];
    disk_read(s_pdrv[lun], buffer, lba, block_count);
    return block_count * s_disk_block_size[lun];
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and return number of written bytes
IRAM_ATTR int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize)
{
    ESP_LOGD(__func__, "");
    (void) offset;

    if (unlikely(lun >= LOGICAL_DISK_NUM)) {
        ESP_LOGE(__func__, "invalid lun number %u", lun);
        return 0;
    }

    const uint32_t block_count = bufsize / s_disk_block_size[lun];
    disk_write(s_pdrv[lun], buffer, lba, block_count);
    return block_count * s_disk_block_size[lun];
}

// Callback invoked when received an SCSI command not in built-in list below
// - READ_CAPACITY10, READ_FORMAT_CAPACITY, INQUIRY, MODE_SENSE6, REQUEST_SENSE
// - READ10 and WRITE10 has their own callbacks
int32_t tud_msc_scsi_cb(uint8_t lun, uint8_t const scsi_cmd[16], void *buffer, uint16_t bufsize)
{
    // read10 & write10 has their own callback and MUST not be handled here
    ESP_LOGD(__func__, "");

    if (unlikely(lun >= LOGICAL_DISK_NUM)) {
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

        case SCSI_CMD_START_STOP_UNIT:
        // Host try to eject/safe remove/poweroff us. We could safely disconnect with disk storage, or go into lower power
        /* scsi_start_stop_unit_t const * start_stop = (scsi_start_stop_unit_t const *) scsi_cmd;
            // Start bit = 0 : low power mode, if load_eject = 1 : unmount disk storage as well
            // Start bit = 1 : Ready mode, if load_eject = 1 : mount disk storage
            start_stop->start;
            start_stop->load_eject;
        */
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
