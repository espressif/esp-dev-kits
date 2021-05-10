/**
 * @file bsp_lcd.c
 * @brief LCD initialize and basic operator.
 * @version 0.1
 * @date 2021-01-14
 * 
 * @copyright Copyright 2021 Espressif Systems (Shanghai) Co. Ltd.
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *               http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#include "bsp_lcd.h"

static const char *TAG = "bsp_lcd";

static scr_driver_t lcd;
static scr_info_t lcd_info;

esp_err_t bsp_lcd_init(void)
{
    /* Reset LCD before initialize */
    tca9554_init();
    ext_io_t io_conf = BSP_EXT_IO_DEFAULT_CONFIG();
    ext_io_t io_level = BSP_EXT_IO_DEFAULT_LEVEL();
    io_level.lcd_rst = 0;
    tca9554_set_configuration(io_conf.val);
    tca9554_write_output_pins(io_level.val);
    vTaskDelay(pdMS_TO_TICKS(20));
    io_level.lcd_rst = 1;
    tca9554_write_output_pins(io_level.val);

    /* Initialize LCD */
    scr_interface_driver_t *iface_drv_i2s;
    scr_controller_config_t lcd_cfg = {0};
    i2s_lcd_config_t i2s_lcd_cfg = {
        .data_width  = LCD_BIT_WIDTH,
        .pin_data_num = {
            GPIO_LCD_D00, GPIO_LCD_D01, GPIO_LCD_D02, GPIO_LCD_D03,
            GPIO_LCD_D04, GPIO_LCD_D05, GPIO_LCD_D06, GPIO_LCD_D07,
            GPIO_LCD_D08, GPIO_LCD_D09, GPIO_LCD_D10, GPIO_LCD_D11,
            GPIO_LCD_D12, GPIO_LCD_D13, GPIO_LCD_D14, GPIO_LCD_D15,
        },
        .pin_num_cs = GPIO_LCD_CS,
        .pin_num_wr = GPIO_LCD_WR,
        .pin_num_rs = GPIO_LCD_RS,
        .clk_freq = 20000000,
        .i2s_port = I2S_NUM_0,
        .buffer_size = 32000,
        .swap_data = false,
    };

    scr_interface_create(SCREEN_IFACE_8080, &i2s_lcd_cfg, &iface_drv_i2s);

    lcd_cfg.interface_drv = iface_drv_i2s,
    lcd_cfg.pin_num_rst = GPIO_LCD_EN,
    lcd_cfg.pin_num_bckl = GPIO_LCD_BL,
    lcd_cfg.rst_active_level = 0,
    lcd_cfg.bckl_active_level = 0,
    lcd_cfg.width = 480;
    lcd_cfg.height = 800;
    lcd_cfg.rotate = SCR_DIR_TBRL;
    scr_find_driver(SCREEN_CONTROLLER_RM68120, &lcd);
    lcd.init(&lcd_cfg);

    /* Print LCD infomation */
    lcd.get_info(&lcd_info);
    ESP_LOGI(TAG, "Screen name : [%s] | width : [%d] | height : [%d]", lcd_info.name, lcd_info.width, lcd_info.height);

    return ESP_OK;
}

esp_err_t bsp_lcd_flush(uint16_t x, uint16_t y, uint16_t w, uint16_t h, void *data)
{
    return lcd.draw_bitmap(x, y, w, h, (uint16_t *)data);
}
