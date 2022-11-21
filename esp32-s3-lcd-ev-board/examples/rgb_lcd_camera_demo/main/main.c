#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_timer.h"
#include "esp_jpeg_dec.h"
#include "esp_event.h"
#include "bsp_lcd.h"
#include "bsp_camera.h"
#include "basic_painter.h"
#include "painter_fonts.h"
#include "protocol_examples_common.h"

static const char *TAG = "app_main";
static uint8_t *frame_buf1 = NULL;
static uint8_t *frame_buf2 = NULL;
static uint8_t *frame_cur_buf = NULL;
static esp_lcd_panel_handle_t panel_handle;

static void bsp_camera_frame_cb(void *frame_data, int frame_size);
static int esp_jpeg_decoder_one_picture(uint8_t *input_buf, int len, uint8_t *output_buf);
static esp_err_t painter_draw_bitmap(int x1, int y1, int w, int h, const void *p_data);

void wifi_task(void *arg)
{
    vTaskDelay(pdMS_TO_TICKS(2000));

    if (nvs_flash_init() != ESP_OK) {
        nvs_flash_erase();
        nvs_flash_init();
    }
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    example_connect();

    vTaskDelete(NULL);
}

void app_main(void)
{
    bsp_lcd_init();
    panel_handle = (esp_lcd_panel_handle_t)bsp_lcd_get_panel_handle();

    bsp_camera_init();

    painter_config_t config = {
        .info = {
            .height = BSP_LCD_HEIGHT,
            .width = BSP_LCD_WIDTH,
        },
        .draw_bitmap = painter_draw_bitmap,
    };
    painter_init(&config);
    painter_clear(0xf800);

    esp_lcd_rgb_panel_get_frame_buffer(panel_handle, 2, (void **)&frame_buf1, (void **)&frame_buf2);
    frame_cur_buf = frame_buf2;
    bsp_camera_frame_cb_register(bsp_camera_frame_cb);

    // Just for test the performance of RGB LCD when using Wifi
    xTaskCreatePinnedToCore(wifi_task, "wifi", 4096, NULL, 3, NULL, 1);
}

static void bsp_camera_frame_cb(void *frame_data, int frame_size)
{
    static int frame_count = 0;
    static int64_t count_start_time = 0;
    static int fps = 0;
    static char fps_str[20];

    if (frame_count == 0) {
        count_start_time = esp_timer_get_time();
    }

    esp_jpeg_decoder_one_picture((uint8_t *)frame_data, frame_size, frame_cur_buf);
    esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, 1, 1, frame_cur_buf);
    frame_cur_buf = (frame_cur_buf == frame_buf1) ? frame_buf2 : frame_buf1;
    if (++frame_count == 20) {
        frame_count = 0;
        fps = 20 * 1000000 / (esp_timer_get_time() - count_start_time);
    }
    if (fps != 0) {
        snprintf(fps_str, sizeof(fps_str), "FPS: %d", fps);
        ESP_LOGI(TAG, "fps: %d", fps);
        painter_draw_string(0, 0, fps_str, &Font24, 0);
    }
}

 static int esp_jpeg_decoder_one_picture(uint8_t *input_buf, int len, uint8_t *output_buf)
 {
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

      int ret = 0;
      // Parse jpeg picture header and get picture for user and decoder
      ret = jpeg_dec_parse_header(jpeg_dec, jpeg_io, out_info);
      if (ret < 0) {
          return ret;
      }

      jpeg_io->outbuf = output_buf;
      int inbuf_consumed = jpeg_io->inbuf_len - jpeg_io->inbuf_remain;
      jpeg_io->inbuf = input_buf + inbuf_consumed;
      jpeg_io->inbuf_len = jpeg_io->inbuf_remain;

      // Start decode jpeg raw data
      ret = jpeg_dec_process(jpeg_dec, jpeg_io);
      if (ret < 0) {
          return ret;
      }

      // Decoder deinitialize
      jpeg_dec_close(jpeg_dec);
      free(out_info);
      free(jpeg_io);
      return ESP_OK;
 }

static esp_err_t painter_draw_bitmap(int x1, int y1, int w, int h, const void *p_data)
{
    esp_lcd_panel_draw_bitmap(panel_handle, x1, y1, x1 + w, y1 + h, p_data);

    return ESP_OK;
}
