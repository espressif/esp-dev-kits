#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/spi_master.h"
#include "esp_check.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_log.h"

#include "lcd_panel_gc9a01.h"
#include "bsp_lcd.h"

#define LCD_HOST                (SPI2_HOST)

#define LCD_PIXEL_CLOCK_HZ      (80 * 1000 * 1000)
// #define LCD_BK_LIGHT_ON_LEVEL   (0)
// #define LCD_BK_LIGHT_OFF_LEVEL  (!LCD_BK_LIGHT_ON_LEVEL)

#define PIN_NUM_LCD_CS          (GPIO_NUM_10)
#define PIN_NUM_LCD_SCLK        (GPIO_NUM_1)
#define PIN_NUM_LCD_DC          (GPIO_NUM_4)
#define PIN_NUM_LCD_MOSI        (GPIO_NUM_0)
#define PIN_NUM_LCD_MISO        (GPIO_NUM_NC)
#define PIN_NUM_LCD_RST         (GPIO_NUM_2)

// Bit number used to represent command and parameter
#define LCD_CMD_BITS           (8)
#define LCD_PARAM_BITS         (8)

#define LEDC_TIMER              (LEDC_TIMER_0)
#define LEDC_MODE               (LEDC_LOW_SPEED_MODE)
#define LEDC_OUTPUT_IO          (GPIO_NUM_5) // Define the output GPIO
#define LEDC_CHANNEL            (LEDC_CHANNEL_0)
#define LEDC_DUTY_RES           (LEDC_TIMER_13_BIT) // Set duty resolution to 13 bits
#define LEDC_FREQUENCY          (5000) // Frequency in Hertz. Set frequency at 5 kHz

static char *TAG = "bsp_lcd";
static esp_lcd_panel_handle_t panel_handle = NULL;
static bsp_lcd_trans_done_cb_t on_trans_done = NULL;

static bool bsp_lcd_on_trans_done(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx);

esp_lcd_panel_handle_t bsp_lcd_init(void)
{
    ESP_LOGI(TAG, "Initialize SPI bus");
    spi_bus_config_t buscfg = {
        .sclk_io_num = PIN_NUM_LCD_SCLK,
        .mosi_io_num = PIN_NUM_LCD_MOSI,
        .miso_io_num = PIN_NUM_LCD_MISO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LCD_H_RES * LCD_V_RES * sizeof(uint16_t) + 10,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));

    ESP_LOGI(TAG, "Install panel IO");
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = PIN_NUM_LCD_DC,
        .cs_gpio_num = PIN_NUM_LCD_CS,
        .pclk_hz = LCD_PIXEL_CLOCK_HZ,
        .lcd_cmd_bits = LCD_CMD_BITS,
        .lcd_param_bits = LCD_PARAM_BITS,
        .spi_mode = 0,
        .trans_queue_depth = 10,
        .on_color_trans_done = bsp_lcd_on_trans_done,
        .user_ctx = NULL,
    };
    // Attach the LCD to the SPI bus
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle));

    ESP_LOGI(TAG, "Install GC9A01 panel driver");
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = PIN_NUM_LCD_RST,
        .color_space = ESP_LCD_COLOR_SPACE_RGB,
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(lcd_new_panel_gc9a01(io_handle, &panel_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
#else
    ESP_ERROR_CHECK(esp_lcd_panel_disp_off(panel_handle, false));
#endif

    if (LEDC_OUTPUT_IO != GPIO_NUM_NC) {
        ESP_LOGI(TAG, "Turn on LCD backlight");
        // Prepare and then apply the LEDC PWM timer configuration
        ledc_timer_config_t ledc_timer = {
            .speed_mode       = LEDC_MODE,
            .timer_num        = LEDC_TIMER,
            .duty_resolution  = LEDC_DUTY_RES,
            .freq_hz          = LEDC_FREQUENCY,  // Set output frequency at 5 kHz
            .clk_cfg          = LEDC_AUTO_CLK
        };
        ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

        // Prepare and then apply the LEDC PWM channel configuration
        ledc_channel_config_t ledc_channel = {
            .speed_mode     = LEDC_MODE,
            .channel        = LEDC_CHANNEL,
            .timer_sel      = LEDC_TIMER,
            .intr_type      = LEDC_INTR_DISABLE,
            .gpio_num       = LEDC_OUTPUT_IO,
            .duty           = 0,
            .hpoint         = 0
        };
        ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
        ESP_ERROR_CHECK(ledc_fade_func_install(0));

        bsp_lcd_set_brightness(0);
    }

    return panel_handle;
}

void bsp_lcd_trans_done_cb_register(bsp_lcd_trans_done_cb_t cb)
{
    on_trans_done = cb;
}

void bsp_lcd_set_brightness(uint8_t percent)
{
    percent = (percent > 100) ? 100 : percent;
    percent = 100 - percent;
    uint32_t duty = BIT(LEDC_DUTY_RES) * percent / 100;
    ESP_ERROR_CHECK(ledc_set_duty_and_update(LEDC_MODE, LEDC_CHANNEL, duty, 0));
}

static bool bsp_lcd_on_trans_done(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    if (on_trans_done) {
        return on_trans_done();
    }

    return false;
}
