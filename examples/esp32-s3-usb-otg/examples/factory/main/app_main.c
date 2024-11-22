// Copyright 2021-2022 Espressif Systems (Shanghai) PTE LTD
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

#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "hal/usb_hal.h"

#include "esp_system.h"
#include "esp_spi_flash.h"
#include "bsp_esp32_s3_usb_otg_ev.h"
#include "spi_bus.h"
#include "app.h"
#include "esp_app_format.h"
#include "esp_ota_ops.h"

static const char *TAG = "app_main";
QueueHandle_t s_default_queue_hdl = NULL;

#define CONFIG_LCD_BUF_WIDTH 240
#define CONFIG_LCD_BUF_HIGHT 48

#define DEMO_SPI_MAX_TRANFER_SIZE (CONFIG_LCD_BUF_WIDTH * CONFIG_LCD_BUF_HIGHT * 2 + 64)

static void button_ok_single_click_cb(void *arg)
{
    hmi_event_t item_id = {.id = BTN_CLICK_OK};
    if (s_default_queue_hdl) {
        xQueueSend(s_default_queue_hdl, ( void *) &item_id, 0);
    }
    for (size_t i = 0; i < _app_driver_count; i++) {
        if( (_app_driver[i].p_queue_hdl) && *(_app_driver[i].p_queue_hdl) ) {
            xQueueSend(*(_app_driver[i].p_queue_hdl), ( void * ) &item_id, 0);
        }
    }
    ESP_LOGI(TAG, "BTN OK: BUTTON_SINGLE_CLICK");
}

static void button_up_single_click_cb(void *arg)
{
    hmi_event_t item_id = {.id = BTN_CLICK_UP};
    if (s_default_queue_hdl) {
        xQueueSend(s_default_queue_hdl, ( void *) &item_id, 0);
    }
    for (size_t i = 0; i < _app_driver_count; i++) {
        if( (_app_driver[i].p_queue_hdl) && *(_app_driver[i].p_queue_hdl) ) {
            xQueueSend(*(_app_driver[i].p_queue_hdl), ( void * ) &item_id, 0);
        }
    }
    ESP_LOGI(TAG, "BTN UP: BUTTON_SINGLE_CLICK");
}

static void button_dw_single_click_cb(void *arg)
{
    hmi_event_t item_id = {.id = BTN_CLICK_DOWN};
    if (s_default_queue_hdl) {
        xQueueSend(s_default_queue_hdl, ( void *) &item_id, portMAX_DELAY);
    }
    for (size_t i = 0; i < _app_driver_count; i++) {
        if( (_app_driver[i].p_queue_hdl) && *(_app_driver[i].p_queue_hdl) ) {
            xQueueSend(*(_app_driver[i].p_queue_hdl), ( void * ) &item_id, 0);
        }
    }
    ESP_LOGI(TAG, "BTN DW: BUTTON_SINGLE_CLICK");
}

static void button_menu_single_click_cb(void *arg)
{
    hmi_event_t item_id = {.id = BTN_CLICK_MENU};
    if (s_default_queue_hdl) {
        xQueueSend(s_default_queue_hdl, ( void *) &item_id, portMAX_DELAY);
    }
    for (size_t i = 0; i < _app_driver_count; i++) {
        if( (_app_driver[i].p_queue_hdl) && *(_app_driver[i].p_queue_hdl) ) {
            xQueueSend(*(_app_driver[i].p_queue_hdl), ( void * ) &item_id, 0);
        }
    }
    ESP_LOGI(TAG, "BTN MENU: BUTTON_SINGLE_CLICK");
}

static void button_menu_double_click_cb(void *arg)
{
    hmi_event_t item_id = {.id = BTN_DOUBLE_CLICK_MENU};
    if (s_default_queue_hdl) {
        xQueueSend(s_default_queue_hdl, ( void *) &item_id, portMAX_DELAY);
    }
    for (size_t i = 0; i < _app_driver_count; i++) {
        if( (_app_driver[i].p_queue_hdl) && *(_app_driver[i].p_queue_hdl) ) {
            xQueueSend(*(_app_driver[i].p_queue_hdl), ( void * ) &item_id, 0);
        }
    }
    ESP_LOGI(TAG, "BTN MENU: BUTTON_DOUBLE_CLICK");
}

static void button_menu_long_press_cb(void *arg)
{
    hmi_event_t item_id = {.id = BTN_LONG_PRESS_MENU};
    if (s_default_queue_hdl) {
        xQueueSend(s_default_queue_hdl, ( void *) &item_id, portMAX_DELAY);
    }
    for (size_t i = 0; i < _app_driver_count; i++) {
        if( (_app_driver[i].p_queue_hdl) && *(_app_driver[i].p_queue_hdl) ) {
            xQueueSend(*(_app_driver[i].p_queue_hdl), ( void * ) &item_id, 0);
        }
    }
    ESP_LOGI(TAG, "BTN MENU: BUTTON_LONG_PRESS_CLICK");
}

__NOINIT_ATTR int s_driver_index = 0;
static int s_active_driver_index = 0;
static bool app_kill(int index);

static bool app_launch(int index)
{
    assert(index >= 0 && index < _app_driver_count);
    if (_app_driver[index].flags.restart_before_init) {
        esp_restart();
    }
    _app_driver[index].init();
    s_active_driver_index = index;
    return true;
}

static bool app_kill(int index)
{
    assert(index >= 0 && index < _app_driver_count);
    if (_app_driver[index].deinit){
        _app_driver[index].deinit();
    }

    if (_app_driver[index].flags.restart_after_deinit || _app_driver[index].deinit == NULL) {
        s_driver_index = 0; //Prevention killed app restart
        esp_restart();
    }

    if(s_active_driver_index == index) //if killed current app
    s_active_driver_index = 0;
    return true;
}

static bool app_hide(int index)
{
    assert(index >= 0 && index < _app_driver_count);
    if (_app_driver[index].hide) {
        _app_driver[index].hide();
        return true;
    }
    return false;
}

static bool app_show(int index)
{
    assert(index >= 0 && index < _app_driver_count);
    if (_app_driver[index].show) {
        _app_driver[index].show();
        return true;
    }
    return false;
}

void app_manager_task( void *pvParameters)
{
    hmi_event_t current_event;
    const int driver_tail = _app_driver_count;
    const int driver_head = 1;
    s_default_queue_hdl = xQueueCreate(1, sizeof(hmi_event_t));
    assert(s_default_queue_hdl != NULL);
    while (1) {
        if(xQueueReceive(s_default_queue_hdl, &current_event, portMAX_DELAY) != pdTRUE) continue;
        switch (current_event.id) {
            case BTN_CLICK_MENU:
                break;
            case BTN_DOUBLE_CLICK_MENU:
                if (s_active_driver_index != 0) {
                    app_kill(s_active_driver_index);
                    app_launch(0);
                }
                break;
            case BTN_LONG_PRESS_MENU:
                break;
            case BTN_CLICK_UP:
                if (++s_driver_index >= driver_tail)
                s_driver_index = driver_head;
                break;
            case BTN_CLICK_DOWN:
                if (--s_driver_index < driver_head)
                s_driver_index = driver_tail - 1;
                break;
            case BTN_CLICK_OK:
                if(s_active_driver_index == 0 && s_driver_index > 0 && s_driver_index < _app_driver_count) {
                    app_kill(0);
                    app_launch(s_driver_index);
                }
                break;
            default:
                ESP_LOGE(TAG, "not supported event %d", current_event.id);
                break;
        }
    }
}

static void _print_info()
{
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("\n");
    ESP_LOGW(TAG, "------------------Chip information------------");
    ESP_LOGI(TAG, "This is %s chip with %d CPU core(s), WiFi%s%s, ",
            CONFIG_IDF_TARGET,
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    ESP_LOGI(TAG, "silicon revision %d, ", chip_info.revision);

    ESP_LOGI(TAG, "%dMB %s flash", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    ESP_LOGI(TAG, "Minimum free heap size: %d bytes", esp_get_minimum_free_heap_size());

    ESP_LOGW(TAG, "-----------------MAC information-----------------");
    const esp_app_desc_t *app_desc = esp_ota_get_app_description();
    uint8_t derived_mac_addr[6] = {0};
    ESP_ERROR_CHECK(esp_read_mac(derived_mac_addr, ESP_MAC_WIFI_SOFTAP));
    ESP_LOGI("SoftAP MAC:", "%02x:%02x:%02x:%02x:%02x:%02x",
             derived_mac_addr[0], derived_mac_addr[1], derived_mac_addr[2],
             derived_mac_addr[3], derived_mac_addr[4], derived_mac_addr[5]);
    ESP_LOGW(TAG, "---------------Application information--------------");
    ESP_LOGI(TAG, "Project name:     %s", app_desc->project_name);
    ESP_LOGI(TAG, "App version:      %s", app_desc->version);
    printf("\n");
}

void app_main(void)
{
    _print_info();
    iot_board_init();
    iot_board_usb_set_mode(USB_DEVICE_MODE);
    iot_board_usb_device_set_power(false, false);
    usb_hal_context_t hal = {
        .use_external_phy = true
    };
    usb_hal_init(&hal);

    xTaskCreate(app_manager_task, "app_mng", 4096, NULL, TASK_APP_PRIO_MAX, NULL);

    iot_board_button_register_cb(iot_board_get_handle(BOARD_BTN_OK_ID), BUTTON_SINGLE_CLICK, button_ok_single_click_cb);
    iot_board_button_register_cb(iot_board_get_handle(BOARD_BTN_UP_ID), BUTTON_SINGLE_CLICK, button_up_single_click_cb);
    iot_board_button_register_cb(iot_board_get_handle(BOARD_BTN_DW_ID), BUTTON_SINGLE_CLICK, button_dw_single_click_cb);
    iot_board_button_register_cb(iot_board_get_handle(BOARD_BTN_MN_ID), BUTTON_SINGLE_CLICK, button_menu_single_click_cb);
    iot_board_button_register_cb(iot_board_get_handle(BOARD_BTN_MN_ID), BUTTON_DOUBLE_CLICK, button_menu_double_click_cb);
    iot_board_button_register_cb(iot_board_get_handle(BOARD_BTN_MN_ID), BUTTON_LONG_PRESS_HOLD, button_menu_long_press_cb);

    if (esp_reset_reason() == ESP_RST_SW && s_driver_index > 0 && s_driver_index < _app_driver_count) {
        ESP_LOGI(TAG, "Restart to run APP: %s",  _app_driver[s_active_driver_index].app_name);
    } else {
        s_driver_index = 0;
    }
    _app_driver[s_driver_index].init();
    s_active_driver_index = s_driver_index;

    bool led_state = false;
    while(1) {
        iot_board_led_all_set_state(led_state);
        led_state = !led_state;
        vTaskDelay(250 / portTICK_PERIOD_MS);
    }
}
