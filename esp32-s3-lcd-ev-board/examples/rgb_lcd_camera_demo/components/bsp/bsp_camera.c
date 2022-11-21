#include <stdlib.h>
#include "esp_heap_caps.h"
#include "esp_err.h"
#include "esp_log.h"
#include "uvc_stream.h"
#include "bsp_camera.h"

/* USB PIN fixed in esp32-s2/s3, can not use io matrix */
#define BOARD_USB_DP_PIN 20
#define BOARD_USB_DN_PIN 19

#define DESCRIPTOR_CONFIGURATION_INDEX              1
#define DESCRIPTOR_FORMAT_MJPEG_INDEX               2
#define DESCRIPTOR_STREAM_ENDPOINT_ADDR             0x81

#define BSP_CAMERA_XFER_MODE                UVC_XFER_BULK
#define BSP_CAMERA_XFER_BUFFER_SIZE         (CONFIG_BSP_CAMERA_BUFFER_SIZE * 1024)
#define BSP_CAMERA_INTERFACE_INDEX          1
#define BSP_CAMERA_INTERFACE_ALT_INDEX      0
#define BSP_CAMERA_EP_MPS                   64

static char *TAG = "bsp_camera";
static bsp_camera_frame_cb_t frame_cb = NULL;

static void on_frame_cb(uvc_frame_t *frame, void *ptr);

static void *_malloc(size_t size)
{
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
}

esp_err_t bsp_camera_init(void)
{
    /* malloc double buffer for usb payload, xfer_buffer_size >= frame_buffer_size*/
    uint8_t *xfer_buffer_a = (uint8_t *)_malloc(BSP_CAMERA_XFER_BUFFER_SIZE);
    assert(xfer_buffer_a != NULL);
    uint8_t *xfer_buffer_b = (uint8_t *)_malloc(BSP_CAMERA_XFER_BUFFER_SIZE);
    assert(xfer_buffer_b != NULL);

    /* malloc frame buffer for a jpeg frame*/
    uint8_t *frame_buffer = (uint8_t *)_malloc(BSP_CAMERA_XFER_BUFFER_SIZE);
    assert(frame_buffer != NULL);

    /* the quick demo skip the standard get descriptors process,
    users need to get params from camera descriptors,
    run the example first to printf the log with descriptors */
    uvc_config_t uvc_config = {
        .dev_speed = USB_SPEED_FULL,
        .xfer_type = BSP_CAMERA_XFER_MODE,
        .configuration = DESCRIPTOR_CONFIGURATION_INDEX,
        .format_index = DESCRIPTOR_FORMAT_MJPEG_INDEX,
        .frame_width = CONFIG_BSP_CAMERA_FRAME_WIDTH,
        .frame_height = CONFIG_BSP_CAMERA_FRAME_HEIGHT,
        .frame_index = CONFIG_BSP_CAMERA_FRAME_INDEX,
        .frame_interval = CONFIG_BSP_CAMERA_FRAME_INTERVAL,
        .interface = BSP_CAMERA_INTERFACE_INDEX,
        .interface_alt = BSP_CAMERA_INTERFACE_ALT_INDEX,
        .isoc_ep_addr = DESCRIPTOR_STREAM_ENDPOINT_ADDR,
        .isoc_ep_mps = BSP_CAMERA_EP_MPS,
        .xfer_buffer_size = BSP_CAMERA_XFER_BUFFER_SIZE,
        .xfer_buffer_a = xfer_buffer_a,
        .xfer_buffer_b = xfer_buffer_b,
        .frame_buffer_size = BSP_CAMERA_XFER_BUFFER_SIZE,
        .frame_buffer = frame_buffer,
    };

    /* pre-config UVC driver with params from known USB Camera Descriptors*/
    esp_err_t ret = uvc_streaming_config(&uvc_config);

    /* Start camera IN stream with pre-configs, uvc driver will create multi-tasks internal
    to handle usb data from different pipes, and user's callback will be called after new frame ready. */
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "uvc streaming config failed");
    } else {
        ret = uvc_streaming_start(on_frame_cb, NULL);
        if (ret != ESP_OK) {
            ESP_LOGI(TAG, "uvc streaming start failed");
        }
        else {
            ESP_LOGI(TAG, "uvc streaming start");
        }
    }

    return ret;
}

void bsp_camera_frame_cb_register(bsp_camera_frame_cb_t cb)
{
    frame_cb = cb;
}

static void on_frame_cb(uvc_frame_t *frame, void *ptr)
{
#if CONFIG_BSP_CAMERA_LOG
    ESP_LOGI(TAG, "callback! frame_format = %d, seq = %u, width = %d, height = %d, length = %u, ptr = %d",
            frame->frame_format, frame->sequence, frame->width, frame->height, frame->data_bytes, (int) ptr);
#endif

    switch (frame->frame_format) {
        case UVC_FRAME_FORMAT_MJPEG:
            if (frame_cb) {
                frame_cb(frame->data, frame->data_bytes);
            }
#if CONFIG_BSP_CAMERA_LOG
            ESP_LOGI(TAG, "frame done");
#endif
            break;
        default:
            ESP_LOGW(TAG, "format not supported");
            assert(0);
            break;
    }
}
