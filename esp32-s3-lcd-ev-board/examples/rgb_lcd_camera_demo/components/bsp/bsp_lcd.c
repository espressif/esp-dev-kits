#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_check.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_lcd_panel_ops.h"
#include "bsp_lcd.h"

static char *TAG = "bsp_lcd";
static esp_lcd_panel_handle_t panel_handle = NULL;
static bsp_lcd_frame_callback_t on_vsync_cb;

static bool on_vsync_event(esp_lcd_panel_handle_t panel, const esp_lcd_rgb_panel_event_data_t *edata, void *user_ctx);

void bsp_lcd_init(void)
{
    esp_lcd_rgb_panel_config_t panel_config = {
        .clk_src = LCD_CLK_SRC_PLL160M,
        .data_width = 16,
        .psram_trans_align = 64,
        .disp_gpio_num = BSP_LCD_DISP_EN_GPIO,
        .pclk_gpio_num = BSP_LCD_PCLK_GPIO,
        .vsync_gpio_num = BSP_LCD_VSYNC_GPIO,
        .hsync_gpio_num = BSP_LCD_HSYNC_GPIO,
        .de_gpio_num = BSP_LCD_DE_GPIO,
        .data_gpio_nums = {
            BSP_LCD_DATA0_GPIO,
            BSP_LCD_DATA1_GPIO,
            BSP_LCD_DATA2_GPIO,
            BSP_LCD_DATA3_GPIO,
            BSP_LCD_DATA4_GPIO,
            BSP_LCD_DATA5_GPIO,
            BSP_LCD_DATA6_GPIO,
            BSP_LCD_DATA7_GPIO,
            BSP_LCD_DATA8_GPIO,
            BSP_LCD_DATA9_GPIO,
            BSP_LCD_DATA10_GPIO,
            BSP_LCD_DATA11_GPIO,
            BSP_LCD_DATA12_GPIO,
            BSP_LCD_DATA13_GPIO,
            BSP_LCD_DATA14_GPIO,
            BSP_LCD_DATA15_GPIO,
        },
        .timings = {
            .pclk_hz = BSP_LCD_PCLK,
            .h_res = BSP_LCD_WIDTH,
            .v_res = BSP_LCD_HEIGHT,
            .hsync_back_porch = BSP_LCD_HSYC_BACK_PORCH,
            .hsync_front_porch = BSP_LCD_HSYC_FRONT_PORCH,
            .hsync_pulse_width = BSP_LCD_HSYC_PLUS_WIDTH,
            .vsync_back_porch = BSP_LCD_VSYC_BACK_PORCH,
            .vsync_front_porch = BSP_LCD_VSYC_FRONT_PORCH,
            .vsync_pulse_width = BSP_LCD_VSYC_PLUS_WIDTH,
            .flags.pclk_active_neg = BSP_LCD_PCLK_ACTIVE_NEG,
        },
        .flags.fb_in_psram = 1,
        .flags.double_fb = 1,
        // .flags.refresh_on_demand = 1,   // Mannually control refresh operation
    };
    ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&panel_config, &panel_handle));
    esp_lcd_rgb_panel_event_callbacks_t cbs = {
        .on_vsync = on_vsync_event,
    };
    ESP_ERROR_CHECK(esp_lcd_rgb_panel_register_event_callbacks(panel_handle, &cbs, NULL));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
}

void *bsp_lcd_get_panel_handle(void)
{
    return (void *)panel_handle;
}

void bsp_lcd_frame_cb_register(bsp_lcd_frame_callback_t cb)
{
    on_vsync_cb = cb;
}

static bool on_vsync_event(
    esp_lcd_panel_handle_t panel,
    const esp_lcd_rgb_panel_event_data_t *edata,
    void *user_ctx
)
{
    if (on_vsync_cb)
        return on_vsync_cb();
    else
        return false;
}
