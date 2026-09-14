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
#include "esp_lcd_jd9366.h"

#define JD9366_CMD_GS_BIT       (1 << 0)
#define JD9366_CMD_SS_BIT       (1 << 1)

typedef struct {
    esp_lcd_panel_io_handle_t io;
    int reset_gpio_num;
    uint8_t madctl_val; // save current value of LCD_CMD_MADCTL register
    uint8_t colmod_val; // save surrent value of LCD_CMD_COLMOD register
    const jd9366_lcd_init_cmd_t *init_cmds;
    uint16_t init_cmds_size;
    struct {
        unsigned int reset_level: 1;
    } flags;
    // To save the original functions of MIPI DPI panel
    esp_err_t (*del)(esp_lcd_panel_t *panel);
    esp_err_t (*init)(esp_lcd_panel_t *panel);
} jd9366_panel_t;

static const char *TAG = "jd9366";

static esp_err_t panel_jd9366_del(esp_lcd_panel_t *panel);
static esp_err_t panel_jd9366_init(esp_lcd_panel_t *panel);
static esp_err_t panel_jd9366_reset(esp_lcd_panel_t *panel);
static esp_err_t panel_jd9366_invert_color(esp_lcd_panel_t *panel, bool invert_color_data);
static esp_err_t panel_jd9366_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y);
static esp_err_t panel_jd9366_disp_on_off(esp_lcd_panel_t *panel, bool on_off);

esp_err_t esp_lcd_new_panel_jd9366(const esp_lcd_panel_io_handle_t io, const esp_lcd_panel_dev_config_t *panel_dev_config,
                                   esp_lcd_panel_handle_t *ret_panel)
{
    ESP_RETURN_ON_FALSE(io && panel_dev_config && ret_panel, ESP_ERR_INVALID_ARG, TAG, "invalid arguments");
    jd9366_vendor_config_t *vendor_config = (jd9366_vendor_config_t *)panel_dev_config->vendor_config;
    ESP_RETURN_ON_FALSE(vendor_config && vendor_config->mipi_config.dpi_config && vendor_config->mipi_config.dsi_bus, ESP_ERR_INVALID_ARG, TAG,
                        "invalid vendor config");

    esp_err_t ret = ESP_OK;
    jd9366_panel_t *jd9366 = (jd9366_panel_t *)calloc(1, sizeof(jd9366_panel_t));
    ESP_RETURN_ON_FALSE(jd9366, ESP_ERR_NO_MEM, TAG, "no mem for jd9366 panel");

    if (panel_dev_config->reset_gpio_num >= 0) {
        gpio_config_t io_conf = {
            .mode = GPIO_MODE_OUTPUT,
            .pin_bit_mask = 1ULL << panel_dev_config->reset_gpio_num,
        };
        ESP_GOTO_ON_ERROR(gpio_config(&io_conf), err, TAG, "configure GPIO for RST line failed");
    }

    switch (panel_dev_config->color_space) {
    case LCD_RGB_ELEMENT_ORDER_RGB:
        jd9366->madctl_val = 0;
        break;
    case LCD_RGB_ELEMENT_ORDER_BGR:
        jd9366->madctl_val |= LCD_CMD_BGR_BIT;
        break;
    default:
        ESP_GOTO_ON_FALSE(false, ESP_ERR_NOT_SUPPORTED, err, TAG, "unsupported color space");
        break;
    }

    jd9366->io = io;
    jd9366->init_cmds = vendor_config->init_cmds;
    jd9366->init_cmds_size = vendor_config->init_cmds_size;
    jd9366->reset_gpio_num = panel_dev_config->reset_gpio_num;
    jd9366->flags.reset_level = panel_dev_config->flags.reset_active_high;

    // Create MIPI DPI panel
    esp_lcd_panel_handle_t panel_handle = NULL;
    ESP_GOTO_ON_ERROR(esp_lcd_new_panel_dpi(vendor_config->mipi_config.dsi_bus, vendor_config->mipi_config.dpi_config, &panel_handle), err, TAG,
                      "create MIPI DPI panel failed");
    ESP_LOGD(TAG, "new MIPI DPI panel @%p", panel_handle);

    // Save the original functions of MIPI DPI panel
    jd9366->del = panel_handle->del;
    jd9366->init = panel_handle->init;
    // Overwrite the functions of MIPI DPI panel
    panel_handle->del = panel_jd9366_del;
    panel_handle->init = panel_jd9366_init;
    panel_handle->reset = panel_jd9366_reset;
    panel_handle->mirror = panel_jd9366_mirror;
    panel_handle->invert_color = panel_jd9366_invert_color;
    panel_handle->disp_on_off = panel_jd9366_disp_on_off;
    panel_handle->user_data = jd9366;
    *ret_panel = panel_handle;
    ESP_LOGD(TAG, "new jd9366 panel @%p", jd9366);

    return ESP_OK;

err:
    if (jd9366) {
        if (panel_dev_config->reset_gpio_num >= 0) {
            gpio_reset_pin(panel_dev_config->reset_gpio_num);
        }
        free(jd9366);
    }
    return ret;
}

static const jd9366_lcd_init_cmd_t vendor_specific_init_default[] = {
    //  {cmd, { data }, data_size, delay_ms}
        // Page 1 commands
        {0x30, (uint8_t []){0x01}, 1, 0},
        {0x78, (uint8_t []){0x49, 0x61, 0x02, 0x00}, 4, 0},
        
        // Page 2 commands  
        {0x30, (uint8_t []){0x02}, 1, 0},
        {0x31, (uint8_t []){0x12}, 1, 0},
        {0x32, (uint8_t []){0x08}, 1, 0},
        {0x33, (uint8_t []){0x3f}, 1, 0},
        {0x3c, (uint8_t []){0x04}, 1, 0},
        {0x3d, (uint8_t []){0x78}, 1, 0},
        {0x3e, (uint8_t []){0x43}, 1, 0},
        {0x3f, (uint8_t []){0x30}, 1, 0},
        {0x42, (uint8_t []){0xa2}, 1, 0},
        {0x43, (uint8_t []){0xf0}, 1, 0},
        {0x44, (uint8_t []){0x01}, 1, 0},
        {0x46, (uint8_t []){0x17}, 1, 0},
        {0x49, (uint8_t []){0xc0}, 1, 0},
        {0x6d, (uint8_t []){0x30}, 1, 0},
        {0x6e, (uint8_t []){0x21}, 1, 0},
        {0x41, (uint8_t []){0x5b, 0x5b, 0x03, 0x03, 0x5b, 0x5b, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03}, 12, 0},
        {0x5a, (uint8_t []){0x00, 0x00, 0x34, 0x34, 0x31, 0x31, 0x23, 0x23, 0x24, 0x24, 0x23}, 11, 0},
        {0x5b, (uint8_t []){0x23, 0x0b, 0x0b, 0x09, 0x09, 0x0f, 0x0f, 0x0d, 0x0d, 0x06, 0x06}, 11, 0},
        {0x5c, (uint8_t []){0x00, 0x00, 0x34, 0x34, 0x31, 0x31, 0x23, 0x23, 0x24, 0x24, 0x23}, 11, 0},
        {0x5d, (uint8_t []){0x23, 0x0a, 0x0a, 0x08, 0x08, 0x0e, 0x0e, 0x0c, 0x0c, 0x05, 0x05}, 11, 0},
        {0x5e, (uint8_t []){0x00, 0x00, 0x31, 0x31, 0x34, 0x34, 0x23, 0x23, 0x24, 0x24, 0x23}, 11, 0},
        {0x5f, (uint8_t []){0x23, 0x0c, 0x0c, 0x0e, 0x0e, 0x08, 0x08, 0x0a, 0x0a, 0x05, 0x05}, 11, 0},
        {0x60, (uint8_t []){0x00, 0x00, 0x31, 0x31, 0x34, 0x34, 0x23, 0x23, 0x24, 0x24, 0x23}, 11, 0},
        {0x61, (uint8_t []){0x23, 0x0d, 0x0d, 0x0f, 0x0f, 0x09, 0x09, 0x0b, 0x0b, 0x06, 0x06}, 11, 0},
        {0x64, (uint8_t []){0xff, 0xff, 0x3f}, 3, 0},
        {0x65, (uint8_t []){0xff, 0xff, 0x3f}, 3, 0},
        {0x6f, (uint8_t []){0x03, 0x00, 0x00}, 3, 0},
        {0x70, (uint8_t []){0x03, 0x00, 0x00}, 3, 0},
        {0x71, (uint8_t []){0x00, 0x00, 0x80}, 3, 0},
        {0x72, (uint8_t []){0x00, 0x00, 0x00}, 3, 0},
        {0x4c, (uint8_t []){0x22, 0x22}, 2, 0},
        {0x73, (uint8_t []){0x2a}, 1, 0},
        {0x4e, (uint8_t []){0x00, 0x00}, 2, 0},
        {0x50, (uint8_t []){0x00, 0x00}, 2, 0},
        {0x55, (uint8_t []){0xff, 0xff}, 2, 0},
        {0x56, (uint8_t []){0xff, 0xff}, 2, 0},
        {0x57, (uint8_t []){0x00, 0x00}, 2, 0},
        {0x58, (uint8_t []){0xff, 0xff}, 2, 0},
        {0x66, (uint8_t []){0xff, 0xff}, 2, 0},
        {0x67, (uint8_t []){0xff, 0xff}, 2, 0},
        {0x4a, (uint8_t []){0x3f}, 1, 0},
        
        // Page 8 commands
        {0x30, (uint8_t []){0x08}, 1, 0},
        {0x31, (uint8_t []){0x65}, 1, 0},
        {0x33, (uint8_t []){0x05}, 1, 0},
        {0x40, (uint8_t []){0x50}, 1, 0},
        {0x41, (uint8_t []){0x80}, 1, 0},
        {0x42, (uint8_t []){0x1a}, 1, 0},
        {0x47, (uint8_t []){0x0a}, 1, 0},
        {0x48, (uint8_t []){0x0d}, 1, 0},
        {0x50, (uint8_t []){0x17}, 1, 0},
        {0x5a, (uint8_t []){0x20}, 1, 0},
        {0x5b, (uint8_t []){0x00}, 1, 0},
        {0x5c, (uint8_t []){0x53}, 1, 0},
        // MIPI 2 lane configuration
        {0x5D, (uint8_t []){0x0D}, 1, 0},
        {0x5F, (uint8_t []){0x01}, 1, 0},
        {0x62, (uint8_t []){0x04}, 1, 0},
        {0x65, (uint8_t []){0x5f}, 1, 0},
        {0x73, (uint8_t []){0x01}, 1, 0},
        
        // Page A commands
        {0x30, (uint8_t []){0x0a}, 1, 0},
        {0x32, (uint8_t []){0xff}, 1, 0},
        {0x33, (uint8_t []){0x28}, 1, 0},
        {0x3f, (uint8_t []){0x53}, 1, 0},
        {0x40, (uint8_t []){0x15}, 1, 0},
        {0x47, (uint8_t []){0x20}, 1, 0},
        {0x48, (uint8_t []){0x80}, 1, 0},
        {0x49, (uint8_t []){0x03}, 1, 0},
        
        // Page B gamma commands
        {0x30, (uint8_t []){0x0b}, 1, 0},
        {0x33, (uint8_t []){0x00, 0x3d}, 2, 0},
        {0x3c, (uint8_t []){0x00, 0xbf}, 2, 0},
        {0x43, (uint8_t []){0xb1}, 1, 0},
        {0x44, (uint8_t []){0x31}, 1, 0},
        {0x3e, (uint8_t []){0x00, 0x10, 0x1e, 0x27, 0x2f}, 5, 0},
        {0x3f, (uint8_t []){0x54, 0x6f, 0x73, 0x7c, 0x77, 0x91, 0x90, 0x99, 0xa8, 0xa5, 0xae, 0xb5, 0xc5, 0xb8}, 14, 0},
        {0x40, (uint8_t []){0x52, 0x52, 0x53, 0x7f, 0x00, 0x10, 0x1e, 0x27, 0x2f}, 9, 0},
        {0x41, (uint8_t []){0x54, 0x6f, 0x73, 0x7c, 0x77, 0x91, 0x98, 0x99, 0xa8, 0xa5, 0xae, 0xb5, 0xc5, 0xb8}, 14, 0},
        {0x42, (uint8_t []){0x52, 0x52, 0x53, 0x7f}, 4, 0},
        {0x45, (uint8_t []){0x70}, 1, 0},
        {0x46, (uint8_t []){0x3b}, 1, 0},
        {0x48, (uint8_t []){0x7c}, 1, 0},
        {0x49, (uint8_t []){0x1e}, 1, 0},
        {0x4a, (uint8_t []){0x3a}, 1, 0},
        
        // Page C commands
        {0x30, (uint8_t []){0x0c}, 1, 0},
        {0x32, (uint8_t []){0x62}, 1, 0},
        {0x71, (uint8_t []){0x77}, 1, 0},
        
        // Page D commands
        {0x30, (uint8_t []){0x0d}, 1, 0},
        {0x4c, (uint8_t []){0x74}, 1, 0},
        
        // Back to page 0
        {0x30, (uint8_t []){0x00}, 1, 0},
        
        // Tearing effect line on
        {0x35, (uint8_t []){0x00}, 1, 0},
        
        // Sleep out and display on
        {0x11, (uint8_t []){0x00}, 1, 120},  // Sleep out - wait 120ms
        {0x29, (uint8_t []){0x00}, 1, 20},   // Display on - wait 20ms
    };
static esp_err_t panel_jd9366_del(esp_lcd_panel_t *panel)
{
    jd9366_panel_t *jd9366 = (jd9366_panel_t *)panel->user_data;

    if (jd9366->reset_gpio_num >= 0) {
        gpio_reset_pin(jd9366->reset_gpio_num);
    }
    // Delete MIPI DPI panel
    jd9366->del(panel);
    ESP_LOGD(TAG, "del jd9366 panel @%p", jd9366);
    free(jd9366);

    return ESP_OK;
}

static esp_err_t panel_jd9366_init(esp_lcd_panel_t *panel)
{
    jd9366_panel_t *jd9366 = (jd9366_panel_t *)panel->user_data;
    esp_lcd_panel_io_handle_t io = jd9366->io;
    const jd9366_lcd_init_cmd_t *init_cmds = NULL;
    uint16_t init_cmds_size = 0;
    bool is_cmd_overwritten = false;

    // Skip ID read - JD9366 panel doesn't respond to command 0x04 properly
    ESP_LOGI(TAG, "Skipping panel ID read (JD9366 doesn't support 0x04 command)");

    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LCD_CMD_MADCTL, (uint8_t[]) {
        jd9366->madctl_val,
    }, 1), TAG, "send command failed");

    // vendor specific initialization, it can be different between manufacturers
    // should consult the LCD supplier for initialization sequence code
    if (jd9366->init_cmds) {
        init_cmds = jd9366->init_cmds;
        init_cmds_size = jd9366->init_cmds_size;
    } else {
        init_cmds = vendor_specific_init_default;
        init_cmds_size = sizeof(vendor_specific_init_default) / sizeof(jd9366_lcd_init_cmd_t);
    }

    ESP_LOGI(TAG, "Sending %d initialization commands...", init_cmds_size);
    for (int i = 0; i < init_cmds_size; i++) {
        // Feed watchdog every 10 commands to prevent timeout
        if (i > 0 && i % 10 == 0) {
            vTaskDelay(1);
        }

        // Check if the command has been used or conflicts with the internal
        if (init_cmds[i].data_bytes > 0) {
            switch (init_cmds[i].cmd) {
            case LCD_CMD_MADCTL:
                is_cmd_overwritten = true;
                jd9366->madctl_val = ((uint8_t *)init_cmds[i].data)[0];
                break;
            default:
                is_cmd_overwritten = false;
                break;
            }

            if (is_cmd_overwritten) {
                is_cmd_overwritten = false;
                ESP_LOGW(TAG, "The %02Xh command has been used and will be overwritten by external initialization sequence",
                         init_cmds[i].cmd);
            }
        }

        // Send command
        ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, init_cmds[i].cmd, init_cmds[i].data, init_cmds[i].data_bytes), TAG, "send command failed");
        vTaskDelay(pdMS_TO_TICKS(init_cmds[i].delay_ms));
    }
    ESP_LOGI(TAG, "Init commands sent successfully");

    ESP_RETURN_ON_ERROR(jd9366->init(panel), TAG, "init MIPI DPI panel failed");

    return ESP_OK;
}

static esp_err_t panel_jd9366_reset(esp_lcd_panel_t *panel)
{
    jd9366_panel_t *jd9366 = (jd9366_panel_t *)panel->user_data;
    esp_lcd_panel_io_handle_t io = jd9366->io;

    // Perform hardware reset
    if (jd9366->reset_gpio_num >= 0) {
        gpio_set_level(jd9366->reset_gpio_num, !jd9366->flags.reset_level);
        vTaskDelay(pdMS_TO_TICKS(5));
        gpio_set_level(jd9366->reset_gpio_num, jd9366->flags.reset_level);
        vTaskDelay(pdMS_TO_TICKS(10));
        gpio_set_level(jd9366->reset_gpio_num, !jd9366->flags.reset_level);
        vTaskDelay(pdMS_TO_TICKS(120));
    } else if (io) { // Perform software reset
        ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LCD_CMD_SWRESET, NULL, 0), TAG, "send command failed");
        vTaskDelay(pdMS_TO_TICKS(120));
    }

    return ESP_OK;
}

static esp_err_t panel_jd9366_invert_color(esp_lcd_panel_t *panel, bool invert_color_data)
{
    jd9366_panel_t *jd9366 = (jd9366_panel_t *)panel->user_data;
    esp_lcd_panel_io_handle_t io = jd9366->io;
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

static esp_err_t panel_jd9366_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y)
{
    jd9366_panel_t *jd9366 = (jd9366_panel_t *)panel->user_data;
    esp_lcd_panel_io_handle_t io = jd9366->io;
    uint8_t madctl_val = jd9366->madctl_val;

    ESP_RETURN_ON_FALSE(io, ESP_ERR_INVALID_STATE, TAG, "invalid panel IO");

    // Control mirror through LCD command
    if (mirror_x) {
        madctl_val |= JD9366_CMD_GS_BIT;
    } else {
        madctl_val &= ~JD9366_CMD_GS_BIT;
    }
    if (mirror_y) {
        madctl_val |= JD9366_CMD_SS_BIT;
    } else {
        madctl_val &= ~JD9366_CMD_SS_BIT;
    }

    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LCD_CMD_MADCTL, (uint8_t []) {
        madctl_val
    }, 1), TAG, "send command failed");
    jd9366->madctl_val = madctl_val;

    return ESP_OK;
}

static esp_err_t panel_jd9366_disp_on_off(esp_lcd_panel_t *panel, bool on_off)
{
    jd9366_panel_t *jd9366 = (jd9366_panel_t *)panel->user_data;
    esp_lcd_panel_io_handle_t io = jd9366->io;
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
