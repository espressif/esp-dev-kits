/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <sys/cdefs.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_lcd_panel_interface.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_commands.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_check.h"

static const char *TAG = "st7796";

static esp_err_t panel_st7796_del(esp_lcd_panel_t *panel);
static esp_err_t panel_st7796_reset(esp_lcd_panel_t *panel);
static esp_err_t panel_st7796_init(esp_lcd_panel_t *panel);
static esp_err_t panel_st7796_draw_bitmap(esp_lcd_panel_t *panel, int x_start, int y_start, int x_end, int y_end, const void *color_data);
static esp_err_t panel_st7796_invert_color(esp_lcd_panel_t *panel, bool invert_color_data);
static esp_err_t panel_st7796_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y);
static esp_err_t panel_st7796_swap_xy(esp_lcd_panel_t *panel, bool swap_axes);
static esp_err_t panel_st7796_set_gap(esp_lcd_panel_t *panel, int x_gap, int y_gap);
static esp_err_t panel_st7796_disp_on_off(esp_lcd_panel_t *panel, bool off);

typedef struct {
    esp_lcd_panel_t base;
    esp_lcd_panel_io_handle_t io;
    int reset_gpio_num;
    bool reset_level;
    int x_gap;
    int y_gap;
    unsigned int bits_per_pixel;
    uint8_t madctl_val; // save current value of LCD_CMD_MADCTL register
    uint8_t colmod_cal; // save surrent value of LCD_CMD_COLMOD register
} st7796_panel_t;

esp_err_t esp_lcd_new_panel_st7796(const esp_lcd_panel_io_handle_t io, const esp_lcd_panel_dev_config_t *panel_dev_config, esp_lcd_panel_handle_t *ret_panel)
{
    esp_err_t ret = ESP_OK;
    st7796_panel_t *st7796 = NULL;
    ESP_GOTO_ON_FALSE(io && panel_dev_config && ret_panel, ESP_ERR_INVALID_ARG, err, TAG, "invalid argument");
    st7796 = calloc(1, sizeof(st7796_panel_t));
    ESP_GOTO_ON_FALSE(st7796, ESP_ERR_NO_MEM, err, TAG, "no mem for st7796 panel");

    if (panel_dev_config->reset_gpio_num >= 0) {
        gpio_config_t io_conf = {
            .mode = GPIO_MODE_OUTPUT,
            .pin_bit_mask = 1ULL << panel_dev_config->reset_gpio_num,
        };
        ESP_GOTO_ON_ERROR(gpio_config(&io_conf), err, TAG, "configure GPIO for RST line failed");
    }

    switch (panel_dev_config->color_space) {
    case ESP_LCD_COLOR_SPACE_RGB:
        st7796->madctl_val |= LCD_CMD_BGR_BIT;
        break;
    case ESP_LCD_COLOR_SPACE_BGR:
        st7796->madctl_val = 0;
        break;
    default:
        ESP_GOTO_ON_FALSE(false, ESP_ERR_NOT_SUPPORTED, err, TAG, "unsupported color space");
        break;
    }

    switch (panel_dev_config->bits_per_pixel) {
    case 16:
        st7796->colmod_cal = 0x55;
        break;
    case 18:
        st7796->colmod_cal = 0x66;
        break;
    default:
        ESP_GOTO_ON_FALSE(false, ESP_ERR_NOT_SUPPORTED, err, TAG, "unsupported pixel width");
        break;
    }

    st7796->io = io;
    st7796->bits_per_pixel = panel_dev_config->bits_per_pixel;
    st7796->reset_gpio_num = panel_dev_config->reset_gpio_num;
    st7796->reset_level = panel_dev_config->flags.reset_active_high;
    st7796->base.del = panel_st7796_del;
    st7796->base.reset = panel_st7796_reset;
    st7796->base.init = panel_st7796_init;
    st7796->base.draw_bitmap = panel_st7796_draw_bitmap;
    st7796->base.invert_color = panel_st7796_invert_color;
    st7796->base.set_gap = panel_st7796_set_gap;
    st7796->base.mirror = panel_st7796_mirror;
    st7796->base.swap_xy = panel_st7796_swap_xy;
    st7796->base.disp_on_off = panel_st7796_disp_on_off;
    *ret_panel = &(st7796->base);
    ESP_LOGD(TAG, "new st7796 panel @%p", st7796);

    return ESP_OK;

err:
    if (st7796) {
        if (panel_dev_config->reset_gpio_num >= 0) {
            gpio_reset_pin(panel_dev_config->reset_gpio_num);
        }
        free(st7796);
    }
    return ret;
}

static esp_err_t panel_st7796_del(esp_lcd_panel_t *panel)
{
    st7796_panel_t *st7796 = __containerof(panel, st7796_panel_t, base);

    if (st7796->reset_gpio_num >= 0) {
        gpio_reset_pin(st7796->reset_gpio_num);
    }
    ESP_LOGD(TAG, "del st7796 panel @%p", st7796);
    free(st7796);
    return ESP_OK;
}

static esp_err_t panel_st7796_reset(esp_lcd_panel_t *panel)
{
    st7796_panel_t *st7796 = __containerof(panel, st7796_panel_t, base);
    esp_lcd_panel_io_handle_t io = st7796->io;

    // perform hardware reset
    if (st7796->reset_gpio_num >= 0) {
        gpio_set_level(st7796->reset_gpio_num, st7796->reset_level);
        vTaskDelay(pdMS_TO_TICKS(10));
        gpio_set_level(st7796->reset_gpio_num, !st7796->reset_level);
        vTaskDelay(pdMS_TO_TICKS(10));
    } else { // perform software reset
        esp_lcd_panel_io_tx_param(io, LCD_CMD_SWRESET, NULL, 0);
        vTaskDelay(pdMS_TO_TICKS(20)); // spec, wait at least 5ms before sending new command
    }

    return ESP_OK;
}

typedef struct {
    uint8_t cmd;
    uint8_t data[16];
    uint8_t data_bytes; // Length of data in above data array; 0xFF = end of cmds.
} lcd_init_cmd_t;

static const lcd_init_cmd_t vendor_specific_init[] = {
    // Enable Inter Register
    {0xf0, {0xc3}, 1},
    {0xf0, {0x96}, 1},
    {0x36, {0x28}, 1},
    {0x3a, {0x55}, 1},
    {0xb4, {0x01}, 1},
    {0xb7, {0xc6}, 1},
    {0xe8, {0x40, 0x8a, 0x00, 0x00, 0x29, 0x19, 0xa5, 0x33}, 8},
    {0xc1, {0x06}, 1},
    {0xc2, {0xa7}, 1},
    {0xc5, {0x18}, 1},
    {0xe0, {0xf0, 0x09, 0x0b, 0x06, 0x04, 0x15, 0x2f, 0x54, 0x42, 0x3c, 0x17, 0x14, 0x18, 0x1b}, 14},
    {0xe1, {0xf0, 0x09, 0x0b, 0x06, 0x04, 0x03, 0x2d, 0x43, 0x42, 0x3b, 0x16, 0x14, 0x17, 0x1b}, 14},
    {0xf0, {0x3c}, 1},
    {0xf0, {0x69}, 1},
    {0x29, {0}, 0},
    // {0x8d, {0x03}, 1},
    // {0x90, {0x08, 0x08, 0x08, 0x08}, 4},
    // {0xff, {0x60, 0x01, 0x04}, 3},
    // {0xC3, {0x13}, 1},
    // {0xC4, {0x13}, 1},
    // {0xC9, {0x30}, 1},
    // {0xbe, {0x11}, 1},
    // {0xe1, {0x10, 0x0e}, 2},
    // {0xdf, {0x21, 0x0c, 0x02}, 3},
    // // Set gamma
    // {0xF0, {0x45, 0x09, 0x08, 0x08, 0x26, 0x2a}, 6},
    // {0xF1, {0x43, 0x70, 0x72, 0x36, 0x37, 0x6f}, 6},
    // {0xF2, {0x45, 0x09, 0x08, 0x08, 0x26, 0x2a}, 6},
    // {0xF3, {0x43, 0x70, 0x72, 0x36, 0x37, 0x6f}, 6},
    // {0xed, {0x1b, 0x0b}, 2},
    // {0xae, {0x77}, 1},
    // {0xcd, {0x63}, 1},
    // {0x70, {0x07, 0x07, 0x04, 0x0e, 0x0f, 0x09, 0x07, 0x08, 0x03}, 9},
    // {0xE8, {0x34}, 1}, // 4 dot inversion
    // {0x60, {0x38, 0x0b, 0x6D, 0x6D, 0x39, 0xf0, 0x6D, 0x6D}, 8},
    // {0x61, {0x38, 0xf4, 0x6D, 0x6D, 0x38, 0xf7, 0x6D, 0x6D}, 8},
    // {0x62, {0x38, 0x0D, 0x71, 0xED, 0x70, 0x70, 0x38, 0x0F, 0x71, 0xEF, 0x70, 0x70}, 12},
    // {0x63, {0x38, 0x11, 0x71, 0xF1, 0x70, 0x70, 0x38, 0x13, 0x71, 0xF3, 0x70, 0x70}, 12},
    // {0x64, {0x28, 0x29, 0xF1, 0x01, 0xF1, 0x00, 0x07}, 7},
    // {0x66, {0x3C, 0x00, 0xCD, 0x67, 0x45, 0x45, 0x10, 0x00, 0x00, 0x00}, 10},
    // {0x67, {0x00, 0x3C, 0x00, 0x00, 0x00, 0x01, 0x54, 0x10, 0x32, 0x98}, 10},
    // {0x74, {0x10, 0x45, 0x80, 0x00, 0x00, 0x4E, 0x00}, 7},
    // {0x98, {0x3e, 0x07}, 2},
    // {0x99, {0x3e, 0x07}, 2},
    {0, {0}, 0xff},
};

static esp_err_t panel_st7796_init(esp_lcd_panel_t *panel)
{
    st7796_panel_t *st7796 = __containerof(panel, st7796_panel_t, base);
    esp_lcd_panel_io_handle_t io = st7796->io;

    // LCD goes into sleep mode and display will be turned off after power on reset, exit sleep mode first
    esp_lcd_panel_io_tx_param(io, LCD_CMD_SLPOUT, NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
    esp_lcd_panel_io_tx_param(io, LCD_CMD_MADCTL, (uint8_t[]) {
        st7796->madctl_val,
    }, 1);
    // esp_lcd_panel_io_tx_param(io, LCD_CMD_COLMOD, (uint8_t[]) {
    //     st7796->colmod_cal,
    // }, 1);

    // vendor specific initialization, it can be different between manufacturers
    // should consult the LCD supplier for initialization sequence code
    int cmd = 0;
    while (vendor_specific_init[cmd].data_bytes != 0xff) {
        esp_lcd_panel_io_tx_param(io, vendor_specific_init[cmd].cmd, vendor_specific_init[cmd].data, vendor_specific_init[cmd].data_bytes & 0x1F);
        cmd++;
    }

    return ESP_OK;
}

static esp_err_t panel_st7796_draw_bitmap(esp_lcd_panel_t *panel, int x_start, int y_start, int x_end, int y_end, const void *color_data)
{
    st7796_panel_t *st7796 = __containerof(panel, st7796_panel_t, base);
    assert((x_start < x_end) && (y_start < y_end) && "start position must be smaller than end position");
    esp_lcd_panel_io_handle_t io = st7796->io;

    x_start += st7796->x_gap;
    x_end += st7796->x_gap;
    y_start += st7796->y_gap;
    y_end += st7796->y_gap;

    // define an area of frame memory where MCU can access
    esp_lcd_panel_io_tx_param(io, LCD_CMD_CASET, (uint8_t[]) {
        (x_start >> 8) & 0xFF,
        x_start & 0xFF,
        ((x_end - 1) >> 8) & 0xFF,
        (x_end - 1) & 0xFF,
    }, 4);
    esp_lcd_panel_io_tx_param(io, LCD_CMD_RASET, (uint8_t[]) {
        (y_start >> 8) & 0xFF,
        y_start & 0xFF,
        ((y_end - 1) >> 8) & 0xFF,
        (y_end - 1) & 0xFF,
    }, 4);
    // transfer frame buffer
    size_t len = (x_end - x_start) * (y_end - y_start) * st7796->bits_per_pixel / 8;
    esp_lcd_panel_io_tx_color(io, LCD_CMD_RAMWR, color_data, len);

    return ESP_OK;
}

static esp_err_t panel_st7796_invert_color(esp_lcd_panel_t *panel, bool invert_color_data)
{
    st7796_panel_t *st7796 = __containerof(panel, st7796_panel_t, base);
    esp_lcd_panel_io_handle_t io = st7796->io;
    int command = 0;
    if (invert_color_data) {
        command = LCD_CMD_INVON;
    } else {
        command = LCD_CMD_INVOFF;
    }
    esp_lcd_panel_io_tx_param(io, command, NULL, 0);
    return ESP_OK;
}

static esp_err_t panel_st7796_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y)
{
    st7796_panel_t *st7796 = __containerof(panel, st7796_panel_t, base);
    esp_lcd_panel_io_handle_t io = st7796->io;
    if (mirror_x) {
        st7796->madctl_val |= LCD_CMD_MX_BIT;
    } else {
        st7796->madctl_val &= ~LCD_CMD_MX_BIT;
    }
    if (mirror_y) {
        st7796->madctl_val |= LCD_CMD_MY_BIT;
    } else {
        st7796->madctl_val &= ~LCD_CMD_MY_BIT;
    }
    esp_lcd_panel_io_tx_param(io, LCD_CMD_MADCTL, (uint8_t[]) {
        st7796->madctl_val
    }, 1);
    return ESP_OK;
}

static esp_err_t panel_st7796_swap_xy(esp_lcd_panel_t *panel, bool swap_axes)
{
    st7796_panel_t *st7796 = __containerof(panel, st7796_panel_t, base);
    esp_lcd_panel_io_handle_t io = st7796->io;
    if (swap_axes) {
        st7796->madctl_val |= LCD_CMD_MV_BIT;
    } else {
        st7796->madctl_val &= ~LCD_CMD_MV_BIT;
    }
    esp_lcd_panel_io_tx_param(io, LCD_CMD_MADCTL, (uint8_t[]) {
        st7796->madctl_val
    }, 1);
    return ESP_OK;
}

static esp_err_t panel_st7796_set_gap(esp_lcd_panel_t *panel, int x_gap, int y_gap)
{
    st7796_panel_t *st7796 = __containerof(panel, st7796_panel_t, base);
    st7796->x_gap = x_gap;
    st7796->y_gap = y_gap;
    return ESP_OK;
}

static esp_err_t panel_st7796_disp_on_off(esp_lcd_panel_t *panel, bool on_off)
{
    st7796_panel_t *st7796 = __containerof(panel, st7796_panel_t, base);
    esp_lcd_panel_io_handle_t io = st7796->io;
    int command = 0;
    if (on_off) {
        command = LCD_CMD_DISPON;
    } else {
        command = LCD_CMD_DISPOFF;
    }
    esp_lcd_panel_io_tx_param(io, command, NULL, 0);
    return ESP_OK;
}
