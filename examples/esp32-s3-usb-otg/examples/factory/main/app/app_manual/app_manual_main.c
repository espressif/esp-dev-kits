
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
#include "esp_log.h"
#include "app.h"
#include "bsp_esp32_s3_usb_otg_ev.h"
#include "display_printf.h"
#include "qrcode.h"

static char *TAG = "app_manual";

#define EVENT_TASK_KILL_BIT_0	( 1 << 0 )
#define EVENT_TASK_KILLED_BIT_1	( 1 << 1 )

QueueHandle_t g_app_manual_queue_hdl = NULL;
static TaskHandle_t s_task_hdl = NULL;
static EventGroupHandle_t s_event_group_hdl = NULL;
//#define QR_BUF_LEN_MAX ((((10) * 4 + 17) * ((10) * 4 + 17) + 7) / 8 + 1) // Calculates the number of bytes needed to store any Version 10 QR Code
static char *contact_us = "http://qr10.cn/Er5Bxj";
static char *user_guide = "http://qr10.cn/FuIsa1";

static void display_info(int page_index)
{
    esp_qrcode_config_t qr_cfg = PAINTER_QRCODE_CONFIG_DEFAULT();
    DISPLAY_PRINTF_CLEAR();
    if (page_index == 0) { //main infomation
        esp_qrcode_generate(&qr_cfg, contact_us);
        DISPLAY_PRINTF_LINE("SD", 14, COLOR_RED, "      Contact US");
    } else if (page_index == 1) {
        esp_qrcode_generate(&qr_cfg, user_guide);
        DISPLAY_PRINTF_LINE("SD", 14, COLOR_RED, "      User Guide");
    } else {
        DISPLAY_PRINTF_CLEAR();
    }
}

void app_manual_task( void *pvParameters )
{
    hmi_event_t current_event;
    EventGroupHandle_t event_group_hdl = (EventGroupHandle_t) pvParameters;

    g_app_manual_queue_hdl = xQueueCreate( 5, sizeof(hmi_event_t));
    assert(g_app_manual_queue_hdl != NULL);
    DISPLAY_PRINTF_INIT((scr_driver_t *)iot_board_get_handle(BOARD_LCD_ID));
    DISPLAY_PRINTF_SET_FONT(Font16);
    int current_info_page = 0;
    int max_info_page = 1;
    display_info(current_info_page);
    while (!(xEventGroupGetBits(event_group_hdl) & EVENT_TASK_KILL_BIT_0)) {
        if(xQueueReceive(g_app_manual_queue_hdl, &current_event, portMAX_DELAY) != pdTRUE) continue;
        switch (current_event.id) {
            case BTN_CLICK_MENU:
                if(++current_info_page > max_info_page) current_info_page = 0;
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
    }

    QueueHandle_t queue_hdl = g_app_manual_queue_hdl;
    g_app_manual_queue_hdl = NULL;
    vQueueDelete(queue_hdl);
    xEventGroupSetBits(event_group_hdl, EVENT_TASK_KILLED_BIT_1);
    xEventGroupClearBits(event_group_hdl, EVENT_TASK_KILL_BIT_0);
    s_task_hdl = NULL;
    vTaskDelete(s_task_hdl);
}

void app_manual_init(void)
{
    if (s_task_hdl != NULL) return;
    s_event_group_hdl = xEventGroupCreate();
    xEventGroupClearBits(s_event_group_hdl, EVENT_TASK_KILL_BIT_0 | EVENT_TASK_KILLED_BIT_1);
    xTaskCreate(app_manual_task, "manual", 4096, (void *)s_event_group_hdl, TASK_APP_PRIO_MIN, &s_task_hdl);
    ESP_LOGI(TAG, "Manual APP Inited");
}

void app_manual_deinit()
{
    if (s_task_hdl == NULL) return;
    xEventGroupSetBits(s_event_group_hdl, EVENT_TASK_KILL_BIT_0);
    xEventGroupWaitBits(s_event_group_hdl, EVENT_TASK_KILLED_BIT_1, true, true, portMAX_DELAY);
    vEventGroupDelete(s_event_group_hdl);
    s_event_group_hdl = NULL;
    ESP_LOGW(TAG, "Manual APP Deinited");
}