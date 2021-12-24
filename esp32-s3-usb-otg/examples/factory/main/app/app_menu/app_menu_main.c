
// Copyright 2019-2021 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_spiffs.h"
#include "hal/usb_hal.h"

#include "jpegd2.h"
#include "app.h"
#include "bsp_esp32_s3_usb_otg_ev.h"

static char *TAG = "app_menu";
#define CONFIG_LCD_BUF_WIDTH 240
#define CONFIG_LCD_BUF_HIGHT 48

#define DEMO_SPI_MAX_TRANFER_SIZE (CONFIG_LCD_BUF_WIDTH * CONFIG_LCD_BUF_HIGHT * 2 + 64)
#define PIC_JPEG_MAX_SIZE (45 * 1024)

#define EVENT_TASK_KILL_BIT_0	( 1 << 0 )
#define EVENT_TASK_KILLED_BIT_1	( 1 << 1 )

extern __NOINIT_ATTR int s_driver_index;
QueueHandle_t g_app_menu_queue_hdl = NULL;
static TaskHandle_t s_task_hdl = NULL;
static EventGroupHandle_t s_event_group_hdl = NULL;

void app_menu_task( void *pvParameters )
{
    iot_board_usb_set_mode(USB_DEVICE_MODE);
    iot_board_usb_device_set_power(false, false);
    usb_hal_context_t hal = {
        .use_external_phy = true
    };
    usb_hal_init(&hal);
    int display_current = 0;
    hmi_event_t current_event;
    EventGroupHandle_t event_group_hdl = (EventGroupHandle_t) pvParameters;

    esp_vfs_spiffs_conf_t spiffs_config = {
        .base_path              = "/spiffs",
        .partition_label        = NULL,
        .max_files              = 5,
        .format_if_mount_failed = false
    };

    ESP_ERROR_CHECK(esp_vfs_spiffs_register(&spiffs_config));

    /* malloc a buffer for RGB565 data, as 320*240*2 = 153600B,
    here malloc a smaller buffer refresh lcd with steps */
    uint8_t *lcd_buffer = (uint8_t *)heap_caps_malloc(DEMO_SPI_MAX_TRANFER_SIZE, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    assert(lcd_buffer != NULL);

    uint8_t *jpeg_buf = malloc(PIC_JPEG_MAX_SIZE);
    assert(jpeg_buf != NULL);
    char file_name[64] = {0};

    g_app_menu_queue_hdl = xQueueCreate( 5, sizeof(hmi_event_t));
    assert(g_app_menu_queue_hdl != NULL);

    if (s_driver_index >= 0 && s_driver_index < _app_driver_count) {
        sprintf(file_name, "/spiffs/icon/%s", _app_driver[s_driver_index].icon_name);
        FILE *fd = fopen(file_name, "r");
        if (fd != NULL) {
            int read_bytes = fread(jpeg_buf, 1, PIC_JPEG_MAX_SIZE, fd);
            fclose(fd);
            mjpegdraw(jpeg_buf, read_bytes, lcd_buffer, CONFIG_LCD_BUF_WIDTH, CONFIG_LCD_BUF_HIGHT, board_lcd_draw_image, 240, 240);
        }
        display_current = s_driver_index;
    }

    while (!(xEventGroupGetBits(event_group_hdl) & EVENT_TASK_KILL_BIT_0)) {
        if(xQueueReceive(g_app_menu_queue_hdl, &current_event, portMAX_DELAY) != pdTRUE) continue;
        switch (current_event.id) {
            case BTN_CLICK_MENU:
                break;
            case BTN_CLICK_UP:
                break;
            case BTN_CLICK_DOWN:
                break;
            case BTN_CLICK_OK:
                break;
            default:
                break;
        }
        if(display_current == s_driver_index) continue;
        sprintf(file_name, "/spiffs/icon/%s", _app_driver[s_driver_index].icon_name);

        FILE *fd = fopen(file_name, "r");
        if (fd == NULL) {
            ESP_LOGE(TAG, "open %s filed ", file_name);
            continue;
        }
        int read_bytes = fread(jpeg_buf, 1, PIC_JPEG_MAX_SIZE, fd);
        fclose(fd);
        mjpegdraw(jpeg_buf, read_bytes, lcd_buffer, CONFIG_LCD_BUF_WIDTH, CONFIG_LCD_BUF_HIGHT, board_lcd_draw_image, 240, 240);
        display_current = s_driver_index;
        ESP_LOGI(TAG, "file_name: %s, fd: %p, read_bytes: %d, free_heap: %d", file_name, fd, read_bytes, esp_get_free_heap_size());
    }

    QueueHandle_t queue_hdl = g_app_menu_queue_hdl;
    g_app_menu_queue_hdl = NULL;
    vQueueDelete(queue_hdl);
    free(lcd_buffer);
    free(jpeg_buf);
    esp_vfs_spiffs_unregister(NULL);
    xEventGroupSetBits(event_group_hdl, EVENT_TASK_KILLED_BIT_1);
    xEventGroupClearBits(event_group_hdl, EVENT_TASK_KILL_BIT_0);
    s_task_hdl = NULL;
    vTaskDelete(s_task_hdl);
}

void app_menu_init(void)
{
    if (s_task_hdl != NULL) return;
    s_event_group_hdl = xEventGroupCreate();
    xEventGroupClearBits(s_event_group_hdl, EVENT_TASK_KILL_BIT_0 | EVENT_TASK_KILLED_BIT_1);
    xTaskCreate(app_menu_task, "menu", 4096, (void *)s_event_group_hdl, TASK_APP_PRIO_MIN, &s_task_hdl);
    ESP_LOGI(TAG, "Menu APP Inited");
}

void app_menu_deinit()
{
    if (s_task_hdl == NULL) return;
    xEventGroupSetBits(s_event_group_hdl, EVENT_TASK_KILL_BIT_0);
    xEventGroupWaitBits(s_event_group_hdl, EVENT_TASK_KILLED_BIT_1, true, true, portMAX_DELAY);
    vEventGroupDelete(s_event_group_hdl);
    s_event_group_hdl = NULL;
    ESP_LOGW(TAG, "Menu APP Deinited");
}