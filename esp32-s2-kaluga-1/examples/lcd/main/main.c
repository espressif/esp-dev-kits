/* LCD example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/spi_master.h"
#include "driver/gpio.h"

#include "esp_system.h"
#include "esp_log.h"
#include "esp_err.h"
#include "pretty_effect.h"
#include "lcd.h"

/*
 This code displays some fancy graphics on the 320x240 LCD on an ESP32-S2-KALUGA board.
 This example demonstrates the use of both spi_device_transmit as well as
 spi_device_queue_trans/spi_device_get_trans_result and pre-transmit callbacks.

 Some info about the ILI9341/ST7789V: It has an C/D line, which is connected to a GPIO here. It expects this
 line to be low for a command and high for data. We use a pre-transmit callback here to control that
 line: every transaction has as the user-definable argument the needed state of the D/C line and just
 before the transaction is sent, the callback will set this line to the correct state.
*/

/*!< Reference the binary-included jpeg fil */
/*!< image.jpg == _binary_image_jpg_start */
extern const uint8_t image_jpg_start[] asm("_binary_image_jpg_start");
extern const uint8_t image_jpg_end[] asm("_binary_image_jpg_end");
/*!< Define the height and width of the jpeg file. Make sure this matches the actual jpeg dimensions*/

#define IMAGE_W 336 /*!< width of jpeg file */
#define IMAGE_H 256 /*!< height of jpeg file */

static const char *TAG = "main";

void app_main(void)
{
    spi_device_handle_t spi = {0};
    spi_bus_config_t bus_cfg = {
        .miso_io_num     = PIN_NUM_MISO,
        .mosi_io_num     = PIN_NUM_MOSI,
        .sclk_io_num     = PIN_NUM_CLK,
        .quadwp_io_num   = -1,
        .quadhd_io_num   = -1,
        .max_transfer_sz = PARALLEL_LINES * 320 * 2 + 8
    };
    spi_device_interface_config_t devcfg = {
#ifdef CONFIG_LCD_OVERCLOCK
        .clock_speed_hz = 26 * 1000 * 1000,              /*!< Clock out at 26 MHz */
#else
        .clock_speed_hz = 10 * 1000 * 1000,              /*!< Clock out at 10 MHz */
#endif
        .mode           = 0,                             /*!< SPI mode 0 */
        .spics_io_num   = PIN_NUM_CS,                    /*!< CS pin */
        .queue_size     = 7,                             /*!< We want to be able to queue 7 transactions at a time */
        .pre_cb         = lcd_spi_pre_transfer_callback, /*!< Specify pre-transfer callback to handle D/C line */
    };
    /*!< Initialize the SPI bus */
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &bus_cfg, DMA_CHAN));
    /*!< Attach the LCD to the SPI bus */
    ESP_ERROR_CHECK(spi_bus_add_device(LCD_HOST, &devcfg, &spi));
    /**< Initialize the LCD */
    ESP_ERROR_CHECK(lcd_init(spi));

    /*!< Initialize and Decode the image , cache it in memory */
    ESP_LOGI(TAG, "the memory get: %d", esp_get_free_heap_size());
    ESP_ERROR_CHECK(pretty_effect_init(image_jpg_start, IMAGE_W, IMAGE_H));
    ESP_LOGI(TAG ,"the memory get: %d", esp_get_free_heap_size());

    /*!< Displays static or dynamic image. */
    display_pretty_colors(spi);
}
