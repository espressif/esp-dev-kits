// Copyright 2020-2021 Espressif Systems (Shanghai) PTE LTD
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

#pragma once

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

typedef struct {
    char* app_name;
    char* icon_name;
    void (* init)(void);
    void (* deinit)(void);
    void (* hide)(void);
    void (* show)(void);
    QueueHandle_t *p_queue_hdl;
    struct {
        uint16_t restart_before_init: 1;
        uint16_t restart_after_deinit: 1;
    } flags;
} user_app_t;

#define TASK_APP_PRIO_MAX (configMAX_PRIORITIES-10)
#define TASK_APP_PRIO_MIN 1

extern void usb_camera_init(void);
extern void usb_camera_deinit(void);
extern void avi_player_init(void);
extern void keyboard_init(void);
extern void app_menu_init(void);
extern void app_menu_deinit(void);
extern void app_manual_init(void);
extern void app_manual_deinit(void);
extern void usb_wireless_disk_init(void);
extern void usb_wireless_disk_deinit(void);
extern void usb_hid_mouse_init(void);
extern void usb_hid_mouse_deinit(void);
extern void usb_hid_keyboard_init(void);
extern void usb_hid_keyboard_deinit(void);
extern QueueHandle_t g_usb_camera_queue_hdl;
extern QueueHandle_t g_app_menu_queue_hdl;
extern QueueHandle_t g_app_manual_queue_hdl;
extern QueueHandle_t g_disk_queue_hdl;
extern QueueHandle_t g_usb_hid_mouse_hdl;
extern QueueHandle_t g_usb_hid_keyboard_hdl;

typedef enum {
    BTN_CLICK_MENU = 0,
    BTN_DOUBLE_CLICK_MENU,
    BTN_LONG_PRESS_MENU,
    BTN_CLICK_UP,
    BTN_CLICK_DOWN,
    BTN_CLICK_OK,
    USR_RESERVED_1,
    USR_RESERVED_2
} hmi_event_id_t;

typedef struct {
    hmi_event_id_t id;
} hmi_event_t;

static user_app_t const _app_driver[] =
{
    {
        .app_name = "app menu",
        .icon_name = "esp_logo.jpg",
        .init = app_menu_init,
        .deinit = app_menu_deinit,
        .p_queue_hdl = &g_app_menu_queue_hdl,
    },
    {
        .app_name = "USB Wireless Disk",
        .icon_name = "icon_flash.jpg",
        .init = usb_wireless_disk_init,
        .deinit = usb_wireless_disk_deinit,
        .p_queue_hdl = &g_disk_queue_hdl,
        .flags.restart_after_deinit = true,
    },
    {
        .app_name = "USB Camera",
        .icon_name = "icon_camera.jpg",
        .init = usb_camera_init,
        .deinit = usb_camera_deinit,
        .p_queue_hdl = &g_usb_camera_queue_hdl,
        // .flags.restart_before_init = false,
        // .flags.restart_after_deinit = false,
    },
    {
        .app_name = "Mouse Device",
        .icon_name = "icon_mouse.jpg",
        .init = usb_hid_mouse_init,
        .deinit = usb_hid_mouse_deinit,
        .p_queue_hdl = &g_usb_hid_mouse_hdl,
        .flags.restart_after_deinit = true,
    },
    {
        .app_name = "Keyboard Device",
        .icon_name = "icon_keyboard.jpg",
        .init = usb_hid_keyboard_init,
        .deinit = usb_hid_keyboard_deinit,
        .p_queue_hdl = &g_usb_hid_keyboard_hdl,
        .flags.restart_after_deinit = true,
    },
    {
        .app_name = "app manual",
        .icon_name = "icon_usermanual.jpg",
        .init = app_manual_init,
        .deinit = app_manual_deinit,
        .p_queue_hdl = &g_app_manual_queue_hdl,
    },

};

const static int _app_driver_count = sizeof(_app_driver) / sizeof(user_app_t);