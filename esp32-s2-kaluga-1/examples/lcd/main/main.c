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
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "lcd.h"
#include "esp_camera.h"
#include "board.h"

static const char *TAG = "main";

#define IMAGE_MAX_SIZE (100 * 1024)/**< The maximum size of a single picture in the boot animation */
#define IMAGE_WIDTH    320 /*!< width of jpeg file */
#define IMAGE_HIGHT    240 /*!< height of jpeg file */

/**
 * @brief rgb -> rgb565
 *
 * @param r red   (0~31)
 * @param g green (0~63)
 * @param b red   (0~31)
 *
 * @return data about color565
 */
uint16_t color565(uint8_t r, uint8_t g, uint8_t b)
{
    uint16_t color = ((r  << 11) | (g  << 6) | b);
    return (color << 8) | (color >> 8);
}

void esp_photo_display(void)
{
    ESP_LOGI(TAG, "LCD photo test....");
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = false
    };

    /*!< Use settings defined above to initialize and mount SPIFFS filesystem. */
    /*!< Note: esp_vfs_spiffs_register is an all-in-one convenience function. */
    ESP_ERROR_CHECK(esp_vfs_spiffs_register(&conf));
    size_t total = 0, used = 0;
    ESP_ERROR_CHECK(esp_spiffs_info(NULL, &total, &used));

    uint8_t *rgb565 = malloc(IMAGE_WIDTH * IMAGE_HIGHT * 2);
    if (NULL == rgb565) {
        ESP_LOGE(TAG, "can't alloc memory for rgb565 buffer");
        return;
    }
    uint8_t *buf = malloc(IMAGE_MAX_SIZE);
    if (NULL == buf) {
        free(rgb565);
        ESP_LOGE(TAG, "can't alloc memory for jpeg file buffer");
        return;
    }
    int read_bytes = 0;

    FILE *fd = fopen("/spiffs/image.jpg", "r");

    read_bytes = fread(buf, 1, IMAGE_MAX_SIZE, fd);
    ESP_LOGI(TAG, "spiffs:read_bytes:%d  fd: %p", read_bytes, fd);
    fclose(fd);

    jpg2rgb565(buf, read_bytes, rgb565, JPG_SCALE_NONE);
    lcd_set_index(0, 0, IMAGE_WIDTH - 1, IMAGE_HIGHT - 1);
    lcd_write_data(rgb565, IMAGE_WIDTH * IMAGE_HIGHT * sizeof(uint16_t));
    free(buf);
    free(rgb565);
    vTaskDelay(2000 / portTICK_RATE_MS);
}

void esp_color_display(void)
{
    ESP_LOGI(TAG, "LCD color test....");
    uint16_t *data_buf = (uint16_t *)heap_caps_calloc(IMAGE_WIDTH * IMAGE_HIGHT, sizeof(uint16_t), MALLOC_CAP_SPIRAM);

    while (1) {
        uint16_t color = color565(0, 0, 0);

        for (int r = 0,  j = 0; j < IMAGE_HIGHT; j++) {
            if (j % 8 == 0) {
                color = color565(r++, 0, 0);
            }

            for (int i = 0; i < IMAGE_WIDTH; i++) {
                data_buf[i + IMAGE_WIDTH * j] = color;
            }
        }

        lcd_set_index(0, 0, IMAGE_WIDTH - 1, IMAGE_HIGHT - 1);
        lcd_write_data((uint8_t *)data_buf, IMAGE_WIDTH * IMAGE_HIGHT * sizeof(uint16_t));
        vTaskDelay(2000 / portTICK_RATE_MS);

        for (int g = 0,  j = 0; j < IMAGE_HIGHT; j++) {
            if (j % 8 == 0) {
                color = color565(0, g++, 0);
            }

            for (int i = 0; i < IMAGE_WIDTH; i++) {
                data_buf[i + IMAGE_WIDTH * j] = color;
            }
        }

        lcd_set_index(0, 0, IMAGE_WIDTH - 1, IMAGE_HIGHT - 1);
        lcd_write_data((uint8_t *)data_buf, IMAGE_WIDTH * IMAGE_HIGHT * sizeof(uint16_t));
        vTaskDelay(2000 / portTICK_RATE_MS);

        for (int b = 0,  j = 0; j < IMAGE_HIGHT; j++) {
            if (j % 8 == 0) {
                color = color565(0, 0, b++);
            }

            for (int i = 0; i < IMAGE_WIDTH; i++) {
                data_buf[i + IMAGE_WIDTH * j] = color;
            }
        }

        lcd_set_index(0, 0, IMAGE_WIDTH - 1, IMAGE_HIGHT - 1);
        lcd_write_data((uint8_t *)data_buf, IMAGE_WIDTH * IMAGE_HIGHT * sizeof(uint16_t));
        vTaskDelay(2000 / portTICK_RATE_MS);

    }
}

void app_main()
{
    lcd_config_t lcd_config = {
#ifdef CONFIG_LCD_ST7789
        .clk_fre         = 80 * 1000 * 1000, /*!< ILI9341 Stable frequency configuration */
#endif
#ifdef CONFIG_LCD_ILI9341
        .clk_fre         = 40 * 1000 * 1000, /*!< ILI9341 Stable frequency configuration */
#endif
        .pin_clk         = LCD_CLK,
        .pin_mosi        = LCD_MOSI,
        .pin_dc          = LCD_DC,
        .pin_cs          = LCD_CS,
        .pin_rst         = LCD_RST,
        .pin_bk          = LCD_BK,
        .max_buffer_size = 2 * 1024,
        .horizontal      = 2, /*!< 2: UP, 3: DOWN */
        .swap_data       = 1,
    };

    lcd_init(&lcd_config);

    /*< Show a picture */
    esp_photo_display();
    /*< RGB display */
    esp_color_display();

}

