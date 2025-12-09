#include "ui_main.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "app_bluetooth.h"
#include "settings.h"

static const char *TAG = "ui_bluetooth";
#define LOG_TRACE(...)  ESP_LOGI(TAG, ##__VA_ARGS__)
#define CHECK(a, str, ret_val) do { \
        if (!(a)) { \
            ESP_LOGE(TAG,"%s(%d): %s", __FUNCTION__, __LINE__, str); \
            return (ret_val); \
        } \
    } while(0)

#define COLOUR_BLUE             0x5C9FD4

/* UI function declaration */
ui_func_desc_t ui_bluetooth_func = {
    .name = "Bluetooth Connect",
    .init = ui_bluetooth_init,
    .show = ui_bluetooth_show,
    .hide = ui_bluetooth_hide,
};

/* LVGL objects definition */
static lv_obj_t *bt_refresh_btn, *obj_page_bluetooth = NULL;
static lv_obj_t *bt_device_list = NULL;

static lv_task_t *bt_update_task;

static ui_state_t ui_bluetooth_state = ui_state_dis;

static uint8_t device_select = 0x00;

static void bt_list_event_handler(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_SHORT_CLICKED == event) {
        device_select = lv_list_get_btn_index(bt_device_list, obj);
        ESP_LOGW(TAG, "Select:%d, %s", device_select, lv_list_get_btn_text(obj));

        bt_scan_result_t scan_result = app_bluetooth_get_scan_result();
        if (device_select < scan_result.device_count) {
            app_bluetooth_connect(scan_result.devices[device_select].addr);
        }
    }
}

static void bt_refresh_event_handler(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_SHORT_CLICKED == event) {
        ESP_LOGI(TAG, "Refresh Bluetooth List");
        app_bluetooth_send_event(BT_EVENT_SCAN);
    }
}

static void update_bt_list(lv_obj_t *parent)
{
    uint8_t list_cn;
    lv_obj_t *list_btn, *label_item;
    sys_param_t *sys_set = settings_get_parameter();

    if (NULL == parent) {
        return;
    }

    app_bluetooth_lock(0);

    bt_scan_result_t scan_result = app_bluetooth_get_scan_result();

    if (BT_SCAN_BUSY == scan_result.scan_state) {
        if (false == parent->hidden) {
            lv_obj_set_hidden(parent, true);
        }

        do {
            list_cn = lv_list_get_size(parent);
            for (int i = 0; i < list_cn; i++) {
                lv_list_remove(parent, i);
            }
        } while (lv_list_get_size(parent));
    } else if ((BT_SCAN_RENEW == scan_result.scan_state) || (BT_SCAN_UPDATE == scan_result.scan_state)) {
        if (true == parent->hidden) {
            lv_obj_set_hidden(parent, false);
        }

        list_cn = lv_list_get_size(parent);

        if (BT_SCAN_UPDATE == scan_result.scan_state) {
            do {
                list_cn = lv_list_get_size(parent);
                for (int i = 0; i < list_cn; i++) {
                    lv_list_remove(parent, i);
                }
            } while (lv_list_get_size(parent));
        } else {
            ESP_LOGI(TAG, "list_cn:%d, [%s]", list_cn, "renew");
        }

        // Move connected device to top if exists
        for (int i = 0; i < scan_result.device_count; i++) {
            if ((0 == memcmp(sys_set->bt_addr, scan_result.devices[i].addr, 6)) && (i > 0)) {
                bt_device_info_t temp_device;
                memcpy(&temp_device, &scan_result.devices[0], sizeof(bt_device_info_t));
                memcpy(&scan_result.devices[0], &scan_result.devices[i], sizeof(bt_device_info_t));
                memcpy(&scan_result.devices[i], &temp_device, sizeof(bt_device_info_t));
            }
        }

        for (int i = 0; i < scan_result.device_count; i++) {

            list_btn = lv_list_add_btn(parent, NULL, NULL);
            lv_obj_set_size(list_btn, 400, 30);
            lv_obj_set_style_local_outline_width(list_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
            lv_obj_set_style_local_outline_width(list_btn, LV_BTN_PART_MAIN, LV_STATE_FOCUSED, 0);

            lv_obj_set_event_cb(list_btn, bt_list_event_handler);

            label_item = lv_label_create(list_btn, NULL);
            lv_obj_set_style_local_text_font(label_item, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_24);

            if (0 == memcmp(scan_result.devices[i].addr, sys_set->bt_addr, 6)) {

                char device_connected[64];
                memset(device_connected, 0, sizeof(device_connected));

                bt_connect_status_t status = app_bluetooth_get_connect_status();
                if (BT_STATUS_CONNECTING == status) {
                    snprintf(device_connected, sizeof(device_connected), "%s %s", 
                            scan_result.devices[i].name,
                            (SR_LANG_CN == sys_set->sr_lang) ? "连接中" : "Connecting");
                } else if (BT_STATUS_CONNECT_FAILED == status) {
                    snprintf(device_connected, sizeof(device_connected), "%s %s", 
                            scan_result.devices[i].name,
                            (SR_LANG_CN == sys_set->sr_lang) ? "未连接" : "Disconnected");
                } else {
                    snprintf(device_connected, sizeof(device_connected), "%s %s", 
                            scan_result.devices[i].name,
                            (SR_LANG_CN == sys_set->sr_lang) ? "已连接" : "Connected");
                }

                lv_label_set_text(label_item, device_connected);
                lv_obj_set_style_local_text_color(label_item, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(COLOUR_BLUE));
            } else {
                lv_obj_set_style_local_text_color(label_item, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x00));
                lv_label_set_text(label_item, scan_result.devices[i].name);
            }
            lv_obj_align(label_item, NULL, LV_ALIGN_IN_LEFT_MID, 0, 0);

            lv_obj_t *label_enter = lv_label_create(list_btn, NULL);
            lv_obj_set_style_local_text_font(label_enter, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_16);
            lv_label_set_text(label_enter, LV_SYMBOL_RIGHT);
            lv_obj_align(label_enter, NULL, LV_ALIGN_IN_RIGHT_MID, 0, 0);
        }

        list_cn = lv_list_get_size(parent);
        ESP_LOGI(TAG, "list_cn:%d, [%s]", list_cn, "end");
    }

    app_bluetooth_unlock();

    if ((BT_SCAN_RENEW == scan_result.scan_state) || (BT_SCAN_UPDATE == scan_result.scan_state)) {
        app_bluetooth_set_scan_state(BT_SCAN_IDLE);
    }
}

void ui_bluetooth_init(void *data)
{
    ui_page_show("Bluetooth Connect");
    obj_page_bluetooth = ui_page_get_obj();

    bt_refresh_btn = lv_btn_create(obj_page_bluetooth, NULL);
    lv_obj_align(bt_refresh_btn, NULL, LV_ALIGN_IN_TOP_RIGHT, 10, 5);
    lv_obj_set_size(bt_refresh_btn, 50, 50);
    lv_obj_set_event_cb(bt_refresh_btn, bt_refresh_event_handler);
    lv_obj_set_style_local_bg_color(bt_refresh_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_bg_color(bt_refresh_btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_WHITE);
    lv_obj_set_style_local_value_color(bt_refresh_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, COLOR_THEME);
    lv_obj_set_style_local_value_color(bt_refresh_btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, COLOR_THEME);
    lv_obj_set_style_local_outline_color(bt_refresh_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_outline_color(bt_refresh_btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_WHITE);
    lv_obj_set_style_local_border_color(bt_refresh_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_border_color(bt_refresh_btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_WHITE);
    lv_obj_set_style_local_value_str(bt_refresh_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_REFRESH);
    lv_obj_set_style_local_value_font(bt_refresh_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_40);

    bt_device_list = lv_list_create(obj_page_bluetooth, NULL);
    lv_obj_set_size(bt_device_list, 700, 310);
    lv_obj_align(bt_device_list, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

    app_bluetooth_set_scan_state(BT_SCAN_RENEW);
    update_bt_list(bt_device_list);

    ui_bluetooth_state = ui_state_show;
}

static void ui_bt_list_timer_cb(lv_task_t *task)
{
    update_bt_list(bt_device_list);
}

void ui_bluetooth_show(void *data)
{
    if (ui_state_dis == ui_bluetooth_state) {
        ui_bluetooth_init(data);
        bt_update_task = lv_task_create(ui_bt_list_timer_cb, 100, 1, NULL);
    } else if (ui_state_hide == ui_bluetooth_state) {
        ui_page_show("Bluetooth Connect");
        ui_bluetooth_state = ui_state_show;
        lv_task_set_cb(bt_update_task, ui_bt_list_timer_cb);
    }
}

void ui_bluetooth_hide(void *data)
{
    if (ui_state_show == ui_bluetooth_state) {
        lv_obj_set_hidden(bt_device_list, true);
        lv_obj_set_hidden(obj_page_bluetooth, true);
        lv_obj_set_hidden(bt_refresh_btn, true);

        ui_bluetooth_state = ui_state_hide;
        lv_task_set_cb(bt_update_task, NULL);
    }
}