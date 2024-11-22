
#include <stdio.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/ringbuf.h"
#include "freertos/event_groups.h"

#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_log.h"
#include "esp_system.h"
#include "assert.h"
#include "bsp_esp32_s3_usb_otg_ev.h"
#include "display_printf.h"
#include "tusb_hid.h"
#include "tusb_msc.h"
#include "qrcode.h"
#include "app.h"

static const char *TAG = "usb_hid_mouse";
#define EVENT_TASK_KILL_BIT_0	( 1 << 0 )
#define EVENT_TASK_KILLED_BIT_1	( 1 << 1 )
#define MOUSE_OFFSET 8

QueueHandle_t g_usb_hid_mouse_hdl = NULL;
static TaskHandle_t s_task_hdl = NULL;
static EventGroupHandle_t s_event_group_hdl = NULL;

static esp_err_t init_flash_fat(const char* base_path)
{
    ESP_LOGI(TAG, "Mounting FAT filesystem");
    esp_err_t ret = ESP_FAIL;
    // To mount device we need name of device partition, define base_path
    // and allow format partition in case if it is new one and was not formated before
    // Handle of the wear levelling library instance
    wl_handle_t wl_handle_1 = WL_INVALID_HANDLE;
    ESP_LOGI(TAG, "using internal flash");
    const esp_vfs_fat_mount_config_t mount_config = {
        .format_if_mount_failed = true,
        .max_files = 9,
        .allocation_unit_size = CONFIG_WL_SECTOR_SIZE
    };
    ret = esp_vfs_fat_spiflash_mount(base_path, NULL, &mount_config, &wl_handle_1);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount FATFS (%s)", esp_err_to_name(ret));
        return ESP_FAIL;
    }
    return ESP_OK;
}

static void _cb_hid_mount(int pdrv, void *arg)
{
    hmi_event_t event = {
        .id = USR_RESERVED_1
    };
    xQueueSend(g_usb_hid_mouse_hdl, &event, portMAX_DELAY);
    ESP_LOGI(TAG, "%s", __func__);
}

static void _cb_hid_unmount(int pdrv, void *arg)
{
    hmi_event_t event = {
        .id = USR_RESERVED_2
    };
    xQueueSend(g_usb_hid_mouse_hdl, &event, portMAX_DELAY);
    ESP_LOGI(TAG, "%s", __func__);
}

void usb_hid_mouse_task(void *pvParameters)
{
    iot_board_usb_set_mode(USB_DEVICE_MODE);
    iot_board_usb_device_set_power(false, false);
    g_usb_hid_mouse_hdl = xQueueCreate(5, sizeof(hmi_event_t));
    assert(g_usb_hid_mouse_hdl != NULL);
    DISPLAY_PRINTF_INIT((scr_driver_t *)iot_board_get_handle(BOARD_LCD_ID));
    DISPLAY_PRINTF_SET_FONT(Font16);
    DISPLAY_PRINTF_CLEAR();
    DISPLAY_PRINTF_LINE(TAG, 1, COLOR_GREEN, "USB HID Mouse");
    DISPLAY_PRINTF_LINE(TAG, 2, COLOR_RED, "Wait Connect to PC");
    EventGroupHandle_t s_event_group_hdl = (EventGroupHandle_t) pvParameters;
    uint8_t pdrv_num = 1;
    sdmmc_card_t *card_hdl = (sdmmc_card_t *)iot_board_get_handle(BOARD_SDCARD_ID);
    if (card_hdl == NULL) pdrv_num = 0;
    init_flash_fat("/local");
    tinyusb_config_t tusb_cfg = {
        .descriptor = NULL,
        .string_descriptor = NULL,
        .external_phy = false // In the most cases you need to use a `false` value
    };

    tinyusb_config_msc_t msc_cfg = {
        .pdrv = pdrv_num,
        .cb_mount = _cb_hid_mount,
        .cb_unmount = _cb_hid_unmount,
    };

    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));
    ESP_ERROR_CHECK(tusb_msc_init(&msc_cfg));
    ESP_LOGI(TAG, "USB initialization DONE");
    hmi_event_t current_event;
    bool if_connect = false;

    while (!(xEventGroupGetBits(s_event_group_hdl) & EVENT_TASK_KILL_BIT_0)) {
        if(xQueueReceive(g_usb_hid_mouse_hdl, &current_event, portMAX_DELAY) != pdTRUE) continue;
        switch (current_event.id) {
            case USR_RESERVED_1:
                if_connect = true;
                DISPLAY_PRINTF_SET_FONT(Font16);
                DISPLAY_PRINTF_LINE(TAG, 2, COLOR_GREEN, "    Connected       ");
                DISPLAY_PRINTF_LINE(TAG, 3, COLOR_RED, "    Click to Move      ");
                break;
            case USR_RESERVED_2:
                if_connect = false;
                DISPLAY_PRINTF_SET_FONT(Font16);
                DISPLAY_PRINTF_LINE(TAG, 2, COLOR_RED, "    Lose Connect    ");
                DISPLAY_PRINTF_LINE(TAG, 3, COLOR_RED, "                        ");
                break;
            case BTN_CLICK_MENU:
                if(if_connect) tinyusb_hid_mouse_move_report(MOUSE_OFFSET, 0, 0, 0);
                DISPLAY_PRINTF_SET_FONT(Font24);
                DISPLAY_PRINTF_LINE(TAG, 5, COLOR_RED, "      Right  ");
                break;
            case BTN_CLICK_UP:
                if(if_connect) tinyusb_hid_mouse_move_report(0, -MOUSE_OFFSET, 0, 0);
                DISPLAY_PRINTF_SET_FONT(Font24);
                DISPLAY_PRINTF_LINE(TAG, 5, COLOR_RED, "      Up   ");
                break;
            case BTN_CLICK_DOWN:
                if(if_connect) tinyusb_hid_mouse_move_report(0, MOUSE_OFFSET, 0, 0);
                DISPLAY_PRINTF_SET_FONT(Font24);
                DISPLAY_PRINTF_LINE(TAG, 5, COLOR_RED, "      Down   ");
                break;
            case BTN_CLICK_OK:
                if(if_connect) tinyusb_hid_mouse_move_report(-MOUSE_OFFSET, 0, 0, 0);
                DISPLAY_PRINTF_SET_FONT(Font24);
                DISPLAY_PRINTF_LINE(TAG, 5, COLOR_RED, "      Left  ");
                break;
            default:
                break;
        }
    };
    xEventGroupSetBits(s_event_group_hdl, EVENT_TASK_KILLED_BIT_1);
    xEventGroupClearBits(s_event_group_hdl, EVENT_TASK_KILL_BIT_0);
    s_task_hdl = NULL;
    vTaskDelete(s_task_hdl);
}

void usb_hid_mouse_init(void)
{
    if (s_task_hdl != NULL) return;
    s_event_group_hdl = xEventGroupCreate();
    xEventGroupClearBits(s_event_group_hdl, EVENT_TASK_KILL_BIT_0 | EVENT_TASK_KILLED_BIT_1);
    xTaskCreate(usb_hid_mouse_task, "usbd_hid_mouse", 4096, (void *)s_event_group_hdl, TASK_APP_PRIO_MIN, &s_task_hdl);
    ESP_LOGI(TAG, "APP HID Mouse Inited");
}

void usb_hid_mouse_deinit(void)
{
    if (s_task_hdl == NULL) return;
    xEventGroupSetBits(s_event_group_hdl, EVENT_TASK_KILL_BIT_0);
    _cb_hid_unmount(0, NULL);
    xEventGroupWaitBits(s_event_group_hdl, EVENT_TASK_KILLED_BIT_1, true, true, portMAX_DELAY);
    vEventGroupDelete(s_event_group_hdl);
    s_event_group_hdl = NULL;
    ESP_LOGW(TAG, "APP HID Mouse Deinited");
}