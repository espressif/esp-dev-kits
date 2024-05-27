/*
 * SPDX-FileCopyrightText: 2019-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "soc/efuse_reg.h"
#include "esp_heap_caps.h"
#include "driver/gpio.h"
#include "driver/spi_common.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "hal/usb_hal.h"
#include "spi_bus.h"
#include "st7789.h"
#include "screen_driver.h"
#include "display_printf.h"
#include "usb_stream.h"
#include "jpegd2.h"
#include "bsp_esp32_s3_usb_otg_ev.h"
#include "app.h"

#ifdef CONFIG_USE_PSRAM
#if CONFIG_IDF_TARGET_ESP32 // ESP32/PICO-D4
#include "esp32/spiram.h"
#elif CONFIG_IDF_TARGET_ESP32S2
#include "esp32s2/spiram.h"
#elif CONFIG_IDF_TARGET_ESP32S3
#include "esp32s3/spiram.h"
#else
#error Target CONFIG_IDF_TARGET is not supported
#endif
#endif

/* USB PIN fixed in esp32-s2, can not use io matrix */
#define BOARD_USB_DP_PIN 20
#define BOARD_USB_DN_PIN 19

/* USB Camera Descriptors Related MACROS,
the quick demo skip the standred get descriptors process,
users need to get params from camera descriptors from PC side,
eg. run `lsusb -v` in linux,
then hardcode the related MACROS below
*/
#define DESCRIPTOR_FRAME_5FPS_INTERVAL  2000000
#define DESCRIPTOR_FRAME_10FPS_INTERVAL 1000000
#define DESCRIPTOR_FRAME_15FPS_INTERVAL 666666
#define DESCRIPTOR_FRAME_30FPS_INTERVAL 333333

/* Demo Related MACROS */
#ifdef CONFIG_SIZE_320_240
#define DEMO_FRAME_WIDTH 320
#define DEMO_FRAME_HEIGHT 240
#define DEMO_XFER_BUFFER_SIZE (35 * 1024) //Double buffer
#define DEMO_FRAME_INTERVAL DESCRIPTOR_FRAME_15FPS_INTERVAL
#elif CONFIG_SIZE_160_120
#define DEMO_FRAME_WIDTH 160
#define DEMO_FRAME_HEIGHT 120
#define DEMO_XFER_BUFFER_SIZE (20 * 1024) //Double buffer
#define DEMO_FRAME_INTERVAL DESCRIPTOR_FRAME_30FPS_INTERVAL
#endif

#ifdef CONFIG_BOOT_ANIMATION
#define BOOT_ANIMATION_MAX_SIZE (45 * 1024)
#endif

#define DEMO_SPI_MAX_TRANFER_SIZE (CONFIG_LCD_BUF_WIDTH * CONFIG_LCD_BUF_HIGHT * 2 + 64)

static const char *TAG = "uvc_demo";
static scr_driver_t s_lcd;
static scr_info_t s_lcd_info;
static bool s_lcd_inited = false;
static TaskHandle_t s_task_hdl = NULL;
QueueHandle_t g_usb_camera_queue_hdl = NULL;

#define EVENT_TASK_KILL_BIT_0   ( 1 << 0 )
#define EVENT_TASK_KILLED_BIT_1 ( 1 << 1 )
static EventGroupHandle_t s_event_group_hdl = NULL;

static void *_malloc(size_t size)
{
#ifdef CONFIG_USE_PSRAM
#ifndef CONFIG_ESP32S2_SPIRAM_SUPPORT
#error CONFIG_SPIRAM no defined, Please enable "Component config → ESP32S2-specific → Support for external SPI ram"
#endif
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
#else
    return heap_caps_malloc(size, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
#endif
}

/* *******************************************************************************************
 * This callback function runs once per frame. Use it to perform any
 * quick processing you need, or have it put the frame into your application's
 * input queue. If this function takes too long, you'll start losing frames. */
static void frame_cb(uvc_frame_t *frame, void *ptr)
{
    assert(ptr);
    uint8_t *lcd_buffer = (uint8_t *)(ptr);
    ESP_LOGV(TAG, "callback! frame_format = %d, seq = %u, width = %d, height = %d, length = %u, ptr = %d",
             frame->frame_format, frame->sequence, frame->width, frame->height, frame->data_bytes, (int) ptr);

    switch (frame->frame_format) {
    case UVC_FRAME_FORMAT_MJPEG:
        mjpegdraw(frame->data, frame->data_bytes, lcd_buffer, CONFIG_LCD_BUF_WIDTH, CONFIG_LCD_BUF_HIGHT, board_lcd_draw_image, 240, 240);
        // vTaskDelay(10 / portTICK_PERIOD_MS); /* add delay to free cpu to other tasks */
        break;
    default:
        ESP_LOGW(TAG, "Format not supported");
        assert(0);
        break;
    }
}

void printf_camera_state(char *state)
{
    DISPLAY_PRINTF_LINE("SD", 3, COLOR_RED, "...%s", state);
}

void usb_camera_task(void *pvParameters)
{
    iot_board_usb_set_mode(USB_HOST_MODE);
    iot_board_usb_device_set_power(true, true);
    gpio_set_level(BOARD_IO_DEV_VBUS_EN, true);//WARNING: open two power source
    usb_hal_context_t hal = {
        .use_external_phy = false
    };
    usb_hal_init(&hal);
    DISPLAY_PRINTF_INIT((scr_driver_t *)iot_board_get_handle(BOARD_LCD_ID));
    DISPLAY_PRINTF_SET_FONT(Font16);
    DISPLAY_PRINTF_CLEAR();
    DISPLAY_PRINTF_LINE("SD", 1, COLOR_GREEN, "USB Camera");
    DISPLAY_PRINTF_LINE("SD", 2, COLOR_RED, "Wait Camera Connect");
    DISPLAY_PRINTF_LINE("SD", 3, COLOR_BLUE, "...Waiting");
    DISPLAY_PRINTF_LINE("SD", 6, COLOR_RED, "Power Source:");
    DISPLAY_PRINTF_LINE("SD", 7, COLOR_BLUE, "Battery or USB DEV");
    EventGroupHandle_t s_event_group_hdl = (EventGroupHandle_t) pvParameters;
    g_usb_camera_queue_hdl = xQueueCreate(1, sizeof(hmi_event_t));
    assert(g_usb_camera_queue_hdl != NULL);

    /* malloc a buffer for RGB565 data, as 320*240*2 = 153600B,
    here malloc a smaller buffer refresh lcd with steps */
    uint8_t *lcd_buffer = (uint8_t *)heap_caps_malloc(DEMO_SPI_MAX_TRANFER_SIZE, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    assert(lcd_buffer != NULL);

    /* Boot animation useful for LCD checking and camera power-on waiting, But consumes much flash */
#ifdef CONFIG_BOOT_ANIMATION
    esp_vfs_spiffs_conf_t spiffs_config = {
        .base_path              = "/spiffs",
        .partition_label        = NULL,
        .max_files              = 5,
        .format_if_mount_failed = false
    };

    ESP_ERROR_CHECK(esp_vfs_spiffs_register(&spiffs_config));

    uint8_t *jpeg_buf = malloc(BOOT_ANIMATION_MAX_SIZE);
    assert(jpeg_buf != NULL);
    for (size_t i = 10; i <= 80; i += 2) {
        char file_name[64] = {0};
        sprintf(file_name, "/spiffs/video/r%03d.jpg", i);
        FILE *fd = fopen(file_name, "r");
        int read_bytes = fread(jpeg_buf, 1, BOOT_ANIMATION_MAX_SIZE, fd);
        fclose(fd);
        mjpegdraw(jpeg_buf, read_bytes, lcd_buffer, CONFIG_LCD_BUF_WIDTH, CONFIG_LCD_BUF_HIGHT, lcd_write_bitmap, 240, 240);
        ESP_LOGD(TAG, "file_name: %s, fd: %p, read_bytes: %d, free_heap: %d", file_name, fd, read_bytes, esp_get_free_heap_size());
    }
    free(jpeg_buf);
#endif

    /* malloc double buffer for usb payload, xfer_buffer_size >= frame_buffer_size*/
    uint8_t *xfer_buffer_a = (uint8_t *)_malloc(DEMO_XFER_BUFFER_SIZE);
    assert(xfer_buffer_a != NULL);
    uint8_t *xfer_buffer_b = (uint8_t *)_malloc(DEMO_XFER_BUFFER_SIZE);
    assert(xfer_buffer_b != NULL);

    /* malloc frame buffer for a jpeg frame*/
    uint8_t *frame_buffer = (uint8_t *)_malloc(DEMO_XFER_BUFFER_SIZE);
    assert(frame_buffer != NULL);

    /* the quick demo skip the standred get descriptors process,
    users need to get params from camera descriptors from PC side,
    eg. run `lsusb -v` in linux, then modify related MACROS */
    uvc_config_t uvc_config = {
        .frame_width = DEMO_FRAME_WIDTH,
        .frame_height = DEMO_FRAME_HEIGHT,
        .frame_interval = DEMO_FRAME_INTERVAL,
        .xfer_buffer_size = DEMO_XFER_BUFFER_SIZE,
        .xfer_buffer_a = xfer_buffer_a,
        .xfer_buffer_b = xfer_buffer_b,
        .frame_buffer_size = DEMO_XFER_BUFFER_SIZE,
        .frame_buffer = frame_buffer,
        .frame_cb = frame_cb,
        .frame_cb_arg = (void *)(lcd_buffer),
    };

    /* pre-config UVC driver with params from known USB Camera Descriptors*/
    esp_err_t ret = uvc_streaming_config(&uvc_config);

    /* Start camera IN stream with pre-configs, uvc driver will create multi-tasks internal
    to handle usb data from different pipes, and user's callback will be called after new frame ready. */
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "uvc streaming config failed");
    } else {
        usb_streaming_start();
    }

    hmi_event_t current_event;
    while (!(xEventGroupGetBits(s_event_group_hdl) & EVENT_TASK_KILL_BIT_0)) {
        if (xQueueReceive(g_usb_camera_queue_hdl, &current_event, portMAX_DELAY) != pdTRUE) {
            continue;
        }
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
    };
    QueueHandle_t queue_hdl = g_usb_camera_queue_hdl;
    g_usb_camera_queue_hdl = NULL;
    vQueueDelete(queue_hdl);
    usb_streaming_stop();
    free(lcd_buffer);
    free(xfer_buffer_a);
    free(xfer_buffer_b);
    free(frame_buffer);
    xEventGroupSetBits(s_event_group_hdl, EVENT_TASK_KILLED_BIT_1);
    xEventGroupClearBits(s_event_group_hdl, EVENT_TASK_KILL_BIT_0);
    iot_board_usb_set_mode(USB_DEVICE_MODE);
    iot_board_usb_device_set_power(false, false);
    gpio_set_level(BOARD_IO_DEV_VBUS_EN, false);
    s_task_hdl = NULL;
    vTaskDelete(s_task_hdl);
}

void usb_camera_init(void)
{
    if (s_task_hdl != NULL) {
        return;
    }
    s_event_group_hdl = xEventGroupCreate();
    xEventGroupClearBits(s_event_group_hdl, EVENT_TASK_KILL_BIT_0 | EVENT_TASK_KILLED_BIT_1);
    xTaskCreate(usb_camera_task, "camera", 4096, (void *)s_event_group_hdl, TASK_APP_PRIO_MIN, &s_task_hdl);
    ESP_LOGI(TAG, "Camera APP Inited");
}

void usb_camera_deinit(void)
{
    if (s_task_hdl == NULL) {
        return;
    }
    xEventGroupSetBits(s_event_group_hdl, EVENT_TASK_KILL_BIT_0);
    xEventGroupWaitBits(s_event_group_hdl, EVENT_TASK_KILLED_BIT_1, true, true, portMAX_DELAY);
    vEventGroupDelete(s_event_group_hdl);
    s_event_group_hdl = NULL;
    ESP_LOGW(TAG, "Camera APP Deinited");
}
