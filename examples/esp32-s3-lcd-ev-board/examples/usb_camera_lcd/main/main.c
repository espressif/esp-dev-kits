/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
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
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_jpeg_dec.h"
#include "esp_painter.h"
#include "usb_stream.h"
#include "bsp/esp-bsp.h"
#include "bsp/display.h"

/* Transfer uvc frame to wifi http */
#define ENABLE_UVC_WIFI_XFER            (1)

/* Print log about SRAM and PSRAM memory */
#define LOG_MEM_INFO    (0)

#define EXAMPLE_UVC_XFER_BUFFER_SIZE       (88 * 1024)     //Double buffer

#if ENABLE_UVC_WIFI_XFER
#include "app_wifi.h"
#include "app_httpd.h"
#include "esp_camera.h"

static SemaphoreHandle_t sem_usb_done = NULL;
static SemaphoreHandle_t sem_xfer_done = NULL;
static camera_fb_t s_fb = {0};
static bool xfer_connect = false;
#endif

static esp_painter_handle_t painter = NULL;
static esp_lcd_panel_handle_t lcd_panel = NULL;
static uint8_t *lcd_frame_buf[CONFIG_BSP_LCD_RGB_BUFFER_NUMS] = { NULL };
static uint8_t draw_buf_index = 0;

static const char *TAG = "example_usb_camera_lcd";

extern esp_lcd_panel_handle_t bsp_lcd_init(void *arg);

static void camera_frame_cb(uvc_frame_t *frame, void *ptr);

void app_main(void)
{
    ESP_ERROR_CHECK(bsp_display_new(NULL, &lcd_panel, NULL));
    assert(lcd_panel);

#if CONFIG_BSP_LCD_RGB_BUFFER_NUMS == 1
    esp_lcd_rgb_panel_get_frame_buffer(lcd_panel, CONFIG_BSP_LCD_RGB_BUFFER_NUMS, (void **)&lcd_frame_buf[0]);
#elif CONFIG_BSP_LCD_RGB_BUFFER_NUMS == 2
    esp_lcd_rgb_panel_get_frame_buffer(lcd_panel, CONFIG_BSP_LCD_RGB_BUFFER_NUMS, (void **)&lcd_frame_buf[0], (void **)&lcd_frame_buf[1]);
#elif CONFIG_BSP_LCD_RGB_BUFFER_NUMS == 3
    esp_lcd_rgb_panel_get_frame_buffer(lcd_panel, CONFIG_BSP_LCD_RGB_BUFFER_NUMS, (void **)&lcd_frame_buf[0], (void **)&lcd_frame_buf[1], (void **)&lcd_frame_buf[2]);
#endif
    draw_buf_index = CONFIG_BSP_LCD_RGB_BUFFER_NUMS - 1;

    uint16_t uvc_frame_w = BSP_LCD_H_RES;
    uint16_t uvc_frame_h = (BSP_LCD_H_RES == 480) ? 320 : BSP_LCD_V_RES;

    esp_painter_config_t painter_config = {
        .brush.color = COLOR_RGB565_RED,
        .canvas = {
            .x = 0,
            .y = 0,
            .width = uvc_frame_w,
            .height = uvc_frame_h,
        },
        .default_font = &esp_painter_basic_font_24,
        .piexl_color_byte = 2,
        .lcd_panel = lcd_panel,
    };
    ESP_ERROR_CHECK(esp_painter_new(&painter_config, &painter));

#if ENABLE_UVC_WIFI_XFER
    sem_usb_done = xSemaphoreCreateBinary();
    sem_xfer_done = xSemaphoreCreateBinary();
    assert(sem_usb_done && sem_xfer_done);
    app_wifi_main();
    app_httpd_main();
#endif

    /* Malloc double buffer for usb payload, xfer_buffer_size >= frame_buffer_size*/
    uint8_t *xfer_buffer_a = (uint8_t *)malloc(EXAMPLE_UVC_XFER_BUFFER_SIZE);
    assert(xfer_buffer_a != NULL);
    uint8_t *xfer_buffer_b = (uint8_t *)malloc(EXAMPLE_UVC_XFER_BUFFER_SIZE);
    assert(xfer_buffer_b != NULL);

    /* Malloc frame buffer for a jpeg frame*/
    uint8_t *frame_buffer = (uint8_t *)malloc(EXAMPLE_UVC_XFER_BUFFER_SIZE);
    assert(frame_buffer != NULL);

    uvc_config_t uvc_config = {
        .frame_width = uvc_frame_w,
        .frame_height = uvc_frame_h,
        .frame_interval = FRAME_INTERVAL_FPS_30,
        .xfer_buffer_size = EXAMPLE_UVC_XFER_BUFFER_SIZE,
        .xfer_buffer_a = xfer_buffer_a,
        .xfer_buffer_b = xfer_buffer_b,
        .frame_buffer_size = EXAMPLE_UVC_XFER_BUFFER_SIZE,
        .frame_buffer = frame_buffer,
        .frame_cb = &camera_frame_cb,
        .frame_cb_arg = NULL,
    };
    ESP_ERROR_CHECK(uvc_streaming_config(&uvc_config));

    /* Start stream with pre-configs, usb stream driver will create multi-tasks internal
    to handle usb data from different pipes, and user's callback will be called after new frame ready. */
    ESP_ERROR_CHECK(usb_streaming_start());

#if LOG_MEM_INFO
    static char buffer[128];    /* Make sure buffer is enough for `sprintf` */
    while (1) {
        /**
         * It's not recommended to frequently use functions like `heap_caps_get_free_size()` to obtain memory information
         * in practical applications, especially when the application extensively uses `malloc()` to dynamically allocate
         * a significant number of memory blocks. The frequent interrupt disabling may potentially lead to issues with other functionalities.
         */
        sprintf(buffer, "   Biggest /     Free /    Total\n"
                "\t  SRAM : [%8d / %8d / %8d]\n"
                "\t PSRAM : [%8d / %8d / %8d]",
                heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL),
                heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
                heap_caps_get_total_size(MALLOC_CAP_INTERNAL),
                heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM),
                heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
                heap_caps_get_total_size(MALLOC_CAP_SPIRAM));
        ESP_LOGI("MEM", "%s", buffer);

        vTaskDelay(pdMS_TO_TICKS(500));
    }
#endif
}

#if ENABLE_UVC_WIFI_XFER
camera_fb_t* esp_camera_fb_get()
{
    xSemaphoreTake(sem_usb_done, portMAX_DELAY);
    return &s_fb;
}

void esp_camera_fb_return(camera_fb_t * fb)
{
    xfer_connect = true;
    xSemaphoreGive(sem_xfer_done);
    return;
}
#endif

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

static void camera_frame_cb(uvc_frame_t *frame, void *ptr)
{
    ESP_LOGD(TAG, "uvc callback! frame_format = %d, seq = %"PRIu32", width = %"PRIu32", height = %"PRIu32", length = %u, ptr = %d",
            frame->frame_format, frame->sequence, frame->width, frame->height, frame->data_bytes, (int) ptr);

    static int frame_count = 0;
    static int64_t count_start_time = 0;
    static int fps = 0;

    switch (frame->frame_format) {
        case UVC_FRAME_FORMAT_MJPEG:
#if ENABLE_UVC_WIFI_XFER
            s_fb.buf = frame->data;
            s_fb.len = frame->data_bytes;
            s_fb.width = frame->width;
            s_fb.height = frame->height;
            s_fb.format = PIXFORMAT_JPEG;
            s_fb.timestamp.tv_sec = frame->sequence;
            xSemaphoreGive(sem_usb_done);
#endif
            esp_jpeg_decoder_one_picture((uint8_t *)frame->data, frame->data_bytes, lcd_frame_buf[draw_buf_index]);
            esp_lcd_panel_draw_bitmap(lcd_panel, 0, 0, frame->width, frame->height, lcd_frame_buf[draw_buf_index]);
            // Switch to next frame buffer for next drawing
            draw_buf_index = (draw_buf_index + 1) == CONFIG_BSP_LCD_RGB_BUFFER_NUMS ? 0 : (draw_buf_index + 1);
            esp_painter_draw_string_format(painter, 0, 0, NULL, COLOR_BRUSH_DEFAULT, "FPS: %d", fps);
#if ENABLE_UVC_WIFI_XFER
            if (xfer_connect) {
                if (xSemaphoreTake(sem_xfer_done, pdMS_TO_TICKS(1000)) == pdFALSE) {
                    // Xfer is disconnected if timeout, so we don't have to wait next time
                    xfer_connect = false;
                }
            }
#endif

            if (count_start_time == 0) {
                count_start_time = esp_timer_get_time();
            }
            if (++frame_count == 20) {
                frame_count = 0;
                fps = 20 * 1000000 / (esp_timer_get_time() - count_start_time);
                count_start_time = esp_timer_get_time();
                ESP_LOGD(TAG, "camera fps: %d", fps);
            }
            vTaskDelay(pdMS_TO_TICKS(1));
            break;
        default:
            ESP_LOGW(TAG, "Format not supported");
            assert(0);
            break;
    }
}
