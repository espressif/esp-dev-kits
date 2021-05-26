/* Camera Example

    This example code is in the Public Domain (or CC0 licensed, at your option.)
    Unless required by applicable law or agreed to in writing, this
    software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
    CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_camera.h"
#include "lcd.h"
#include "board.h"

static const char *TAG = "main";


void app_main()
{
    lcd_config_t lcd_config = {
        .clk_fre         = 40 * 1000 * 1000,
        .pin_clk         = LCD_CLK,
        .pin_mosi        = LCD_MOSI,
        .pin_dc          = LCD_DC,
        .pin_cs          = LCD_CS,
        .pin_rst         = LCD_RST,
        .pin_bk          = LCD_BK,
        .max_buffer_size = 2 * 1024,
        .horizontal      = 2, /*!< 2: UP, 3: DOWN */
#ifdef CONFIG_CAMERA_JPEG_MODE
        .swap_data       = 1,
#else
        .swap_data       = 0,
#endif
    };

    lcd_init(&lcd_config);

    camera_config_t camera_config = {
        .pin_pwdn = -1,
        .pin_reset = -1,
        .pin_xclk = CAM_XCLK,
        .pin_sscb_sda = CAM_SDA,
        .pin_sscb_scl = CAM_SCL,

        .pin_d7 = CAM_D7,
        .pin_d6 = CAM_D6,
        .pin_d5 = CAM_D5,
        .pin_d4 = CAM_D4,
        .pin_d3 = CAM_D3,
        .pin_d2 = CAM_D2,
        .pin_d1 = CAM_D1,
        .pin_d0 = CAM_D0,
        .pin_vsync = CAM_VSYNC,
        .pin_href = CAM_HSYNC,
        .pin_pclk = CAM_PCLK,

        //XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
        .xclk_freq_hz = 20000000,
        .ledc_timer = LEDC_TIMER_0,
        .ledc_channel = LEDC_CHANNEL_0,
#ifdef CONFIG_CAMERA_JPEG_MODE
        .pixel_format = PIXFORMAT_JPEG, //YUV422,GRAYSCALE,RGB565,JPEG
#else
        .pixel_format = PIXFORMAT_RGB565,
#endif
        .frame_size = FRAMESIZE_QVGA,    //QQVGA-UXGA Do not use sizes above QVGA when not JPEG
        .jpeg_quality = 12, //0-63 lower number means higher quality
        .fb_count = 2       //if more than one, i2s runs in continuous mode. Use only with JPEG
    };
    //initialize the camera
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera Init Failed");
        return;
    }
    sensor_t *s = esp_camera_sensor_get();
    s->set_vflip(s, 1);//flip it back
    //initial sensors are flipped vertically and colors are a bit saturated
    if (s->id.PID == OV3660_PID) {
        s->set_brightness(s, 1);//up the blightness just a bit
        s->set_saturation(s, -2);//lower the saturation
    }
    //drop down frame size for higher initial frame rate
    s->set_framesize(s, FRAMESIZE_QVGA);
    ESP_LOGI(TAG, "Camera Init done");

#ifdef CONFIG_CAMERA_JPEG_MODE
    ESP_LOGI(TAG, "Camera jpeg mode");
    uint8_t *rgb565 = malloc(240 * 320 * 2);
    if (NULL == rgb565) {
        ESP_LOGE(TAG, "can't alloc memory for rgb565 buffer");
        return;
    }
#endif
    while (1) {
        camera_fb_t *pic = esp_camera_fb_get();
        if (pic) {
            ESP_LOGI(TAG, "picture: %d x %d %dbyte", pic->width, pic->height, pic->len);
            lcd_set_index(0, 0, pic->width - 1, pic->height - 1);
#ifdef CONFIG_CAMERA_JPEG_MODE
            jpg2rgb565(pic->buf, pic->len, rgb565, JPG_SCALE_NONE);
            lcd_write_data(rgb565, 2 * (pic->width * pic->height));
#else
            lcd_write_data(pic->buf, pic->len);
#endif
            esp_camera_fb_return(pic);
        } else {
            ESP_LOGE(TAG, "Get frame failed");
        }
    }

#ifdef CONFIG_CAMERA_JPEG_MODE
    free(rgb565);
#endif
}
