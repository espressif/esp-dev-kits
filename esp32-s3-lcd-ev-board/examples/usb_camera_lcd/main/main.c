/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "usb_stream.h"

/* uncomment when using bulk mode camera */
#define EXAMPLE_BULK_TRANSFER_MODE

/* uncomment if transfer uvc frame to wifi http instead of lcd*/
#define ENABLE_UVC_WIFI_XFER    (1)

/* USB Camera Descriptors Related MACROS,
the quick demo skip the standard get descriptors process,
users need to get params from camera descriptors from log,
then hardcode the related MACROS below
*/
#define DESCRIPTOR_CONFIGURATION_INDEX  (1)
#define DESCRIPTOR_FORMAT_MJPEG_INDEX   (2)

#define DESCRIPTOR_FRAME_800_480_INDEX  (1)
#define DESCRIPTOR_FRAME_640_480_INDEX  (2)
#define DESCRIPTOR_FRAME_320_240_INDEX  (3)

#define DESCRIPTOR_FRAME_5FPS_INTERVAL  (2000000)
#define DESCRIPTOR_FRAME_10FPS_INTERVAL (1000000)
#define DESCRIPTOR_FRAME_15FPS_INTERVAL (666666)
#define DESCRIPTOR_FRAME_25FPS_INTERVAL (400000)
#define DESCRIPTOR_FRAME_30FPS_INTERVAL (333333)

#define DESCRIPTOR_STREAM_INTERFACE_INDEX       (1)
#define DESCRIPTOR_STREAM_INTERFACE_ALT_MPS_128 (1)
#define DESCRIPTOR_STREAM_INTERFACE_ALT_MPS_256 (2)
#define DESCRIPTOR_STREAM_INTERFACE_ALT_MPS_512 (3)
#define DESCRIPTOR_STREAM_INTERFACE_BULK        (0)

#define DESCRIPTOR_STREAM_ENDPOINT_ADDR (0x81)

/* Demo Related MACROS */
#ifndef EXAMPLE_BULK_TRANSFER_MODE
/* Isochronous transfer mode config */
#define DEMO_XFER_MODE          UVC_XFER_ISOC
#define DEMO_FRAME_WIDTH        (320)
#define DEMO_FRAME_HEIGHT       (240)
#define DEMO_XFER_BUFFER_SIZE   (35 * 1024) //Double buffer
#define DEMO_FRAME_INDEX        DESCRIPTOR_FRAME_320_240_INDEX
#define DEMO_FRAME_INTERVAL     DESCRIPTOR_FRAME_15FPS_INTERVAL
#define DEMO_INTERFACE_ALT      DESCRIPTOR_STREAM_INTERFACE_ALT_MPS_512
#define DEMO_EP_MPS             (512)         // max MPS of esp32-s2/s3 is 1*512
#elif defined(EXAMPLE_BULK_TRANSFER_MODE)
/* Bulk transfer mode config */
#define DEMO_XFER_MODE          UVC_XFER_BULK
#define DEMO_FRAME_WIDTH        (800)
#define DEMO_FRAME_HEIGHT       (480)
#define DEMO_XFER_BUFFER_SIZE   (48 * 1024) //Double buffer
#define DEMO_FRAME_INDEX        DESCRIPTOR_FRAME_800_480_INDEX
#define DEMO_FRAME_INTERVAL     DESCRIPTOR_FRAME_15FPS_INTERVAL
#define DEMO_INTERFACE_ALT      DESCRIPTOR_STREAM_INTERFACE_BULK
#define DEMO_EP_MPS             (64)
#endif

#if ENABLE_UVC_WIFI_XFER
#include "app_wifi.h"
#include "app_httpd.h"
#include "esp_camera.h"

#define BIT1_NEW_FRAME_START        (0x01 << 1)
#define BIT2_NEW_FRAME_END          (0x01 << 2)

static SemaphoreHandle_t frame_ready = NULL;
static SemaphoreHandle_t http_ready = NULL;
static camera_fb_t s_fb = {0};
#else
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_jpeg_dec.h"
#include "esp_painter.h"
#include "bsp_lcd.h"

static esp_painter_handle_t painter = NULL;
static esp_lcd_panel_handle_t lcd_panel = NULL;
static uint8_t *rgb_frame_buf1 = NULL;
static uint8_t *rgb_frame_buf2 = NULL;
static uint8_t *cur_frame_buf = NULL;
#endif

static const char *TAG = "app_main";

static void camera_frame_cb(uvc_frame_t *frame, void *ptr);

void app_main(void)
{
#if !ENABLE_UVC_WIFI_XFER
    lcd_panel = bsp_lcd_init();
    assert(lcd_panel);
    esp_lcd_rgb_panel_get_frame_buffer(lcd_panel, 2, (void **)&rgb_frame_buf1, (void **)&rgb_frame_buf2);
    cur_frame_buf = rgb_frame_buf2;

    esp_painter_config_t painter_config = {
        .brush.color = COLOR_RGB565_RED,
        .canvas = {
            .x = 0,
            .y = 0,
            .width = BSP_LCD_H_RES,
            .height = BSP_LCD_V_RES,
        },
        .default_font = &esp_painter_basic_font_24,
        .piexl_color_byte = 2,
        .lcd_panel = lcd_panel,
    };
    ESP_ERROR_CHECK(esp_painter_new(&painter_config, &painter));
#endif

    /* malloc double buffer for usb payload, xfer_buffer_size >= frame_buffer_size*/
    uint8_t *xfer_buffer_a = (uint8_t *)malloc(DEMO_XFER_BUFFER_SIZE);
    assert(xfer_buffer_a != NULL);
    uint8_t *xfer_buffer_b = (uint8_t *)malloc(DEMO_XFER_BUFFER_SIZE);
    assert(xfer_buffer_b != NULL);
    /* malloc frame buffer for a jpeg frame*/
    uint8_t *frame_buffer = (uint8_t *)malloc(DEMO_XFER_BUFFER_SIZE);
    assert(frame_buffer != NULL);

    /* the quick demo skip the standred get descriptors process,
    users need to get params from camera descriptors from demo print */
    uvc_config_t uvc_config = {
        .format_index = DESCRIPTOR_FORMAT_MJPEG_INDEX,
        .frame_width = DEMO_FRAME_WIDTH,
        .frame_height = DEMO_FRAME_HEIGHT,
        .frame_index = DEMO_FRAME_INDEX,
        .frame_interval = DEMO_FRAME_INTERVAL,
        .interface = DESCRIPTOR_STREAM_INTERFACE_INDEX,
        .interface_alt = DEMO_INTERFACE_ALT,
        .ep_addr = DESCRIPTOR_STREAM_ENDPOINT_ADDR,
        .ep_mps = DEMO_EP_MPS,
        .xfer_type = UVC_XFER_BULK,
        .xfer_buffer_size = DEMO_XFER_BUFFER_SIZE,
        .xfer_buffer_a = xfer_buffer_a,
        .xfer_buffer_b = xfer_buffer_b,
        .frame_buffer_size = DEMO_XFER_BUFFER_SIZE,
        .frame_buffer = frame_buffer,
        .frame_cb = &camera_frame_cb,
        .frame_cb_arg = NULL,
    };
    /* pre-config UVC driver with params from known USB Camera Descriptors*/
    ESP_ERROR_CHECK(uvc_streaming_config(&uvc_config));
    /* Start stream with pre-configs, usb stream driver will create multi-tasks internal
    to handle usb data from different pipes, and user's callback will be called after new frame ready. */
    ESP_ERROR_CHECK(usb_streaming_start());

#if ENABLE_UVC_WIFI_XFER
    frame_ready = xSemaphoreCreateBinary();
    http_ready = xSemaphoreCreateBinary();
    assert(frame_ready && http_ready);
    app_wifi_main();
    app_httpd_main();
#endif
}

#if ENABLE_UVC_WIFI_XFER
camera_fb_t* esp_camera_fb_get()
{
    xSemaphoreTake(frame_ready, portMAX_DELAY);
    return &s_fb;
}

void esp_camera_fb_return(camera_fb_t * fb)
{
    xSemaphoreGive(http_ready);
    return;
}
#else
static int esp_jpeg_decoder_one_picture(uint8_t *input_buf, int len, uint8_t *output_buf)
{
    esp_err_t ret = ESP_OK;
    // Generate default configuration
    jpeg_dec_config_t config = DEFAULT_JPEG_DEC_CONFIG();

    // Empty handle to jpeg_decoder
    jpeg_dec_handle_t jpeg_dec = NULL;

    // Create jpeg_dec
    jpeg_dec = jpeg_dec_open(&config);

    // Create io_callback handle
    jpeg_dec_io_t *jpeg_io = calloc(1, sizeof(jpeg_dec_io_t));
    if (jpeg_io == NULL) {
        return ESP_FAIL;
    }

    // Create out_info handle
    jpeg_dec_header_info_t *out_info = calloc(1, sizeof(jpeg_dec_header_info_t));
    if (out_info == NULL) {
        return ESP_FAIL;
    }
    // Set input buffer and buffer len to io_callback
    jpeg_io->inbuf = input_buf;
    jpeg_io->inbuf_len = len;

    // Parse jpeg picture header and get picture for user and decoder
    ret = jpeg_dec_parse_header(jpeg_dec, jpeg_io, out_info);
    if (ret < 0) {
        goto _exit;
    }

    jpeg_io->outbuf = output_buf;
    int inbuf_consumed = jpeg_io->inbuf_len - jpeg_io->inbuf_remain;
    jpeg_io->inbuf = input_buf + inbuf_consumed;
    jpeg_io->inbuf_len = jpeg_io->inbuf_remain;

    // Start decode jpeg raw data
    ret = jpeg_dec_process(jpeg_dec, jpeg_io);
    if (ret < 0) {
        goto _exit;
    }

_exit:
    // Decoder deinitialize
    jpeg_dec_close(jpeg_dec);
    free(out_info);
    free(jpeg_io);
    return ret;
}
#endif

static void camera_frame_cb(uvc_frame_t *frame, void *ptr)
{
    ESP_LOGD(TAG, "uvc callback! frame_format = %d, seq = %"PRIu32", width = %"PRIu32", height = %"PRIu32", length = %u, ptr = %d",
            frame->frame_format, frame->sequence, frame->width, frame->height, frame->data_bytes, (int) ptr);

    static int frame_count = 0;
    static int64_t count_start_time = 0;
    static int fps = 0;

    switch (frame->frame_format) {
        case UVC_FRAME_FORMAT_MJPEG:

            if (count_start_time == 0) {
                count_start_time = esp_timer_get_time();
            }
            if (++frame_count == 20) {
                frame_count = 0;
                fps = 20 * 1000000 / (esp_timer_get_time() - count_start_time);
                count_start_time = esp_timer_get_time();
                ESP_LOGI(TAG, "camera fps: %d", fps);
            }
#if ENABLE_UVC_WIFI_XFER
            s_fb.buf = frame->data;
            s_fb.len = frame->data_bytes;
            s_fb.width = frame->width;
            s_fb.height = frame->height;
            s_fb.format = PIXFORMAT_JPEG;
            s_fb.timestamp.tv_sec = frame->sequence;
            xSemaphoreGive(frame_ready);
            xSemaphoreTake(http_ready, pdMS_TO_TICKS(1000));
            ESP_LOGD(TAG, "send frame = %"PRIu32"",frame->sequence);
#else
            esp_jpeg_decoder_one_picture((uint8_t *)frame->data, frame->data_bytes, cur_frame_buf);
            esp_lcd_panel_draw_bitmap(lcd_panel, 0, 0, 1, 1, cur_frame_buf);
            cur_frame_buf = (cur_frame_buf == rgb_frame_buf1) ? rgb_frame_buf2 : rgb_frame_buf1;
            esp_painter_draw_string_format(painter, 0, 0, NULL, COLOR_BRUSH_DEFAULT, "FPS: %d", fps);

            vTaskDelay(pdMS_TO_TICKS(1));
#endif
            break;
        default:
            ESP_LOGW(TAG, "Format not supported");
            assert(0);
            break;
    }
}
