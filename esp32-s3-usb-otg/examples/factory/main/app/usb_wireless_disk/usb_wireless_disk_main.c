
#include <stdio.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/ringbuf.h"
#include "freertos/event_groups.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_spiffs.h"
#include "nvs_flash.h"
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_wifi.h"
#include "driver/sdmmc_defs.h"
#include "driver/sdmmc_types.h"
#include "sdmmc_cmd.h"
#include "assert.h"
#include "bsp_esp32_s3_usb_otg_ev.h"
#include "display_printf.h"
#include "tusb_msc.h"
#include "qrcode.h"
#include "app.h"

static const char *TAG = "usb_msc_demo";
#define EVENT_TASK_KILL_BIT_0	( 1 << 0 )
#define EVENT_TASK_KILLED_BIT_1	( 1 << 1 )

QueueHandle_t g_disk_queue_hdl = NULL;
static TaskHandle_t s_task_hdl = NULL;
static EventGroupHandle_t s_event_group_hdl = NULL;

extern esp_err_t start_file_server(const char *base_path);

static void _display_card_info(const sdmmc_card_t* card)
{
    bool print_scr = false;
    bool print_csd = false;
    const char* type;
    char msg[64] = "";
    sprintf(msg, "SDCart Name: %s", card->cid.name);
    DISPLAY_PRINTF_LINE("SD", 2, COLOR_RED, msg);
    if (card->is_sdio) {
        type = "SDIO";
        print_scr = true;
        print_csd = true;
    } else if (card->is_mmc) {
        type = "MMC";
        print_csd = true;
    } else {
        type = (card->ocr & SD_OCR_SDHC_CAP) ? "SDHC/SDXC" : "SDSC";
    }
    sprintf(msg, "Type: %s", type);
    DISPLAY_PRINTF_LINE("SD", 3, COLOR_RED, msg);
    if (card->max_freq_khz < 1000) {
        sprintf(msg, "Speed: %d kHz", card->max_freq_khz);
    } else {
        sprintf(msg, "Speed: %d MHz%s", card->max_freq_khz / 1000,
                card->is_ddr ? ", DDR" : "");
    }
    DISPLAY_PRINTF_LINE("SD", 4, COLOR_RED, msg);
    sprintf(msg, "Size: %lluMB", ((uint64_t) card->csd.capacity) * card->csd.sector_size / (1024 * 1024));
    DISPLAY_PRINTF_LINE("SD", 5, COLOR_RED, msg);

    if (print_csd) {
        sprintf(msg, "CSD: ver=%d, sector_size=%d, capacity=%d read_bl_len=%d",
                card->csd.csd_ver,
                card->csd.sector_size, card->csd.capacity, card->csd.read_block_len);
    }
    if (print_scr) {
        sprintf(msg, "SCR: sd_spec=%d, bus_width=%d", card->scr.sd_spec, card->scr.bus_width);
    }
}

#define QR_BUF_LEN_MAX ((((10) * 4 + 17) * ((10) * 4 + 17) + 7) / 8 + 1) // Calculates the number of bytes needed to store any Version 10 QR Code
static char s_wifi_qr_buffer[QR_BUF_LEN_MAX] = {0};

static void display_info(int page_index)
{
    if (page_index == 0) { //main infomation
        DISPLAY_PRINTF_CLEAR();
        sdmmc_card_t *card_hdl = (sdmmc_card_t *)iot_board_get_handle(BOARD_SDCARD_ID);
        if(card_hdl == NULL) {
            DISPLAY_PRINTF_LINE("SD", 1, COLOR_GREEN, "USB Wireless Disk");
            DISPLAY_PRINTF_LINE("SD", 2, COLOR_RED, "SDCard Not Found");
            DISPLAY_PRINTF_LINE("SD", 3, COLOR_RED, "Or Not Supported!");
            DISPLAY_PRINTF_LINE("SD", 5, COLOR_YELLOW, "Using internal Flash");}
        else {
            _display_card_info(card_hdl);
        }
        DISPLAY_PRINTF_LINE("SD", 6, COLOR_BLUE, "Access files from");
        DISPLAY_PRINTF_LINE("SD", 7, COLOR_BLUE, "USB or Wi-Fi AP");
        DISPLAY_PRINTF_LINE("SD", 9, COLOR_RED, "Press OK:");
        DISPLAY_PRINTF_LINE("SD", 10, COLOR_BLUE, "Scan QRCode");
        DISPLAY_PRINTF_LINE("SD", 11, COLOR_BLUE, "to Connect Wi-Fi");
        DISPLAY_PRINTF_LINE("SD", 13, COLOR_RED, "Server: 192.168.4.1"); 
    } else if (page_index == 1) { //qrcode picture
        wifi_config_t wifi_cfg;
        esp_wifi_get_config(WIFI_IF_AP, &wifi_cfg);
        snprintf(s_wifi_qr_buffer, sizeof(s_wifi_qr_buffer), "WIFI:S:%s;T:%s;P:%s;H:%s;", wifi_cfg.ap.ssid, wifi_cfg.ap.password[0]?"WPA":"", wifi_cfg.ap.password, wifi_cfg.ap.ssid_hidden?"true":"false");
        esp_qrcode_config_t qr_cfg = PAINTER_QRCODE_CONFIG_DEFAULT();
        ESP_LOGI(TAG, "Scan below QR Code to Connect Wi-Fi\n");
        DISPLAY_PRINTF_CLEAR();
        esp_qrcode_generate(&qr_cfg, s_wifi_qr_buffer);
        DISPLAY_PRINTF_LINE("SD", 13, COLOR_BLUE, "Scan Wi-Fi QRCode");
        DISPLAY_PRINTF_LINE("SD", 14, COLOR_RED, "Server: 192.168.4.1");
    } else {
        DISPLAY_PRINTF_CLEAR();
    }
}

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

extern void start_dns_server(void);
void usb_wireless_disk_task(void *pvParameters)
{
    iot_board_usb_set_mode(USB_DEVICE_MODE);
    iot_board_usb_device_set_power(false, false);
    g_disk_queue_hdl = xQueueCreate(1, sizeof(hmi_event_t));
    assert(g_disk_queue_hdl != NULL);
    DISPLAY_PRINTF_INIT((scr_driver_t *)iot_board_get_handle(BOARD_LCD_ID));
    DISPLAY_PRINTF_SET_FONT(Font16);
    EventGroupHandle_t s_event_group_hdl = (EventGroupHandle_t) pvParameters;
    sdmmc_card_t *card_hdl = (sdmmc_card_t *)iot_board_get_handle(BOARD_SDCARD_ID);
    if (card_hdl == NULL) {
        init_flash_fat(BOARD_SDCARD_BASE_PATH);
    }
    /* Start the file server */
#ifdef CONFIG_WIFI_HTTP_ACCESS
    ESP_ERROR_CHECK(iot_board_wifi_init());
    ESP_ERROR_CHECK(start_file_server(BOARD_SDCARD_BASE_PATH));
    //start_dns_server();
#endif

    tinyusb_config_t tusb_cfg = {
        .descriptor = NULL,
        .string_descriptor = NULL,
        .external_phy = false // In the most cases you need to use a `false` value
    };

    tinyusb_config_msc_t msc_cfg = {
        .pdrv = 0,
    };

    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));
    ESP_ERROR_CHECK(tusb_msc_init(&msc_cfg));
    ESP_LOGI(TAG, "USB initialization DONE");

    hmi_event_t current_event;
    int current_info_page = 0;
    int max_info_page = 1;
    display_info(current_info_page);
    while (!(xEventGroupGetBits(s_event_group_hdl) & EVENT_TASK_KILL_BIT_0)) {
        if(xQueueReceive(g_disk_queue_hdl, &current_event, portMAX_DELAY) != pdTRUE) continue;
        switch (current_event.id) {
            case BTN_CLICK_MENU:
                break;
            case BTN_CLICK_UP:
                if(++current_info_page > max_info_page) current_info_page = 0;
                break;
            case BTN_CLICK_DOWN:
                if(--current_info_page < 0) current_info_page = max_info_page;
                break;
            case BTN_CLICK_OK:
                if(--current_info_page < 0) current_info_page = max_info_page;
                break;
            default:
                break;
        }
        display_info(current_info_page);
    };
    xEventGroupSetBits(s_event_group_hdl, EVENT_TASK_KILLED_BIT_1);
    xEventGroupClearBits(s_event_group_hdl, EVENT_TASK_KILL_BIT_0);
    s_task_hdl = NULL;
    vTaskDelete(s_task_hdl);
}

void usb_wireless_disk_init(void)
{
    if (s_task_hdl != NULL) return;
    s_event_group_hdl = xEventGroupCreate();
    xEventGroupClearBits(s_event_group_hdl, EVENT_TASK_KILL_BIT_0 | EVENT_TASK_KILLED_BIT_1);
    xTaskCreate(usb_wireless_disk_task, "usb_wireless_disk", 4096, (void *)s_event_group_hdl, TASK_APP_PRIO_MIN, &s_task_hdl);
    ESP_LOGI(TAG, "Disk APP Inited");
}

void usb_wireless_disk_deinit(void)
{
    if (s_task_hdl == NULL) return;
    xEventGroupSetBits(s_event_group_hdl, EVENT_TASK_KILL_BIT_0);
    xEventGroupWaitBits(s_event_group_hdl, EVENT_TASK_KILLED_BIT_1, true, true, portMAX_DELAY);
    vEventGroupDelete(s_event_group_hdl);
    s_event_group_hdl = NULL;
    ESP_LOGW(TAG, "Disk APP Deinited");
}