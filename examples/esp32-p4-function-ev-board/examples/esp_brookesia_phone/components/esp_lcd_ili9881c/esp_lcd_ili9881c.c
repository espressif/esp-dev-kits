/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "soc/soc_caps.h"

#if SOC_MIPI_DSI_SUPPORTED
#include "esp_check.h"
#include "esp_log.h"
#include "esp_lcd_panel_commands.h"
#include "esp_lcd_panel_interface.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_mipi_dsi.h"
#include "esp_lcd_panel_vendor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_lcd_ili9881c.h"

typedef struct {
    esp_lcd_panel_io_handle_t io;
    int reset_gpio_num;
    uint8_t madctl_val; // save current value of LCD_CMD_MADCTL register
    uint8_t colmod_val; // save surrent value of LCD_CMD_COLMOD register
    const ili9881c_lcd_init_cmd_t *init_cmds;
    uint16_t init_cmds_size;
    struct {
        unsigned int reset_level: 1;
    } flags;
    // To save the original functions of MIPI DPI panel
    esp_err_t (*del)(esp_lcd_panel_t *panel);
    esp_err_t (*init)(esp_lcd_panel_t *panel);
} ili9881c_panel_t;

static const char *TAG = "ili9881c";

static esp_err_t panel_ili9881c_del(esp_lcd_panel_t *panel);
static esp_err_t panel_ili9881c_init(esp_lcd_panel_t *panel);
static esp_err_t panel_ili9881c_reset(esp_lcd_panel_t *panel);
static esp_err_t panel_ili9881c_invert_color(esp_lcd_panel_t *panel, bool invert_color_data);
static esp_err_t panel_ili9881c_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y);
static esp_err_t panel_ili9881c_disp_on_off(esp_lcd_panel_t *panel, bool on_off);

esp_err_t esp_lcd_new_panel_ili9881c(const esp_lcd_panel_io_handle_t io, const esp_lcd_panel_dev_config_t *panel_dev_config,
                                     esp_lcd_panel_handle_t *ret_panel)
{
    ESP_RETURN_ON_FALSE(io && panel_dev_config && ret_panel, ESP_ERR_INVALID_ARG, TAG, "invalid arguments");
    ili9881c_vendor_config_t *vendor_config = (ili9881c_vendor_config_t *)panel_dev_config->vendor_config;
    ESP_RETURN_ON_FALSE(vendor_config && vendor_config->mipi_config.dpi_config && vendor_config->mipi_config.dsi_bus, ESP_ERR_INVALID_ARG, TAG,
                        "invalid vendor config");

    esp_err_t ret = ESP_OK;
    ili9881c_panel_t *ili9881c = (ili9881c_panel_t *)calloc(1, sizeof(ili9881c_panel_t));
    ESP_RETURN_ON_FALSE(ili9881c, ESP_ERR_NO_MEM, TAG, "no mem for ili9881c panel");

    if (panel_dev_config->reset_gpio_num >= 0) {
        gpio_config_t io_conf = {
            .mode = GPIO_MODE_OUTPUT,
            .pin_bit_mask = 1ULL << panel_dev_config->reset_gpio_num,
        };
        ESP_GOTO_ON_ERROR(gpio_config(&io_conf), err, TAG, "configure GPIO for RST line failed");
    }

    switch (panel_dev_config->color_space) {
    case LCD_RGB_ELEMENT_ORDER_RGB:
        ili9881c->madctl_val = 0;
        break;
    case LCD_RGB_ELEMENT_ORDER_BGR:
        ili9881c->madctl_val |= LCD_CMD_BGR_BIT;
        break;
    default:
        ESP_GOTO_ON_FALSE(false, ESP_ERR_NOT_SUPPORTED, err, TAG, "unsupported color space");
        break;
    }

    ili9881c->io = io;
    ili9881c->init_cmds = vendor_config->init_cmds;
    ili9881c->init_cmds_size = vendor_config->init_cmds_size;
    ili9881c->reset_gpio_num = panel_dev_config->reset_gpio_num;
    ili9881c->flags.reset_level = panel_dev_config->flags.reset_active_high;

    // Create MIPI DPI panel
    esp_lcd_panel_handle_t panel_handle = NULL;
    ESP_GOTO_ON_ERROR(esp_lcd_new_panel_dpi(vendor_config->mipi_config.dsi_bus, vendor_config->mipi_config.dpi_config, &panel_handle), err, TAG,
                      "create MIPI DPI panel failed");
    ESP_LOGD(TAG, "new MIPI DPI panel @%p", panel_handle);

    // Save the original functions of MIPI DPI panel
    ili9881c->del = panel_handle->del;
    ili9881c->init = panel_handle->init;
    // Overwrite the functions of MIPI DPI panel
    panel_handle->del = panel_ili9881c_del;
    panel_handle->init = panel_ili9881c_init;
    panel_handle->reset = panel_ili9881c_reset;
    panel_handle->mirror = panel_ili9881c_mirror;
    panel_handle->invert_color = panel_ili9881c_invert_color;
    panel_handle->disp_on_off = panel_ili9881c_disp_on_off;
    panel_handle->user_data = ili9881c;
    *ret_panel = panel_handle;
    ESP_LOGD(TAG, "new ili9881c panel @%p", ili9881c);

    return ESP_OK;

err:
    if (ili9881c) {
        if (panel_dev_config->reset_gpio_num >= 0) {
            gpio_reset_pin(panel_dev_config->reset_gpio_num);
        }
        free(ili9881c);
    }
    return ret;
}

/* Vendor init sequence for the MX101BA2140-48C (BOE TV101WXU-N90 10.1" 800x1280)
 * panel, converted 1:1 from the vendor reference code
 * "ILI9881C+BOE 10.1寸 8001280 B4 TV101WXU-N90 Code_20190820(1.txt".
 * Format: REGISTER,<cmd>,<count>,<params...>; "FF 98 81 xx" selects the
 * command page (3: GIP, 4: power, 1: panel/gamma, 0: standard DCS). */
static const ili9881c_lcd_init_cmd_t vendor_specific_init_default[] = {
    //  {cmd, { data }, data_size, delay_ms}
        // Page 3: GIP_1
        {0xFF, (uint8_t[]){0x98, 0x81, 0x03}, 3, 0},
        {0x01, (uint8_t[]){0x00}, 1, 0},
        {0x02, (uint8_t[]){0x00}, 1, 0},
        {0x03, (uint8_t[]){0x73}, 1, 0},
        {0x04, (uint8_t[]){0x00}, 1, 0},
        {0x05, (uint8_t[]){0x00}, 1, 0},
        {0x06, (uint8_t[]){0x08}, 1, 0},
        {0x07, (uint8_t[]){0x00}, 1, 0},
        {0x08, (uint8_t[]){0x00}, 1, 0},
        {0x09, (uint8_t[]){0x00}, 1, 0},
        {0x0A, (uint8_t[]){0x01}, 1, 0},
        {0x0B, (uint8_t[]){0x01}, 1, 0},
        {0x0C, (uint8_t[]){0x00}, 1, 0},
        {0x0D, (uint8_t[]){0x01}, 1, 0},
        {0x0E, (uint8_t[]){0x01}, 1, 0},
        {0x0F, (uint8_t[]){0x00}, 1, 0},
        {0x10, (uint8_t[]){0x00}, 1, 0},
        {0x11, (uint8_t[]){0x00}, 1, 0},
        {0x12, (uint8_t[]){0x00}, 1, 0},
        {0x13, (uint8_t[]){0x00}, 1, 0},
        {0x14, (uint8_t[]){0x00}, 1, 0},
        {0x15, (uint8_t[]){0x00}, 1, 0},
        {0x16, (uint8_t[]){0x00}, 1, 0},
        {0x17, (uint8_t[]){0x00}, 1, 0},
        {0x18, (uint8_t[]){0x00}, 1, 0},
        {0x19, (uint8_t[]){0x00}, 1, 0},
        {0x1A, (uint8_t[]){0x00}, 1, 0},
        {0x1B, (uint8_t[]){0x00}, 1, 0},
        {0x1C, (uint8_t[]){0x00}, 1, 0},
        {0x1D, (uint8_t[]){0x00}, 1, 0},
        {0x1E, (uint8_t[]){0x40}, 1, 0},
        {0x1F, (uint8_t[]){0xC0}, 1, 0},
        {0x20, (uint8_t[]){0x06}, 1, 0},
        {0x21, (uint8_t[]){0x01}, 1, 0},
        {0x22, (uint8_t[]){0x06}, 1, 0},
        {0x23, (uint8_t[]){0x01}, 1, 0},
        {0x24, (uint8_t[]){0x88}, 1, 0},
        {0x25, (uint8_t[]){0x88}, 1, 0},
        {0x26, (uint8_t[]){0x00}, 1, 0},
        {0x27, (uint8_t[]){0x00}, 1, 0},
        {0x28, (uint8_t[]){0x3B}, 1, 0},
        {0x29, (uint8_t[]){0x03}, 1, 0},
        {0x2A, (uint8_t[]){0x00}, 1, 0},
        {0x2B, (uint8_t[]){0x00}, 1, 0},
        {0x2C, (uint8_t[]){0x00}, 1, 0},
        {0x2D, (uint8_t[]){0x00}, 1, 0},
        {0x2E, (uint8_t[]){0x00}, 1, 0},
        {0x2F, (uint8_t[]){0x00}, 1, 0},
        {0x30, (uint8_t[]){0x00}, 1, 0},
        {0x31, (uint8_t[]){0x00}, 1, 0},
        {0x32, (uint8_t[]){0x00}, 1, 0},
        {0x33, (uint8_t[]){0x00}, 1, 0},
        {0x34, (uint8_t[]){0x00}, 1, 0},  // GPWR1/2 non overlap time 2.62us
        {0x35, (uint8_t[]){0x00}, 1, 0},
        {0x36, (uint8_t[]){0x00}, 1, 0},
        {0x37, (uint8_t[]){0x00}, 1, 0},
        {0x38, (uint8_t[]){0x00}, 1, 0},
        {0x39, (uint8_t[]){0x00}, 1, 0},
        {0x3A, (uint8_t[]){0x00}, 1, 0},
        {0x3B, (uint8_t[]){0x00}, 1, 0},
        {0x3C, (uint8_t[]){0x00}, 1, 0},
        {0x3D, (uint8_t[]){0x00}, 1, 0},
        {0x3E, (uint8_t[]){0x00}, 1, 0},
        {0x3F, (uint8_t[]){0x00}, 1, 0},
        {0x40, (uint8_t[]){0x00}, 1, 0},
        {0x41, (uint8_t[]){0x00}, 1, 0},
        {0x42, (uint8_t[]){0x00}, 1, 0},
        {0x43, (uint8_t[]){0x00}, 1, 0},
        {0x44, (uint8_t[]){0x00}, 1, 0},

        // Page 3: GIP_2
        {0x50, (uint8_t[]){0x01}, 1, 0},
        {0x51, (uint8_t[]){0x23}, 1, 0},
        {0x52, (uint8_t[]){0x45}, 1, 0},
        {0x53, (uint8_t[]){0x67}, 1, 0},
        {0x54, (uint8_t[]){0x89}, 1, 0},
        {0x55, (uint8_t[]){0xAB}, 1, 0},
        {0x56, (uint8_t[]){0x01}, 1, 0},
        {0x57, (uint8_t[]){0x23}, 1, 0},
        {0x58, (uint8_t[]){0x45}, 1, 0},
        {0x59, (uint8_t[]){0x67}, 1, 0},
        {0x5A, (uint8_t[]){0x89}, 1, 0},
        {0x5B, (uint8_t[]){0xAB}, 1, 0},
        {0x5C, (uint8_t[]){0xCD}, 1, 0},
        {0x5D, (uint8_t[]){0xEF}, 1, 0},

        // Page 3: GIP_3
        {0x5E, (uint8_t[]){0x00}, 1, 0},
        {0x5F, (uint8_t[]){0x01}, 1, 0},
        {0x60, (uint8_t[]){0x01}, 1, 0},
        {0x61, (uint8_t[]){0x06}, 1, 0},
        {0x62, (uint8_t[]){0x06}, 1, 0},
        {0x63, (uint8_t[]){0x07}, 1, 0},
        {0x64, (uint8_t[]){0x07}, 1, 0},
        {0x65, (uint8_t[]){0x00}, 1, 0},
        {0x66, (uint8_t[]){0x00}, 1, 0},
        {0x67, (uint8_t[]){0x02}, 1, 0},
        {0x68, (uint8_t[]){0x02}, 1, 0},
        {0x69, (uint8_t[]){0x05}, 1, 0},
        {0x6A, (uint8_t[]){0x05}, 1, 0},
        {0x6B, (uint8_t[]){0x02}, 1, 0},
        {0x6C, (uint8_t[]){0x0D}, 1, 0},
        {0x6D, (uint8_t[]){0x0D}, 1, 0},
        {0x6E, (uint8_t[]){0x0C}, 1, 0},
        {0x6F, (uint8_t[]){0x0C}, 1, 0},
        {0x70, (uint8_t[]){0x0F}, 1, 0},
        {0x71, (uint8_t[]){0x0F}, 1, 0},
        {0x72, (uint8_t[]){0x0E}, 1, 0},
        {0x73, (uint8_t[]){0x0E}, 1, 0},
        {0x74, (uint8_t[]){0x02}, 1, 0},
        {0x75, (uint8_t[]){0x01}, 1, 0},
        {0x76, (uint8_t[]){0x01}, 1, 0},
        {0x77, (uint8_t[]){0x06}, 1, 0},
        {0x78, (uint8_t[]){0x06}, 1, 0},
        {0x79, (uint8_t[]){0x07}, 1, 0},
        {0x7A, (uint8_t[]){0x07}, 1, 0},
        {0x7B, (uint8_t[]){0x00}, 1, 0},
        {0x7C, (uint8_t[]){0x00}, 1, 0},
        {0x7D, (uint8_t[]){0x02}, 1, 0},
        {0x7E, (uint8_t[]){0x02}, 1, 0},
        {0x7F, (uint8_t[]){0x05}, 1, 0},
        {0x80, (uint8_t[]){0x05}, 1, 0},
        {0x81, (uint8_t[]){0x02}, 1, 0},
        {0x82, (uint8_t[]){0x0D}, 1, 0},
        {0x83, (uint8_t[]){0x0D}, 1, 0},
        {0x84, (uint8_t[]){0x0C}, 1, 0},
        {0x85, (uint8_t[]){0x0C}, 1, 0},
        {0x86, (uint8_t[]){0x0F}, 1, 0},
        {0x87, (uint8_t[]){0x0F}, 1, 0},
        {0x88, (uint8_t[]){0x0E}, 1, 0},
        {0x89, (uint8_t[]){0x0E}, 1, 0},
        {0x8A, (uint8_t[]){0x02}, 1, 0},

        // Page 4: power
        {0xFF, (uint8_t[]){0x98, 0x81, 0x04}, 3, 0},
        {0x3B, (uint8_t[]){0xC0}, 1, 0},  // ILI4003D sel
        {0x6C, (uint8_t[]){0x15}, 1, 0},  // Set VCORE voltage = 1.5V
        {0x6E, (uint8_t[]){0x2A}, 1, 0},  // di_pwr_reg=0 for power mode 2A // VGH clamp 18V
        {0x6F, (uint8_t[]){0x33}, 1, 0},  // 45 // pumping ratio VGH=5x VGL=-3x
        {0x8D, (uint8_t[]){0x1B}, 1, 0},  // VGL clamp -10V
        {0x87, (uint8_t[]){0xBA}, 1, 0},  // ESD
        {0x3A, (uint8_t[]){0x24}, 1, 0},  // POWER SAVING
        {0x26, (uint8_t[]){0x76}, 1, 0},
        {0xB2, (uint8_t[]){0xD1}, 1, 0},

        // Page 1: panel control + gamma
        {0xFF, (uint8_t[]){0x98, 0x81, 0x01}, 3, 0},
        // PAD_CONTROL (Espressif stock driver writes this from lane_num;
        // our fork never did): D1 LANSEL_SW_EN=1 takes lane selection away
        // from the module's hardware bonding, D0 LANSEL_SW=1 selects 2
        // lanes. Without it the TCON lane config stays at power-on default
        // (pin bond) — if this module is bonded for 4 lanes it waits for a
        // 4-lane HS interleave that a 2-lane host can never produce.
        // Stock driver: 0x03 = 2-lane, 0x02 = 3/4-lane, reset default 0x00.
        {0xB7, (uint8_t[]){0x03}, 1, 0},
        {0x22, (uint8_t[]){0x0A}, 1, 0},  // BGR, SS
        {0x31, (uint8_t[]){0x00}, 1, 0},  // Zigzag type3 inversion
        {0x40, (uint8_t[]){0x53}, 1, 0},  // ILI4003D sel
        {0x43, (uint8_t[]){0x66}, 1, 0},
        {0x53, (uint8_t[]){0x4C}, 1, 0},
        {0x50, (uint8_t[]){0x87}, 1, 0},
        {0x51, (uint8_t[]){0x82}, 1, 0},
        {0x60, (uint8_t[]){0x15}, 1, 0},
        {0x61, (uint8_t[]){0x01}, 1, 0},
        {0x62, (uint8_t[]){0x0C}, 1, 0},
        {0x63, (uint8_t[]){0x00}, 1, 0},

        // Gamma P
        {0xA0, (uint8_t[]){0x00}, 1, 0},
        {0xA1, (uint8_t[]){0x13}, 1, 0},  // VP251
        {0xA2, (uint8_t[]){0x23}, 1, 0},  // VP247
        {0xA3, (uint8_t[]){0x14}, 1, 0},  // VP243
        {0xA4, (uint8_t[]){0x16}, 1, 0},  // VP239
        {0xA5, (uint8_t[]){0x29}, 1, 0},  // VP231
        {0xA6, (uint8_t[]){0x1E}, 1, 0},  // VP219
        {0xA7, (uint8_t[]){0x1D}, 1, 0},  // VP203
        {0xA8, (uint8_t[]){0x86}, 1, 0},  // VP175
        {0xA9, (uint8_t[]){0x1E}, 1, 0},  // VP144
        {0xAA, (uint8_t[]){0x29}, 1, 0},  // VP111
        {0xAB, (uint8_t[]){0x74}, 1, 0},  // VP80
        {0xAC, (uint8_t[]){0x19}, 1, 0},  // VP52
        {0xAD, (uint8_t[]){0x17}, 1, 0},  // VP36
        {0xAE, (uint8_t[]){0x4B}, 1, 0},  // VP24
        {0xAF, (uint8_t[]){0x20}, 1, 0},  // VP16
        {0xB0, (uint8_t[]){0x26}, 1, 0},  // VP12
        {0xB1, (uint8_t[]){0x4C}, 1, 0},  // VP8
        {0xB2, (uint8_t[]){0x5D}, 1, 0},  // VP4
        {0xB3, (uint8_t[]){0x3F}, 1, 0},  // VP0

        // Gamma N
        {0xC0, (uint8_t[]){0x00}, 1, 0},  // VN255 GAMMA N
        {0xC1, (uint8_t[]){0x13}, 1, 0},  // VN251
        {0xC2, (uint8_t[]){0x23}, 1, 0},  // VN247
        {0xC3, (uint8_t[]){0x14}, 1, 0},  // VN243
        {0xC4, (uint8_t[]){0x16}, 1, 0},  // VN239
        {0xC5, (uint8_t[]){0x29}, 1, 0},  // VN231
        {0xC6, (uint8_t[]){0x1E}, 1, 0},  // VN219
        {0xC7, (uint8_t[]){0x1D}, 1, 0},  // VN203
        {0xC8, (uint8_t[]){0x86}, 1, 0},  // VN175
        {0xC9, (uint8_t[]){0x1E}, 1, 0},  // VN144
        {0xCA, (uint8_t[]){0x29}, 1, 0},  // VN111
        {0xCB, (uint8_t[]){0x74}, 1, 0},  // VN80
        {0xCC, (uint8_t[]){0x19}, 1, 0},  // VN52
        {0xCD, (uint8_t[]){0x17}, 1, 0},  // VN36
        {0xCE, (uint8_t[]){0x4B}, 1, 0},  // VN24
        {0xCF, (uint8_t[]){0x20}, 1, 0},  // VN16
        {0xD0, (uint8_t[]){0x26}, 1, 0},  // VN12
        {0xD1, (uint8_t[]){0x4C}, 1, 0},  // VN8
        {0xD2, (uint8_t[]){0x5D}, 1, 0},  // VN4
        {0xD3, (uint8_t[]){0x3F}, 1, 0},  // VN0

        // Page 0: standard DCS
        {0xFF, (uint8_t[]){0x98, 0x81, 0x00}, 3, 0},

        // Tearing effect line on
        {0x35, (uint8_t[]){0x00}, 1, 0},

        // COLMOD: RGB565 (16-bit/pixel) — must match DPI pixel format,
        // panel OTP default may be RGB888 which garbles the pixel stream
        {0x3A, (uint8_t[]){0x55}, 1, 0},

        // Lane-count history: the Guanhaiwei app-note Page8 R20H "auto
        // detect" was a wrong target (reads 0x00 => register does not exist
        // in this BOE module; also 0xEE vendor page-select is ignored). The
        // authoritative switch is Page1 0xB7 PAD_CONTROL — written from the
        // Page1 section above — which the stock Espressif driver derives
        // from lane_num. OTP does NOT hold lane config (ID/VCOM/VREG/gamma
        // only), so a 4-lane-bonded module is software-convertible to 2
        // lanes; no OTP "4-lane lock" exists to block a 2-lane host.

        // Sleep out and display on
        {0x11, NULL, 0, 120},  // Sleep out - wait 120ms
        {0x29, NULL, 0, 20},   // Display on - wait 20ms

        // BIST self-test block REMOVED (was: Page4 R2F=0x01, FRM_EN=1,
        // verified to produce a test pattern on this module). Reason: BIST
        // bypasses/replaces the external video input, so with it enabled a
        // WORKING video link could never show the UI — a fixed link would be
        // misread as still broken. Video-arrival verdict now comes from the
        // readback task log (Page1 OTP/ID section) instead of the screen.
        // Sequence ends in Page 0 (0x35/0x3A above), so no page restore
        // needed before the post-init DISPON/MADCTL DCS writes.
    };

static esp_err_t panel_ili9881c_del(esp_lcd_panel_t *panel)
{
    ili9881c_panel_t *ili9881c = (ili9881c_panel_t *)panel->user_data;

    if (ili9881c->reset_gpio_num >= 0) {
        gpio_reset_pin(ili9881c->reset_gpio_num);
    }
    // Delete MIPI DPI panel
    ili9881c->del(panel);
    ESP_LOGD(TAG, "del ili9881c panel @%p", ili9881c);
    free(ili9881c);

    return ESP_OK;
}

static esp_err_t panel_ili9881c_init(esp_lcd_panel_t *panel)
{
    ili9881c_panel_t *ili9881c = (ili9881c_panel_t *)panel->user_data;
    esp_lcd_panel_io_handle_t io = ili9881c->io;
    const ili9881c_lcd_init_cmd_t *init_cmds = NULL;
    uint16_t init_cmds_size = 0;
    int cur_page = 0; // tracks the panel register page while iterating init_cmds

    // Skip ID read - DBI reads are not reliable over MIPI-DSI on this panel
    ESP_LOGI(TAG, "Skipping panel ID read (DBI read not reliable over MIPI-DSI)");

    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LCD_CMD_MADCTL, (uint8_t[]) {
        ili9881c->madctl_val,
    }, 1), TAG, "send command failed");

    // vendor specific initialization, it can be different between manufacturers
    // should consult the LCD supplier for initialization sequence code
    if (ili9881c->init_cmds) {
        init_cmds = ili9881c->init_cmds;
        init_cmds_size = ili9881c->init_cmds_size;
    } else {
        init_cmds = vendor_specific_init_default;
        init_cmds_size = sizeof(vendor_specific_init_default) / sizeof(ili9881c_lcd_init_cmd_t);
    }

    ESP_LOGI(TAG, "Sending %d initialization commands...", init_cmds_size);
    for (int i = 0; i < init_cmds_size; i++) {
        // Feed watchdog every 10 commands to prevent timeout
        if (i > 0 && i % 10 == 0) {
            vTaskDelay(1);
        }

        // Track page switches: 0xFF 98 81 xx selects Page0-4 (standard form),
        // 0xEE xx selects vendor pages (Page5+, e.g. Page8). Needed so the
        // MADCTL conflict check below only fires for the real MADCTL.
        if (init_cmds[i].cmd == 0xFF && init_cmds[i].data_bytes == 3) {
            cur_page = ((uint8_t *)init_cmds[i].data)[2];
        } else if (init_cmds[i].cmd == 0xEE && init_cmds[i].data_bytes == 1) {
            cur_page = ((uint8_t *)init_cmds[i].data)[0];
        }

        // Check if the command has been used or conflicts with the internal.
        // Only a Page0 0x36 is the standard MADCTL; the vendor GIP table also
        // contains a 0x36 (Page3 GIP register) which is NOT MADCTL — treating
        // it as such clobbered madctl_val and spammed a bogus warning.
        if (init_cmds[i].data_bytes > 0 && cur_page == 0 && init_cmds[i].cmd == LCD_CMD_MADCTL) {
            ili9881c->madctl_val = ((uint8_t *)init_cmds[i].data)[0];
            ESP_LOGW(TAG, "The %02Xh command has been used and will be overwritten by external initialization sequence",
                     init_cmds[i].cmd);
        }

        // Send command
        ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, init_cmds[i].cmd, init_cmds[i].data, init_cmds[i].data_bytes), TAG, "send command failed");
        vTaskDelay(pdMS_TO_TICKS(init_cmds[i].delay_ms));
    }
    ESP_LOGI(TAG, "Init commands sent successfully");

    ESP_RETURN_ON_ERROR(ili9881c->init(panel), TAG, "init MIPI DPI panel failed");

    return ESP_OK;
}

static esp_err_t panel_ili9881c_reset(esp_lcd_panel_t *panel)
{
    ili9881c_panel_t *ili9881c = (ili9881c_panel_t *)panel->user_data;
    esp_lcd_panel_io_handle_t io = ili9881c->io;

    // Perform hardware reset
    if (ili9881c->reset_gpio_num >= 0) {
        gpio_set_level(ili9881c->reset_gpio_num, !ili9881c->flags.reset_level);
        vTaskDelay(pdMS_TO_TICKS(5));
        gpio_set_level(ili9881c->reset_gpio_num, ili9881c->flags.reset_level);
        vTaskDelay(pdMS_TO_TICKS(10));
        gpio_set_level(ili9881c->reset_gpio_num, !ili9881c->flags.reset_level);
        vTaskDelay(pdMS_TO_TICKS(120));
    } else if (io) { // Perform software reset
        ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LCD_CMD_SWRESET, NULL, 0), TAG, "send command failed");
        vTaskDelay(pdMS_TO_TICKS(120));
    }

    return ESP_OK;
}

static esp_err_t panel_ili9881c_invert_color(esp_lcd_panel_t *panel, bool invert_color_data)
{
    ili9881c_panel_t *ili9881c = (ili9881c_panel_t *)panel->user_data;
    esp_lcd_panel_io_handle_t io = ili9881c->io;
    uint8_t command = 0;

    ESP_RETURN_ON_FALSE(io, ESP_ERR_INVALID_STATE, TAG, "invalid panel IO");

    if (invert_color_data) {
        command = LCD_CMD_INVON;
    } else {
        command = LCD_CMD_INVOFF;
    }
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, command, NULL, 0), TAG, "send command failed");

    return ESP_OK;
}

static esp_err_t panel_ili9881c_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y)
{
    ili9881c_panel_t *ili9881c = (ili9881c_panel_t *)panel->user_data;
    esp_lcd_panel_io_handle_t io = ili9881c->io;
    uint8_t madctl_val = ili9881c->madctl_val;

    ESP_RETURN_ON_FALSE(io, ESP_ERR_INVALID_STATE, TAG, "invalid panel IO");

    // Control mirror through standard DCS MADCTL bits (MX=0x40, MY=0x80)
    if (mirror_x) {
        madctl_val |= LCD_CMD_MX_BIT;
    } else {
        madctl_val &= ~LCD_CMD_MX_BIT;
    }
    if (mirror_y) {
        madctl_val |= LCD_CMD_MY_BIT;
    } else {
        madctl_val &= ~LCD_CMD_MY_BIT;
    }

    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LCD_CMD_MADCTL, (uint8_t []) {
        madctl_val
    }, 1), TAG, "send command failed");
    ili9881c->madctl_val = madctl_val;

    return ESP_OK;
}

static esp_err_t panel_ili9881c_disp_on_off(esp_lcd_panel_t *panel, bool on_off)
{
    ili9881c_panel_t *ili9881c = (ili9881c_panel_t *)panel->user_data;
    esp_lcd_panel_io_handle_t io = ili9881c->io;
    int command = 0;

    if (on_off) {
        command = LCD_CMD_DISPON;
    } else {
        command = LCD_CMD_DISPOFF;
    }
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, command, NULL, 0), TAG, "send command failed");
    return ESP_OK;
}
#endif
