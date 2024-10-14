/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cmath>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_wifi.h"
#include "esp_check.h"
#include "esp_memory_utils.h"
#include "esp_mac.h"
#include "bsp/esp-bsp.h"
#include "bsp_board_extra.h"
#include "nvs.h"

#include "ui/ui.h"
#include "Setting.hpp"
#include "app_sntp.h"

#include "esp_brookesia_versions.h"

#define ENABLE_DEBUG_LOG                (0)

#define HOME_REFRESH_TASK_STACK_SIZE    (1024 * 4)
#define HOME_REFRESH_TASK_PRIORITY      (1)
#define HOME_REFRESH_TASK_PERIOD_MS     (2000)

#define WIFI_SCAN_TASK_STACK_SIZE       (1024 * 6)
#define WIFI_SCAN_TASK_PRIORITY         (1)
#define WIFI_SCAN_TASK_PERIOD_MS        (5 * 1000)

#define WIFI_CONNECT_TASK_STACK_SIZE    (1024 * 4)
#define WIFI_CONNECT_TASK_PRIORITY      (4)
#define WIFI_CONNECT_TASK_STACK_CORE    (0)
#define WIFI_CONNECT_UI_WAIT_TIME_MS    (1 * 1000)
#define WIFI_CONNECT_UI_PANEL_SIZE      (1 * 1000)
#define WIFI_CONNECT_RET_WAIT_TIME_MS   (10 * 1000)

#define SCREEN_BRIGHTNESS_MIN           (20)
#define SCREEN_BRIGHTNESS_MAX           (BSP_LCD_BACKLIGHT_BRIGHTNESS_MAX)

#define SPEAKER_VOLUME_MIN              (0)
#define SPEAKER_VOLUME_MAX              (100)

#define NVS_STORAGE_NAMESPACE           "storage"
#define NVS_KEY_WIFI_ENABLE             "wifi_en"
#define NVS_KEY_BLE_ENABLE              "ble_en"
#define NVS_KEY_AUDIO_VOLUME            "volume"
#define NVS_KEY_DISPLAY_BRIGHTNESS      "brightness"

#define UI_MAIN_ITEM_LEFT_OFFSET        (20)
#define UI_WIFI_LIST_UP_OFFSET          (20)
#define UI_WIFI_LIST_UP_PAD             (20)
#define UI_WIFI_LIST_DOWN_PAD           (20)
#define UI_WIFI_LIST_H_PERCENT          (75)
#define UI_WIFI_LIST_ITEM_H             (60)
#define UI_WIFI_LIST_ITEM_FONT          (&lv_font_montserrat_26)
#define UI_WIFI_KEYBOARD_H_PERCENT      (30)
#define UI_WIFI_ICON_LOCK_RIGHT_OFFSET       (-10)
#define UI_WIFI_ICON_SIGNAL_RIGHT_OFFSET     (-50)
#define UI_WIFI_ICON_CONNECT_RIGHT_OFFSET    (-90)

using namespace std;

#define SCAN_LIST_SIZE      25

static const char TAG[] = "EUI_Setting";

TaskHandle_t wifi_scan_handle_task;

static EventGroupHandle_t s_wifi_event_group;

static char st_wifi_ssid[32];
static char st_wifi_password[64];

static uint8_t base_mac_addr[6] = {0};
static char mac_str[18] = {0};

static lv_obj_t* panel_wifi_btn[SCAN_LIST_SIZE];
static lv_obj_t* label_wifi_ssid[SCAN_LIST_SIZE];
static lv_obj_t* img_img_wifi_lock[SCAN_LIST_SIZE];
static lv_obj_t* wifi_image[SCAN_LIST_SIZE];
static lv_obj_t* wifi_connect[SCAN_LIST_SIZE];

static int brightness;

LV_IMG_DECLARE(img_wifisignal_absent);
LV_IMG_DECLARE(img_wifisignal_wake);
LV_IMG_DECLARE(img_wifisignal_moderate);
LV_IMG_DECLARE(img_wifisignal_good);
LV_IMG_DECLARE(img_wifi_lock);
LV_IMG_DECLARE(img_wifi_connect_success);
LV_IMG_DECLARE(img_wifi_connect_fail);

typedef enum {
    WIFI_EVENT_CONNECTED = BIT(0),
    WIFI_EVENT_INIT_DONE = BIT(1),
    WIFI_EVENT_UI_INIT_DONE = BIT(2),
    WIFI_EVENT_SCANING = BIT(3)
} wifi_event_id_t;

LV_IMG_DECLARE(img_app_setting);
extern lv_obj_t *ui_Min;
extern lv_obj_t *ui_Hour;
extern lv_obj_t *ui_Sec;
extern lv_obj_t *ui_Date;
extern lv_obj_t *ui_Clock_Number;

AppSettings::AppSettings():
    ESP_Brookesia_PhoneApp("Settings", &img_app_setting, false),                  // auto_resize_visual_area
    _is_ui_resumed(false),
    _is_ui_del(true),
    _screen_index(UI_MAIN_SETTING_INDEX),
    _screen_list({nullptr})
{
}

AppSettings::~AppSettings()
{
}

bool AppSettings::run(void)
{
    _is_ui_del = false;

    // Initialize Squareline UI
    ui_setting_init();

    // Get MAC
    esp_read_mac(base_mac_addr, ESP_MAC_EFUSE_FACTORY);
    snprintf(mac_str, sizeof(mac_str), "%02X-%02X-%02X-%02X-%02X-%02X",
             base_mac_addr[0], base_mac_addr[1], base_mac_addr[2],
             base_mac_addr[3], base_mac_addr[4], base_mac_addr[5]);


    // Initialize custom UI
    extraUiInit();

    // Upate UI by NVS parametres
    updateUiByNvsParam();

    xEventGroupSetBits(s_wifi_event_group, WIFI_EVENT_UI_INIT_DONE);

    return true;
}

bool AppSettings::back(void)
{
    _is_ui_resumed = false;

    if (_screen_index == UI_WIFI_CONNECT_INDEX) {
        lv_scr_load(ui_ScreenSettingWiFi);
    } else if (_screen_index != UI_MAIN_SETTING_INDEX) {
        lv_scr_load(ui_ScreenSettingMain);
    } else {
        while(xEventGroupGetBits(s_wifi_event_group) & WIFI_EVENT_SCANING) {
            ESP_LOGI(TAG, "WiFi is scanning, please wait");
            vTaskDelay(pdMS_TO_TICKS(100));
            stopWifiScan();
        } 
        notifyCoreClosed();
    }

    return true;
}

bool AppSettings::close(void)
{
    while(xEventGroupGetBits(s_wifi_event_group) & WIFI_EVENT_SCANING) {
        ESP_LOGI(TAG, "WiFi is scanning, please wait");
        vTaskDelay(pdMS_TO_TICKS(100));
        stopWifiScan();
    } 
    
    _is_ui_del = true;
    
    return true;
}

bool AppSettings::init(void)
{
    ESP_Brookesia_Phone *phone = getPhone();
    ESP_Brookesia_PhoneHome& home = phone->getHome();
    status_bar = home.getStatusBar();
    backstage = home.getRecentsScreen();

    // Initialize NVS parameters
    _nvs_param_map[NVS_KEY_WIFI_ENABLE] = false;
    _nvs_param_map[NVS_KEY_BLE_ENABLE] = false;
    _nvs_param_map[NVS_KEY_AUDIO_VOLUME] = bsp_extra_codec_volume_get();
    _nvs_param_map[NVS_KEY_AUDIO_VOLUME] = max(min((int)_nvs_param_map[NVS_KEY_AUDIO_VOLUME], SPEAKER_VOLUME_MAX), SPEAKER_VOLUME_MIN);
    // _nvs_param_map[NVS_KEY_DISPLAY_BRIGHTNESS] = bsp_display_brightness_get();
    _nvs_param_map[NVS_KEY_DISPLAY_BRIGHTNESS] = brightness;
    _nvs_param_map[NVS_KEY_DISPLAY_BRIGHTNESS] = max(min((int)_nvs_param_map[NVS_KEY_DISPLAY_BRIGHTNESS], SCREEN_BRIGHTNESS_MAX), SCREEN_BRIGHTNESS_MIN);
    // Load NVS parameters if exist
    loadNvsParam();
    // Update System parameters
    bsp_extra_codec_volume_set(_nvs_param_map[NVS_KEY_AUDIO_VOLUME], (int *)&_nvs_param_map[NVS_KEY_AUDIO_VOLUME]);
    bsp_display_brightness_set(_nvs_param_map[NVS_KEY_DISPLAY_BRIGHTNESS]);

    xTaskCreate(euiRefresTask, "Home Refresh", HOME_REFRESH_TASK_STACK_SIZE, this, HOME_REFRESH_TASK_PRIORITY, NULL);
    xTaskCreate(wifiScanTask, "WiFi Scan", WIFI_SCAN_TASK_STACK_SIZE, this, WIFI_SCAN_TASK_PRIORITY, NULL);

    return true;
}

bool AppSettings::pause(void)
{
    _is_ui_resumed = true;

    return true;
}

bool AppSettings::resume(void)
{
    _is_ui_resumed = false;

    return true;
}

void AppSettings::extraUiInit(void)
{
    /* Main */
    lv_label_set_text(ui_LabelPanelSettingMainContainer3Volume, "Audio");
    lv_label_set_text(ui_LabelPanelSettingMainContainer4Light, "Display");
    lv_obj_align_to(ui_LabelPanelSettingMainContainer1WiFi, ui_ImagePanelSettingMainContainer1WiFi, LV_ALIGN_OUT_RIGHT_MID,
                    UI_MAIN_ITEM_LEFT_OFFSET, 0);
    lv_obj_align_to(ui_LabelPanelSettingMainContainer2Blue, ui_ImagePanelSettingMainContainer2Blue, LV_ALIGN_OUT_RIGHT_MID,
                    UI_MAIN_ITEM_LEFT_OFFSET, 0);
    lv_obj_align_to(ui_LabelPanelSettingMainContainer3Volume, ui_ImagePanelSettingMainContainer3Volume, LV_ALIGN_OUT_RIGHT_MID,
                    UI_MAIN_ITEM_LEFT_OFFSET, 0);
    lv_obj_align_to(ui_LabelPanelSettingMainContainer4Light, ui_ImagePanelSettingMainContainer4Light, LV_ALIGN_OUT_RIGHT_MID,
                    UI_MAIN_ITEM_LEFT_OFFSET, 0);
    lv_obj_align_to(ui_LabelPanelSettingMainContainer5About, ui_ImagePanelSettingMainContainer5About, LV_ALIGN_OUT_RIGHT_MID,
                    UI_MAIN_ITEM_LEFT_OFFSET, 0);
    // Record the screen index and install the screen loaded event callback
    _screen_list[UI_MAIN_SETTING_INDEX] = ui_ScreenSettingMain;
    lv_obj_add_event_cb(ui_ScreenSettingMain, onScreenLoadEventCallback, LV_EVENT_SCREEN_LOADED, this);

    /* WiFi */
    // Switch
    lv_obj_add_event_cb(ui_SwitchPanelScreenSettingWiFiSwitch, onSwitchPanelScreenSettingWiFiSwitchValueChangeEventCallback,
                        LV_EVENT_VALUE_CHANGED, this);
    // List
    // lv_obj_clear_flag(ui_PanelScreenSettingWiFiList, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(ui_PanelScreenSettingWiFiList, LV_DIR_VER);
    lv_obj_set_height(ui_PanelScreenSettingWiFiList, lv_pct(UI_WIFI_LIST_H_PERCENT));
    lv_obj_align_to(ui_PanelScreenSettingWiFiList, ui_PanelScreenSettingWiFiSwitch, LV_ALIGN_OUT_BOTTOM_MID, 0,
                    UI_WIFI_LIST_UP_OFFSET);
    lv_obj_set_style_pad_all(ui_PanelScreenSettingWiFiList, 0, 0);
    lv_obj_set_style_pad_top(ui_PanelScreenSettingWiFiList, UI_WIFI_LIST_UP_PAD, 0);
    lv_obj_set_style_pad_bottom(ui_PanelScreenSettingWiFiList, UI_WIFI_LIST_DOWN_PAD, 0);
    for(int i = 0; i < SCAN_LIST_SIZE; i++) {
        panel_wifi_btn[i] = lv_obj_create(ui_PanelScreenSettingWiFiList);
        lv_obj_set_size(panel_wifi_btn[i], lv_pct(100), UI_WIFI_LIST_ITEM_H);
        lv_obj_set_style_radius(panel_wifi_btn[i], 0, 0);
        lv_obj_set_style_border_width(panel_wifi_btn[i], 0, 0);
        lv_obj_set_style_text_font(panel_wifi_btn[i], UI_WIFI_LIST_ITEM_FONT, 0);
        lv_obj_add_flag(panel_wifi_btn[i], LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag( panel_wifi_btn[i], LV_OBJ_FLAG_SCROLLABLE );
        lv_obj_set_style_bg_color(panel_wifi_btn[i], lv_color_hex(0xCBCBCB), LV_PART_MAIN | LV_STATE_PRESSED );
        lv_obj_set_style_bg_opa(panel_wifi_btn[i], 255, LV_PART_MAIN| LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(panel_wifi_btn[i], lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT );
        lv_obj_set_style_border_opa(panel_wifi_btn[i], 255, LV_PART_MAIN| LV_STATE_DEFAULT);

        label_wifi_ssid[i] = lv_label_create(panel_wifi_btn[i]);
        lv_obj_set_align(label_wifi_ssid[i], LV_ALIGN_LEFT_MID);

        img_img_wifi_lock[i] = lv_img_create(panel_wifi_btn[i]);
        lv_obj_align(img_img_wifi_lock[i], LV_ALIGN_RIGHT_MID, UI_WIFI_ICON_LOCK_RIGHT_OFFSET, 0);
        lv_obj_add_flag(img_img_wifi_lock[i], LV_OBJ_FLAG_HIDDEN);

        wifi_image[i] = lv_img_create(panel_wifi_btn[i]);
        lv_obj_align(wifi_image[i], LV_ALIGN_RIGHT_MID, UI_WIFI_ICON_SIGNAL_RIGHT_OFFSET, 0);

        wifi_connect[i] = lv_label_create(panel_wifi_btn[i]);
        lv_label_set_text(wifi_connect[i], LV_SYMBOL_OK);
        lv_obj_align(wifi_connect[i], LV_ALIGN_RIGHT_MID, UI_WIFI_ICON_CONNECT_RIGHT_OFFSET, 0);
        lv_obj_add_flag(wifi_connect[i], LV_OBJ_FLAG_HIDDEN);

        lv_obj_add_event_cb(panel_wifi_btn[i], onButtonWifiListClickedEventCallback, LV_EVENT_CLICKED, (void*)label_wifi_ssid[i]);
        if(!(xEventGroupGetBits(s_wifi_event_group) & WIFI_EVENT_SCANING)) {
            lv_obj_add_flag(ui_PanelScreenSettingWiFiList, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(ui_SpinnerScreenSettingWiFi, LV_OBJ_FLAG_HIDDEN);
        }
    }
    lv_obj_add_flag(ui_ButtonScreenSettingWiFiReturn, LV_OBJ_FLAG_HIDDEN);
    // Connect
    lv_obj_add_flag(ui_SpinnerScreenSettingVerification, LV_OBJ_FLAG_HIDDEN);
    _panel_wifi_connect = lv_obj_create(ui_ScreenSettingVerification);
    lv_obj_set_size(_panel_wifi_connect, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(_panel_wifi_connect, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(_panel_wifi_connect, LV_OPA_50, 0);
    lv_obj_center(_panel_wifi_connect);
    _img_wifi_connect = lv_img_create(_panel_wifi_connect);
    lv_obj_center(_img_wifi_connect);
    _spinner_wifi_connect = lv_spinner_create(_panel_wifi_connect, 1000, 600);
    lv_obj_set_size(_spinner_wifi_connect, lv_pct(20), lv_pct(20));
    lv_obj_center(_spinner_wifi_connect);
    processWifiConnect(WIFI_CONNECT_HIDE);
    // Keyboard
    lv_textarea_set_password_mode(ui_TextAreaScreenSettingVerificationPassword, true);
    // lv_obj_set_size(ui_KeyboardScreenSettingVerification, lv_pct(100), lv_pct(UI_WIFI_KEYBOARD_H_PERCENT));
    // lv_obj_align(ui_KeyboardScreenSettingVerification, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_event_cb(ui_KeyboardScreenSettingVerification, onKeyboardScreenSettingVerificationClickedEventCallback,
                        LV_EVENT_CLICKED, this);
    // Record the screen index and install the screen loaded event callback
    lv_obj_add_flag(ui_ButtonScreenSettingBLEReturn, LV_OBJ_FLAG_HIDDEN);
    _screen_list[UI_WIFI_SCAN_INDEX] = ui_ScreenSettingWiFi;
    lv_obj_add_event_cb(ui_ScreenSettingWiFi, onScreenLoadEventCallback, LV_EVENT_SCREEN_LOADED, this);
    _screen_list[UI_WIFI_CONNECT_INDEX] = ui_ScreenSettingVerification;
    lv_obj_add_event_cb(ui_ScreenSettingVerification, onScreenLoadEventCallback, LV_EVENT_SCREEN_LOADED, this);

    /* Bluetooth */
    lv_obj_add_event_cb(ui_SwitchPanelScreenSettingBLESwitch, onSwitchPanelScreenSettingBLESwitchValueChangeEventCallback,
                        LV_EVENT_VALUE_CHANGED, this);
    // Record the screen index and install the screen loaded event callback
    _screen_list[UI_BLUETOOTH_SETTING_INDEX] = ui_ScreenSettingBLE;
    lv_obj_add_event_cb(ui_ScreenSettingBLE, onScreenLoadEventCallback, LV_EVENT_SCREEN_LOADED, this);

    /* Display */
    lv_slider_set_range(ui_SliderPanelScreenSettingLightSwitch1, SCREEN_BRIGHTNESS_MIN, SCREEN_BRIGHTNESS_MAX);
    lv_obj_add_event_cb(ui_SliderPanelScreenSettingLightSwitch1, onSliderPanelLightSwitchValueChangeEventCallback,
                        LV_EVENT_VALUE_CHANGED, this);
    lv_obj_add_flag(ui_ButtonScreenSettingLightReturn, LV_OBJ_FLAG_HIDDEN);
    // Record the screen index and install the screen loaded event callback
    _screen_list[UI_BRIGHTNESS_SETTING_INDEX] = ui_ScreenSettingLight;
    lv_obj_add_event_cb(ui_ScreenSettingLight, onScreenLoadEventCallback, LV_EVENT_SCREEN_LOADED, this);

    /* Audio */
    lv_slider_set_range(ui_SliderPanelScreenSettingVolumeSwitch, SPEAKER_VOLUME_MIN, SPEAKER_VOLUME_MAX);
    lv_obj_add_event_cb(ui_SliderPanelScreenSettingVolumeSwitch, onSliderPanelVolumeSwitchValueChangeEventCallback,
                        LV_EVENT_VALUE_CHANGED, this);
    lv_obj_add_flag(ui_ButtonScreenSettingVolumeReturn, LV_OBJ_FLAG_HIDDEN);
    // Record the screen index and install the screen loaded event callback
    _screen_list[UI_VOLUME_SETTING_INDEX] = ui_ScreenSettingVolume;
    lv_obj_add_event_cb(ui_ScreenSettingVolume, onScreenLoadEventCallback, LV_EVENT_SCREEN_LOADED, this);

    /* About */
    lv_label_set_text(ui_LabelPanelPanelScreenSettingAbout4, "ESP_Brookesia");
    lv_obj_add_flag(ui_ButtonScreenSettingAboutReturn, LV_OBJ_FLAG_HIDDEN);
    // Record the screen index and install the screen loaded event callback
    _screen_list[UI_ABOUT_SETTING_INDEX] = ui_ScreenSettingAbout;
    lv_obj_add_event_cb(ui_ScreenSettingAbout, onScreenLoadEventCallback, LV_EVENT_SCREEN_LOADED, this);

    lv_obj_add_flag(ui_PanelSettingMainContainerItem2, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(ui_LabelPanelPanelScreenSettingAbout3, mac_str);
    lv_label_set_text(ui_LabelPanelPanelScreenSettingAbout5, "v0.2.0");
    lv_label_set_text(ui_LabelPanelPanelScreenSettingAbout2, "ESP32-P4-Function-EV-Board");
    lv_obj_set_x( ui_LabelPanelPanelScreenSettingAbout2, 167 );

    char char_ui_version[20];
    snprintf(char_ui_version, sizeof(char_ui_version), "v%d.%d.%d", ESP_BROOKESIA_CONF_VER_MAJOR, ESP_BROOKESIA_CONF_VER_MINOR, ESP_BROOKESIA_CONF_VER_PATCH);
    lv_label_set_text(ui_LabelPanelPanelScreenSettingAbout6, char_ui_version);
}

void AppSettings::processWifiConnect(WifiConnectState_t state)
{
    switch (state) {
    case WIFI_CONNECT_HIDE:
        lv_obj_add_flag(_panel_wifi_connect, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(_img_wifi_connect, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(_spinner_wifi_connect, LV_OBJ_FLAG_HIDDEN);
        break;
    case WIFI_CONNECT_RUNNING:
        lv_obj_clear_flag(_panel_wifi_connect, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(_img_wifi_connect, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(_spinner_wifi_connect, LV_OBJ_FLAG_HIDDEN);
        break;
    case WIFI_CONNECT_SUCCESS:
        lv_obj_clear_flag(_panel_wifi_connect, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(_img_wifi_connect, LV_OBJ_FLAG_HIDDEN);
        lv_img_set_src(_img_wifi_connect, &img_wifi_connect_success);
        lv_obj_add_flag(_spinner_wifi_connect, LV_OBJ_FLAG_HIDDEN);
        break;
    case WIFI_CONNECT_FAIL:
        lv_obj_clear_flag(_panel_wifi_connect, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(_img_wifi_connect, LV_OBJ_FLAG_HIDDEN);
        lv_img_set_src(_img_wifi_connect, &img_wifi_connect_fail);
        lv_obj_add_flag(_spinner_wifi_connect, LV_OBJ_FLAG_HIDDEN);
        break;
    default:
        break;
    }
}

bool AppSettings::loadNvsParam(void)
{
    esp_err_t err = ESP_OK;
    nvs_handle_t nvs_handle;
    err = nvs_open(NVS_STORAGE_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return false;
    }

    for (auto& key_value : _nvs_param_map) {
        err = nvs_get_i32(nvs_handle, key_value.first.c_str(), &key_value.second);
        switch (err) {
        case ESP_OK:
            ESP_LOGI(TAG, "Load %s: %d", key_value.first.c_str(), key_value.second);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            err = nvs_set_i32(nvs_handle, key_value.first.c_str(), key_value.second);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Error (%s) setting %s", esp_err_to_name(err), key_value.first.c_str());
            }
            ESP_LOGW(TAG, "The value of %s is not initialized yet, set it to default value: %d", key_value.first.c_str(),
                     key_value.second);
            break;
        default:
            break;
        }
    }

    err = nvs_commit(nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) committing NVS changes", esp_err_to_name(err));
        return false;
    }
    nvs_close(nvs_handle);

    return true;
}

bool AppSettings::setNvsParam(std::string key, int value)
{
    esp_err_t err = ESP_OK;
    nvs_handle_t nvs_handle;
    err = nvs_open(NVS_STORAGE_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return false;
    }

    err = nvs_set_i32(nvs_handle, key.c_str(), value);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) setting %s", esp_err_to_name(err), key.c_str());
    }

    err = nvs_commit(nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) committing NVS changes", esp_err_to_name(err));
        return false;
    }
    nvs_close(nvs_handle);

    return true;
}

void AppSettings::updateUiByNvsParam(void)
{
    if (_nvs_param_map[NVS_KEY_WIFI_ENABLE]) {
        lv_obj_add_state(ui_SwitchPanelScreenSettingWiFiSwitch, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(ui_SwitchPanelScreenSettingWiFiSwitch, LV_STATE_CHECKED);
    }

    if (_nvs_param_map[NVS_KEY_BLE_ENABLE]) {
        lv_obj_add_state(ui_SwitchPanelScreenSettingBLESwitch, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(ui_SwitchPanelScreenSettingBLESwitch, LV_STATE_CHECKED);
    }

    lv_slider_set_value(ui_SliderPanelScreenSettingLightSwitch1, _nvs_param_map[NVS_KEY_DISPLAY_BRIGHTNESS], LV_ANIM_OFF);
    lv_slider_set_value(ui_SliderPanelScreenSettingVolumeSwitch, _nvs_param_map[NVS_KEY_AUDIO_VOLUME], LV_ANIM_OFF);
}

esp_err_t AppSettings::initWifi()
{
    s_wifi_event_group = xEventGroupCreate();
    xEventGroupClearBits(s_wifi_event_group, WIFI_EVENT_CONNECTED);
    xEventGroupClearBits(s_wifi_event_group, WIFI_EVENT_INIT_DONE);
    xEventGroupClearBits(s_wifi_event_group, WIFI_EVENT_SCANING);
    if(!(xEventGroupGetBits(s_wifi_event_group) & WIFI_EVENT_UI_INIT_DONE)) {
        xEventGroupClearBits(s_wifi_event_group, WIFI_EVENT_UI_INIT_DONE);
    }

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifiEventHandler,
                                                        this,
                                                        &instance_any_id));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    return ESP_OK;
}

void AppSettings::startWifiScan(void)
{
    ESP_LOGI(TAG, "Start Wi-Fi scan");
    xEventGroupSetBits(s_wifi_event_group, WIFI_EVENT_SCANING);
    lv_obj_clear_flag(ui_SpinnerScreenSettingWiFi, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(ui_SwitchPanelScreenSettingWiFiSwitch, LV_OBJ_FLAG_CLICKABLE);
}

void AppSettings::stopWifiScan(void)
{
    ESP_LOGI(TAG, "Stop Wi-Fi scan");
    xEventGroupClearBits(s_wifi_event_group, WIFI_EVENT_SCANING);
    lv_obj_add_flag(ui_PanelScreenSettingWiFiList, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui_SpinnerScreenSettingWiFi, LV_OBJ_FLAG_HIDDEN);
    deinitWifiListButton();
}

void AppSettings::scanWifiAndUpdateUi(void)
{
    bool psk_flag = false;

    uint16_t number = SCAN_LIST_SIZE;
    wifi_ap_record_t ap_info[SCAN_LIST_SIZE];
    uint16_t ap_count = 0;
    memset(ap_info, 0, sizeof(ap_info));

    esp_wifi_start();
    esp_wifi_scan_start(NULL, true);
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
#if ENABLE_DEBUG_LOG
    ESP_LOGI(TAG, "Total APs scanned = %u", ap_count);
#endif

    bsp_display_lock(0);
    if(xEventGroupGetBits(s_wifi_event_group) & WIFI_EVENT_SCANING) {
        deinitWifiListButton();
    }
    bsp_display_unlock();

    for (int i = 0; (i < SCAN_LIST_SIZE) && (i < ap_count); i++) {
#if ENABLE_DEBUG_LOG
        ESP_LOGI(TAG, "SSID \t\t%s", ap_info[i].ssid);
        ESP_LOGI(TAG, "RSSI \t\t%d", ap_info[i].rssi);
        ESP_LOGI(TAG, "Channel \t\t%d", ap_info[i].primary);
#endif

        if(ap_info[i].authmode != WIFI_AUTH_OPEN && ap_info[i].authmode != WIFI_AUTH_OWE) {
            psk_flag = true;
        }
#if ENABLE_DEBUG_LOG
        ESP_LOGI(TAG, "psk_flag: %d", psk_flag);
#endif

        if(ap_info[i].rssi > -100 && ap_info[i].rssi <= -80) {
            _wifi_signal_strength_level = WIFI_SIGNAL_STRENGTH_WEAK;
        } else if(ap_info[i].rssi > -80 && ap_info[i].rssi <= -60) {
            _wifi_signal_strength_level = WIFI_SIGNAL_STRENGTH_MODERATE;
        } else if(ap_info[i].rssi > -60) {
            _wifi_signal_strength_level = WIFI_SIGNAL_STRENGTH_GOOD;
        } else {
            _wifi_signal_strength_level = WIFI_SIGNAL_STRENGTH_NONE;
        }
#if ENABLE_DEBUG_LOG
        ESP_LOGI(TAG, "signal_strength: %d", _wifi_signal_strength_level);
#endif

        bsp_display_lock(0);
        if(xEventGroupGetBits(s_wifi_event_group) & WIFI_EVENT_SCANING) {
            initWifiListButton(label_wifi_ssid[i], img_img_wifi_lock[i], wifi_image[i], wifi_connect[i],
                                ap_info[i].ssid, psk_flag, _wifi_signal_strength_level);     
        }
        bsp_display_unlock();
    }
}

void AppSettings::initWifiListButton(lv_obj_t* lv_label_ssid, lv_obj_t* lv_img_wifi_lock, lv_obj_t* lv_wifi_img,
                                     lv_obj_t *lv_wifi_connect, uint8_t* ssid, bool psk, WifiSignalStrengthLevel_t signal_strength)
{
    lv_label_set_text_fmt(lv_label_ssid, "%s", (const char*)ssid);

    if (strcmp((const char*)ssid, (const char*)st_wifi_ssid) == 0) {
        lv_obj_clear_flag(lv_wifi_connect, LV_OBJ_FLAG_HIDDEN);
    }

    if(psk) {
        lv_img_set_src(lv_img_wifi_lock, &img_wifi_lock);
        lv_obj_clear_flag(lv_img_wifi_lock, LV_OBJ_FLAG_HIDDEN);
    }

    if (signal_strength == WIFI_SIGNAL_STRENGTH_GOOD) {
        lv_img_set_src(lv_wifi_img, &img_wifisignal_good);
    } else if (signal_strength == WIFI_SIGNAL_STRENGTH_MODERATE) {
        lv_img_set_src(lv_wifi_img, &img_wifisignal_moderate);
    } else if (signal_strength == WIFI_SIGNAL_STRENGTH_WEAK) {
        lv_img_set_src(lv_wifi_img, &img_wifisignal_wake);
    } else {
        lv_img_set_src(lv_wifi_img, &img_wifisignal_absent);
    }
}

void AppSettings::deinitWifiListButton(void)
{
    for (int i = 0; i < SCAN_LIST_SIZE; i++) {
        lv_obj_add_flag(img_img_wifi_lock[i], LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(wifi_connect[i], LV_OBJ_FLAG_HIDDEN);
    }
}

void AppSettings::euiRefresTask(void *arg)
{
    AppSettings *app = (AppSettings *)arg;
    time_t now;
    struct tm timeinfo;
    bool is_time_pm = false;
    // char textBuf[50];
    uint16_t free_sram_size_kb = 0;
    uint16_t total_sram_size_kb = 0;
    uint16_t free_psram_size_kb = 0;
    uint16_t total_psram_size_kb = 0;

    if (app == NULL) {
        ESP_LOGE(TAG, "App instance is NULL");
        goto err;
    }

    while (1) {
        /* Update status bar */
        // time
        time(&now);
        localtime_r(&now, &timeinfo);
        is_time_pm = (timeinfo.tm_hour >= 12);

        bsp_display_lock(0);
        if(!app->status_bar->setClock(timeinfo.tm_hour, timeinfo.tm_min, is_time_pm)) {
            ESP_LOGE(TAG, "Set clock failed");
        }
        bsp_display_unlock();

        // Update WiFi icon state
        if((xEventGroupGetBits(s_wifi_event_group) & WIFI_EVENT_CONNECTED)) {
            app_sntp_init();

            bsp_display_lock(0);
            if(app->_wifi_signal_strength_level == WIFI_SIGNAL_STRENGTH_NONE) {
                app->status_bar->setWifiIconState(0);
            } else if(app->_wifi_signal_strength_level == WIFI_SIGNAL_STRENGTH_WEAK) {
                app->status_bar->setWifiIconState(1);
            } else if(app->_wifi_signal_strength_level == WIFI_SIGNAL_STRENGTH_MODERATE) {
                app->status_bar->setWifiIconState(2);
            } else if (app->_wifi_signal_strength_level == WIFI_SIGNAL_STRENGTH_GOOD) {
                app->status_bar->setWifiIconState(3);
            }
            bsp_display_unlock();
        }

        /* Updte Smart Gadget app */
        // app->updateGadgetTime(timeinfo);

        // Update memory in backstage
        if(app->backstage->checkVisible()) {
            free_sram_size_kb = heap_caps_get_free_size(MALLOC_CAP_INTERNAL) / 1024;
            total_sram_size_kb = heap_caps_get_total_size(MALLOC_CAP_INTERNAL) / 1024;
            free_psram_size_kb = heap_caps_get_free_size(MALLOC_CAP_SPIRAM) / 1024;
            total_psram_size_kb = heap_caps_get_total_size(MALLOC_CAP_SPIRAM) / 1024;
            ESP_LOGI(TAG, "Free sram size: %d KB, total sram size: %d KB, "
                        "free psram size: %d KB, total psram size: %d KB",
                        free_sram_size_kb, total_sram_size_kb, free_psram_size_kb, total_psram_size_kb);

            bsp_display_lock(0);
            if(!app->backstage->setMemoryLabel(free_sram_size_kb, total_sram_size_kb, free_psram_size_kb, total_psram_size_kb)) {
                ESP_LOGE(TAG, "Update memory usage failed");
            }
            bsp_display_unlock();
        }

        vTaskDelay(pdMS_TO_TICKS(HOME_REFRESH_TASK_PERIOD_MS));
    }

err:
    vTaskDelete(NULL);
}

void AppSettings::wifiScanTask(void *arg)
{
    AppSettings *app = (AppSettings *)arg;
    esp_err_t ret = ESP_OK;

    if (app == NULL) {
        ESP_LOGE(TAG, "App instance is NULL");
        goto err;
    }

    ret = app->initWifi();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Init Wi-Fi failed");
        goto err;
    }

    if (ret == ESP_OK) {
        xEventGroupSetBits(s_wifi_event_group, WIFI_EVENT_INIT_DONE);
        ESP_LOGI(TAG, "wifi_init done");
    } else {
        ESP_LOGE(TAG, "wifi_init failed");
    }

    while (true) {
        if((xEventGroupGetBits(s_wifi_event_group) & WIFI_EVENT_INIT_DONE) &&
           (xEventGroupGetBits(s_wifi_event_group) & WIFI_EVENT_UI_INIT_DONE)){
            lv_obj_add_flag(ui_SwitchPanelScreenSettingWiFiSwitch, LV_OBJ_FLAG_CLICKABLE);
            xEventGroupClearBits(s_wifi_event_group, WIFI_EVENT_INIT_DONE);
            xEventGroupClearBits(s_wifi_event_group, WIFI_EVENT_UI_INIT_DONE);
        }

        if(xEventGroupGetBits(s_wifi_event_group) & WIFI_EVENT_SCANING){
            app->scanWifiAndUpdateUi();
            vTaskDelay(pdMS_TO_TICKS(WIFI_SCAN_TASK_PERIOD_MS));
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }

err:
    vTaskDelete(NULL);
}

void AppSettings::wifiConnectTask(void *arg)
{
    AppSettings *app = (AppSettings *)arg;
    wifi_config_t wifi_config = { 0 };

    esp_wifi_disconnect();
    app->status_bar->setWifiIconState(0);

    memcpy(st_wifi_ssid, lv_label_get_text(ui_LabelScreenSettingVerificationSSID), sizeof(wifi_config.sta.ssid));
    memcpy(st_wifi_password, lv_textarea_get_text(ui_TextAreaScreenSettingVerificationPassword), sizeof(wifi_config.sta.ssid));

    memcpy(wifi_config.sta.ssid, st_wifi_ssid, sizeof(wifi_config.sta.ssid));
    memcpy(wifi_config.sta.password, st_wifi_password, sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );

    ESP_LOGI(TAG, "SSID:%s, password:%s.", wifi_config.sta.ssid, wifi_config.sta.password);
    esp_wifi_connect();

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_EVENT_CONNECTED,
            pdFALSE,
            pdFALSE,
            pdMS_TO_TICKS(WIFI_CONNECT_RET_WAIT_TIME_MS));

    if (bits & WIFI_EVENT_CONNECTED) {
        ESP_LOGI(TAG, "Connected successfully");

        if (!app->_is_ui_del) {
            bsp_display_lock(0);
            app->processWifiConnect(WIFI_CONNECT_SUCCESS);
            bsp_display_unlock();
        }

        vTaskDelay(pdMS_TO_TICKS(WIFI_CONNECT_UI_WAIT_TIME_MS));

        if (!app->_is_ui_del) {
            bsp_display_lock(0);
            app->processWifiConnect(WIFI_CONNECT_HIDE);
            // lv_obj_clear_flag(ui_KeyboardScreenSettingVerification, LV_OBJ_FLAG_HIDDEN);
            lv_textarea_set_text(ui_TextAreaScreenSettingVerificationPassword, "");
            app->back();
            bsp_display_unlock();
        }

        // app->updateGadgetTime(timeinfo);
    } else {
        ESP_LOGI(TAG, "Connect failed");

        if (!app->_is_ui_del) {
            bsp_display_lock(0);
            app->processWifiConnect(WIFI_CONNECT_FAIL);
            bsp_display_unlock();
        }

        vTaskDelay(pdMS_TO_TICKS(WIFI_CONNECT_UI_WAIT_TIME_MS));

        if (!app->_is_ui_del) {
            bsp_display_lock(0);
            app->processWifiConnect(WIFI_CONNECT_HIDE);
            // lv_obj_clear_flag(ui_KeyboardScreenSettingVerification, LV_OBJ_FLAG_HIDDEN);
            lv_textarea_set_text(ui_TextAreaScreenSettingVerificationPassword, "");
            // app->back();
            bsp_display_unlock();
        }
    }

    // if (!app->_is_ui_del) {
    //     xEventGroupSetBits(s_wifi_event_group, WIFI_EVENT_SCANING);
    //     app->startWifiScan();
    // }

    vTaskDelete(NULL);
}

void AppSettings::wifiEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    AppSettings *app = (AppSettings *)arg;

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
        xEventGroupSetBits(s_wifi_event_group, WIFI_EVENT_CONNECTED);
        ESP_LOGI(TAG, "connected to ap SSID:%s, password:%s.", st_wifi_ssid, st_wifi_password);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        xEventGroupClearBits(s_wifi_event_group, WIFI_EVENT_CONNECTED);
        ESP_LOGI(TAG, "disconnected from ap SSID:%s, password:%s.", st_wifi_ssid, st_wifi_password);
        memset(st_wifi_ssid, 0, sizeof(st_wifi_ssid));

        // app->back();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_SCAN_DONE) {
        if(lv_obj_has_flag(ui_PanelScreenSettingWiFiList, LV_OBJ_FLAG_HIDDEN) &&
           xEventGroupGetBits(s_wifi_event_group) & WIFI_EVENT_SCANING) {
            if (!app->_is_ui_del) {
                bsp_display_lock(0);
                lv_obj_clear_flag(ui_PanelScreenSettingWiFiList, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(ui_SpinnerScreenSettingWiFi, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(ui_SwitchPanelScreenSettingWiFiSwitch, LV_OBJ_FLAG_CLICKABLE);
                app->status_bar->setWifiIconState(0);
                bsp_display_unlock();
            }
        }
    }
}

void AppSettings::onKeyboardScreenSettingVerificationClickedEventCallback(lv_event_t *e)
{
    AppSettings *app = (AppSettings *)lv_event_get_user_data(e);
    lv_obj_t *target = lv_event_get_target(e);

    ESP_BROOKESIA_CHECK_NULL_GOTO(app, end, "Invalid app pointer");

    lv_keyboard_set_textarea(target, ui_TextAreaScreenSettingVerificationPassword);

    if(lv_keyboard_get_selected_btn(target) == 39) {
        app->processWifiConnect(WIFI_CONNECT_RUNNING);
        // lv_obj_add_flag(ui_KeyboardScreenSettingVerification, LV_OBJ_FLAG_HIDDEN);

        app->stopWifiScan();

        xTaskCreatePinnedToCore(wifiConnectTask, "wifi Connect", WIFI_CONNECT_TASK_STACK_SIZE, app,
                                WIFI_CONNECT_TASK_PRIORITY, NULL, WIFI_CONNECT_TASK_STACK_CORE);
    }

end:
    return;
}

void AppSettings::onScreenLoadEventCallback( lv_event_t * e)
{
    AppSettings *app = (AppSettings *)lv_event_get_user_data(e);
    SettingScreenIndex_t last_scr_index = app->_screen_index;

    ESP_BROOKESIA_CHECK_NULL_GOTO(app, end, "Invalid app pointer");

    for (int i = 0; i < UI_MAX_INDEX; i++) {
        if (app->_screen_list[i] == lv_event_get_target(e)) {
            app->_screen_index = (SettingScreenIndex_t)i;
            break;
        }
    }

    if (last_scr_index == UI_WIFI_SCAN_INDEX) {
        app->stopWifiScan();
    }

    if ((app->_screen_index == UI_WIFI_SCAN_INDEX) && (app->_nvs_param_map[NVS_KEY_WIFI_ENABLE] == true)) {
        app->startWifiScan();
    }

end:
    return;
}

void AppSettings::onSwitchPanelScreenSettingWiFiSwitchValueChangeEventCallback( lv_event_t * e) {
    lv_state_t state = lv_obj_get_state(ui_SwitchPanelScreenSettingWiFiSwitch);

    AppSettings *app = (AppSettings *)lv_event_get_user_data(e);
    ESP_BROOKESIA_CHECK_NULL_GOTO(app, end, "Invalid app pointer");

    if (state & LV_STATE_CHECKED) {
        app->_nvs_param_map[NVS_KEY_WIFI_ENABLE] = true;
        app->setNvsParam(NVS_KEY_WIFI_ENABLE, 1);
        if (app->_screen_index == UI_WIFI_SCAN_INDEX) {
            app->startWifiScan();
        }
    } else {
        app->_nvs_param_map[NVS_KEY_WIFI_ENABLE] = false;
        app->setNvsParam(NVS_KEY_WIFI_ENABLE, 0);
        if (app->_screen_index == UI_WIFI_SCAN_INDEX) {
            app->stopWifiScan();
            if (xEventGroupGetBits(s_wifi_event_group) & WIFI_EVENT_CONNECTED) {
                ESP_ERROR_CHECK(esp_wifi_disconnect());
                app->status_bar->setWifiIconState(0);
            }
        }
    }

end:
    return;
}

void AppSettings::onButtonWifiListClickedEventCallback(lv_event_t * e)
{
    lv_obj_t *label_wifi_ssid = (lv_obj_t*)lv_event_get_user_data(e);
    lv_obj_t *btn = lv_event_get_target(e);
    lv_area_t btn_click_area;
    lv_point_t point;

    lv_obj_get_click_area(btn, &btn_click_area);
    lv_indev_get_point(lv_indev_get_act(), &point);
    if ((point.x < btn_click_area.x1) || (point.x > btn_click_area.x2) ||
        (point.y < btn_click_area.y1) || (point.y > btn_click_area.y2)) {
        return;
    }

    lv_scr_load(ui_ScreenSettingVerification);
    lv_label_set_text_fmt(ui_LabelScreenSettingVerificationSSID, "%s", lv_label_get_text(label_wifi_ssid));

    xEventGroupClearBits(s_wifi_event_group, WIFI_EVENT_SCANING);

    esp_wifi_scan_stop();
}

void AppSettings::onSwitchPanelScreenSettingBLESwitchValueChangeEventCallback( lv_event_t * e) {
    lv_state_t state = lv_obj_get_state(ui_SwitchPanelScreenSettingBLESwitch);

    AppSettings *app = (AppSettings *)lv_event_get_user_data(e);
    ESP_BROOKESIA_CHECK_NULL_GOTO(app, end, "Invalid app pointer");

    if (state & LV_STATE_CHECKED) {
        app->_nvs_param_map[NVS_KEY_WIFI_ENABLE] = true;
        app->setNvsParam(NVS_KEY_BLE_ENABLE, 1);
    } else {
        app->_nvs_param_map[NVS_KEY_WIFI_ENABLE] = false;
        app->setNvsParam(NVS_KEY_BLE_ENABLE, 0);
    }

end:
    return;
}

void AppSettings::onSliderPanelVolumeSwitchValueChangeEventCallback( lv_event_t * e) {
    int volume = lv_slider_get_value(ui_SliderPanelScreenSettingVolumeSwitch);

    AppSettings *app = (AppSettings *)lv_event_get_user_data(e);
    ESP_BROOKESIA_CHECK_NULL_GOTO(app, end, "Invalid app pointer");

    if (volume != app->_nvs_param_map[NVS_KEY_AUDIO_VOLUME]) {
        if ((bsp_extra_codec_volume_set(volume, NULL) != ESP_OK) && (bsp_extra_codec_volume_get() != volume)) {
            ESP_LOGE(TAG, "Set volume failed");
            lv_slider_set_value(ui_SliderPanelScreenSettingVolumeSwitch, app->_nvs_param_map[NVS_KEY_AUDIO_VOLUME], LV_ANIM_OFF);
            return;
        }
        app->_nvs_param_map[NVS_KEY_AUDIO_VOLUME] = volume;
        app->setNvsParam(NVS_KEY_AUDIO_VOLUME, volume);
    }

end:
    return;
}

void AppSettings::onSliderPanelLightSwitchValueChangeEventCallback( lv_event_t * e) {
    brightness = lv_slider_get_value(ui_SliderPanelScreenSettingLightSwitch1);

    AppSettings *app = (AppSettings *)lv_event_get_user_data(e);
    ESP_BROOKESIA_CHECK_NULL_GOTO(app, end, "Invalid app pointer");

    if (brightness != app->_nvs_param_map[NVS_KEY_DISPLAY_BRIGHTNESS]) {
        // if ((bsp_display_brightness_set(brightness) != ESP_OK) && (bsp_display_brightness_get() != brightness)) {
        if (bsp_display_brightness_set(brightness) != ESP_OK) {
            ESP_LOGE(TAG, "Set brightness failed");
            lv_slider_set_value(ui_SliderPanelScreenSettingLightSwitch1, app->_nvs_param_map[NVS_KEY_DISPLAY_BRIGHTNESS], LV_ANIM_OFF);
            return;
        }
        app->_nvs_param_map[NVS_KEY_DISPLAY_BRIGHTNESS] = brightness;
        app->setNvsParam(NVS_KEY_DISPLAY_BRIGHTNESS, brightness);
    }

end:
    return;
}
