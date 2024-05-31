/* SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "soc/soc_caps.h"
#include "esp_check.h"
#include "driver/gpio.h"
#include "msc_host.h"
#include "msc_host_vfs.h"
#include "esp_private/usb_phy.h"
#include "usb/usb_host.h"
#include "app_ui_control.h"

static const char *TAG = "app_msc";

#define USB_DISCONNECT_PIN  GPIO_NUM_10

#define READY_TO_UNINSTALL (HOST_NO_CLIENT | HOST_ALL_FREE)

static msc_host_device_handle_t msc_device;
static msc_host_device_info_t info;
static msc_host_vfs_handle_t vfs_handle;

typedef enum {
    HOST_NO_CLIENT = 0x1,
    HOST_ALL_FREE = 0x2,
    DEVICE_CONNECTED = 0x4,
    DEVICE_DISCONNECTED = 0x8,
    DEVICE_ADDRESS_MASK = 0xFF0,
} app_event_t;

static EventGroupHandle_t usb_flags;

static void msc_event_cb(const msc_host_event_t *event, void *arg)
{
    if (event->event == MSC_DEVICE_CONNECTED) {
        ESP_LOGI(TAG, "MSC device connected");
        // Obtained USB device address is placed after application events
        xEventGroupSetBits(usb_flags, DEVICE_CONNECTED | (event->device.address << 4));
        ui_control_msc_push();
    } else if (event->event == MSC_DEVICE_DISCONNECTED) {
        xEventGroupSetBits(usb_flags, DEVICE_DISCONNECTED);
        ui_control_msc_pull();
        ESP_LOGI(TAG, "MSC device disconnected");
    }
}

static void print_device_info(msc_host_device_info_t *info)
{
    const size_t megabyte = 1024 * 1024;
    uint64_t capacity = ((uint64_t)info->sector_size * info->sector_count) / megabyte;

    printf("Device info:\n");
    printf("\t Capacity: %llu MB\n", capacity);
    printf("\t Sector size: %"PRIu32"\n", info->sector_size);
    printf("\t Sector count: %"PRIu32"\n", info->sector_count);
    printf("\t PID: 0x%4X \n", info->idProduct);
    printf("\t VID: 0x%4X \n", info->idVendor);
    wprintf(L"\t iProduct: %S \n", info->iProduct);
    wprintf(L"\t iManufacturer: %S \n", info->iManufacturer);
    wprintf(L"\t iSerialNumber: %S \n", info->iSerialNumber);
}

static bool wait_for_event(EventBits_t event, TickType_t timeout)
{
    return xEventGroupWaitBits(usb_flags, event, pdTRUE, pdTRUE, timeout) & event;
}

// Handles common USB host library events
static void handle_usb_events(void *args)
{
    while (1) {
        uint32_t event_flags;
        usb_host_lib_handle_events(portMAX_DELAY, &event_flags);

        // Release devices once all clients has deregistered
        if (event_flags & USB_HOST_LIB_EVENT_FLAGS_NO_CLIENTS) {
            usb_host_device_free_all();
            xEventGroupSetBits(usb_flags, HOST_NO_CLIENT);
        }
        // Give ready_to_uninstall_usb semaphore to indicate that USB Host library
        // can be deinitialized, and terminate this task.
        if (event_flags & USB_HOST_LIB_EVENT_FLAGS_ALL_FREE) {
            xEventGroupSetBits(usb_flags, HOST_ALL_FREE);
        }
    }

    vTaskDelete(NULL);
}

static uint8_t wait_for_msc_device(void)
{
    EventBits_t event;

    ESP_LOGI(TAG, "Waiting for USB stick to be connected");
    event = xEventGroupWaitBits(usb_flags, DEVICE_CONNECTED | DEVICE_ADDRESS_MASK,
                                pdTRUE, pdFALSE, portMAX_DELAY);
    ESP_LOGI(TAG, "connection...");
    // Extract USB device address from event group bits
    return (event & DEVICE_ADDRESS_MASK) >> 4;
}

static void msc_task(void *args)
{
    const esp_vfs_fat_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 1024,
    };

    while (1) {
        if (wait_for_event(DEVICE_CONNECTED, 200 / portTICK_PERIOD_MS)) {
            uint8_t device_address = wait_for_msc_device();
            ESP_ERROR_CHECK( msc_host_install_device(device_address, &msc_device) );
            msc_host_print_descriptors(msc_device);
            ESP_ERROR_CHECK( msc_host_get_device_info(msc_device, &info) );
            print_device_info(&info);
            ESP_ERROR_CHECK( msc_host_vfs_register(msc_device, "", &mount_config, &vfs_handle) );
            ui_control_msc_ready();
        } else if (wait_for_event(DEVICE_DISCONNECTED, 200 / portTICK_PERIOD_MS)) {
            xEventGroupClearBits(usb_flags, READY_TO_UNINSTALL);
            ESP_ERROR_CHECK( msc_host_vfs_unregister(vfs_handle) );
            ESP_ERROR_CHECK( msc_host_uninstall_device(msc_device) );
        }
    }

    vTaskDelete(NULL);
}

esp_err_t msc_init(void)
{
    BaseType_t task_created;

    usb_flags = xEventGroupCreate();
    assert(usb_flags);

    const usb_host_config_t host_config = {
        .skip_phy_setup = false,
        .intr_flags = 0,
    };
    ESP_ERROR_CHECK(usb_host_install(&host_config));

    task_created = xTaskCreate(handle_usb_events, "usb_events", 2048, NULL, 2, NULL);
    assert(task_created);

    const msc_host_driver_config_t msc_config = {
        .create_backround_task = true,
        .task_priority = 5,
        .stack_size = 3 * 1024,
        .core_id = 1,
        .callback = msc_event_cb,
    };
    ESP_ERROR_CHECK( msc_host_install(&msc_config) );

    task_created = xTaskCreate(msc_task, "msc_task", 4096, NULL, 5, NULL);
    assert(task_created);

    return ESP_OK;
}
