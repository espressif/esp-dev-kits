
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

static const char *TAG = "usb_hid_keyboard";
#define EVENT_TASK_KILL_BIT_0	( 1 << 0 )
#define EVENT_TASK_KILLED_BIT_1	( 1 << 1 )
#define MOUSE_OFFSET 8

QueueHandle_t g_usb_hid_keyboard_hdl = NULL;
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
    xQueueSend(g_usb_hid_keyboard_hdl, &event, portMAX_DELAY);
    ESP_LOGI(TAG, "%s", __func__);
}

static void _cb_hid_unmount(int pdrv, void *arg)
{
    hmi_event_t event = {
        .id = USR_RESERVED_2
    };
    xQueueSend(g_usb_hid_keyboard_hdl, &event, portMAX_DELAY);
    ESP_LOGI(TAG, "%s", __func__);
}

const char keyboard_line0[] = "0 1 2 3 4 5 6 7 8";
const char keyboard_line1[] = "9 a b c d e f g h";
const char keyboard_line2[] = "i j k l m n o p q";
const char keyboard_line3[] = "r s t u v w x y z";
const char blank_line[] = "                 ";
const char *keyboard_lines[] = {keyboard_line0, keyboard_line1, keyboard_line2, keyboard_line3};
const uint16_t keyboard_lines_color[] = {COLOR_RED, COLOR_BLUE, COLOR_RED, COLOR_BLUE};

const size_t screen_line_base = 3;
const size_t screen_line_step = 2;
const char pointer_text = '^';
const uint16_t pointer_text_color = COLOR_GREEN;

static void _display_info(int page_index)
{
    size_t screen_current_base = screen_line_base;
    if (page_index == 0) { //main infomation
        DISPLAY_PRINTF_SET_FONT(Font20);
        for (size_t i = 0; i < sizeof(keyboard_lines)/sizeof(keyboard_lines[0]) ; i++) {
            DISPLAY_PRINTF_LINE(TAG, screen_current_base, COLOR_BLUE, keyboard_lines[i]);
            screen_current_base += screen_line_step;
        }
    } else if (page_index == 1) {
    } else {
    }
}

void usb_hid_keyboard_task(void *pvParameters)
{
    iot_board_usb_set_mode(USB_DEVICE_MODE);
    iot_board_usb_device_set_power(false, false);
    g_usb_hid_keyboard_hdl = xQueueCreate(5, sizeof(hmi_event_t));
    assert(g_usb_hid_keyboard_hdl != NULL);
    DISPLAY_PRINTF_INIT((scr_driver_t *)iot_board_get_handle(BOARD_LCD_ID));
    DISPLAY_PRINTF_SET_FONT(Font20);
    DISPLAY_PRINTF_CLEAR();
    DISPLAY_PRINTF_LINE(TAG, 0, COLOR_GREEN, "USB HID Keyboard");
    DISPLAY_PRINTF_LINE(TAG, 1, COLOR_RED, "Wait Connect PC");
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
    _display_info(0);

    uint8_t _keycode[6] = { 0 };
    int8_t last_line_num = 0;
    int8_t current_line_num = 0;
    int8_t current_char_num = 0;
    DISPLAY_PRINTF_LINE(TAG, current_line_num*screen_line_step+screen_line_base+1, COLOR_GREEN, "^                ");
    bool if_connect = false;

    while (!(xEventGroupGetBits(s_event_group_hdl) & EVENT_TASK_KILL_BIT_0)) {
        if(xQueueReceive(g_usb_hid_keyboard_hdl, &current_event, portMAX_DELAY) != pdTRUE) continue;
        switch (current_event.id) {
            case USR_RESERVED_1:
                if_connect = true;
                DISPLAY_PRINTF_LINE(TAG, 1, COLOR_GREEN, "  Connected      ");
                DISPLAY_PRINTF_LINE(TAG, 2, COLOR_RED, "Click to Choose    ");
                break;
            case USR_RESERVED_2:
                if_connect = false;
                DISPLAY_PRINTF_LINE(TAG, 1, COLOR_RED, "  Lose Connect   ");
                DISPLAY_PRINTF_LINE(TAG, 2, COLOR_RED, "                    ");
                break;
            case BTN_CLICK_MENU:
                if (++current_line_num >= sizeof(keyboard_lines)/sizeof(keyboard_lines[0])) {
                    current_line_num = 0;
                }
                break;
            case BTN_CLICK_UP:
                if (keyboard_lines[current_line_num][++current_char_num] != '\0' && keyboard_lines[current_line_num][current_char_num] == ' ') {
                    ++current_char_num;
                }
                if (keyboard_lines[current_line_num][current_char_num] == '\0') {
                    current_char_num = 0;
                    if (++current_line_num >= sizeof(keyboard_lines)/sizeof(keyboard_lines[0])) {
                        current_line_num = 0;
                    }
                }
                break;
            case BTN_CLICK_DOWN:
                if (--current_char_num > 0 && keyboard_lines[current_line_num][current_char_num] == ' ') {
                    --current_char_num;
                }
                if (current_char_num < 0) {
                    if (--current_line_num < 0) {
                        current_line_num = sizeof(keyboard_lines)/sizeof(keyboard_lines[0]) - 1;
                    }
                    current_char_num = sizeof(keyboard_line0)-1-1;
                }
                break;
            case BTN_CLICK_OK:
                _keycode[0] = (uint8_t)keyboard_lines[current_line_num][current_char_num];
                if(_keycode[0] >= '1' && _keycode[0] <= '9') {
                    _keycode[0] = _keycode[0] - '1' + HID_KEY_1;
                } else if(_keycode[0] >= 'a' && _keycode[0] <= 'z') {
                    _keycode[0] = _keycode[0] - 'a' + HID_KEY_A;
                } else if (_keycode[0] == '0'){
                    _keycode[0] = HID_KEY_0;
                }
                if(if_connect) tinyusb_hid_keyboard_report(_keycode);
                break;
            default:
                break;
        }
        if (last_line_num != current_line_num)
        DISPLAY_PRINTF_LINE(TAG, last_line_num*screen_line_step+screen_line_base+1, COLOR_BLACK, blank_line);
        
        ESP_LOGW(TAG, "current_line_num=%d current_char_num=%d\n", current_line_num, current_char_num);
        ESP_LOGW(TAG, "x=%d y=%d\n", current_char_num * Font20.Width, (current_line_num*screen_line_step+screen_line_base+1)*Font20.Height);
        char pointer_char_line[] = "                 ";
        if(current_char_num<sizeof(pointer_char_line)) pointer_char_line[current_char_num] = pointer_text;
        DISPLAY_PRINTF_LINE(TAG, current_line_num*screen_line_step+screen_line_base+1, COLOR_GREEN, pointer_char_line);
        last_line_num = current_line_num;
    };
    xEventGroupSetBits(s_event_group_hdl, EVENT_TASK_KILLED_BIT_1);
    xEventGroupClearBits(s_event_group_hdl, EVENT_TASK_KILL_BIT_0);
    s_task_hdl = NULL;
    vTaskDelete(s_task_hdl);
}

void usb_hid_keyboard_init(void)
{
    if (s_task_hdl != NULL) return;
    s_event_group_hdl = xEventGroupCreate();
    xEventGroupClearBits(s_event_group_hdl, EVENT_TASK_KILL_BIT_0 | EVENT_TASK_KILLED_BIT_1);
    xTaskCreate(usb_hid_keyboard_task, "usbd_hid_keyboard", 4096, (void *)s_event_group_hdl, TASK_APP_PRIO_MIN, &s_task_hdl);
    ESP_LOGI(TAG, "APP HID Keyboard Inited");
}

void usb_hid_keyboard_deinit(void)
{
    if (s_task_hdl == NULL) return;
    xEventGroupSetBits(s_event_group_hdl, EVENT_TASK_KILL_BIT_0);
    _cb_hid_unmount(0, NULL);
    xEventGroupWaitBits(s_event_group_hdl, EVENT_TASK_KILLED_BIT_1, true, true, portMAX_DELAY);
    vEventGroupDelete(s_event_group_hdl);
    s_event_group_hdl = NULL;
    ESP_LOGW(TAG, "APP HID Keyboard Deinited");
}