/*
 * SPDX-FileCopyrightText: 2024-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/spi_master.h>
#include <driver/ledc.h>
#include <driver/i2c_master.h>
#include <driver/i2s_std.h>
#include <driver/rtc_io.h>
#include <driver/gpio.h>

#include "esp_vfs_fat.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_log.h"
#include "esp_check.h"
#include "esp_vfs_fat.h"
#include "esp_spiffs.h"
#include "esp_ldo_regulator.h"
#include "sd_pwr_ctrl_by_on_chip_ldo.h"
#include "esp_sleep.h"

#include "bsp/esp32_p4_eye.h"
#include "bsp_err_check.h"
#include "bsp/display.h"

#include "esp_cam_sensor_xclk.h"
#include "esp_codec_dev_defaults.h"

static const char *TAG = "p4-eye";

/**
 * @brief ESP32-P4-EYE I2S pinout
 *
 * Can be used for i2s_pdm_rx_gpio_config_t and/or i2s_pdm_rx_config_t initialization
 */
#define BSP_I2S_PDM_GPIO_CFG     \
    {                            \
        .clk = BSP_I2S_CLK,      \
        .din = BSP_I2S_DAT,      \
        .invert_flags = {        \
            .clk_inv = false,    \
        },                       \
    }

/**
 * @brief Mono Duplex I2S configuration structure
 *
 * This configuration is used by default in bsp_microphone_init()
 */
#define BSP_I2S_PDM_DUPLEX_MONO_CFG(_sample_rate)                                         \
    {                                                                                    \
        .clk_cfg = I2S_PDM_RX_CLK_DEFAULT_CONFIG(_sample_rate),                          \
        .slot_cfg = I2S_PDM_RX_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT,             \
                                                   I2S_SLOT_MODE_MONO),                  \
        .gpio_cfg = BSP_I2S_PDM_GPIO_CFG,                                                \
    }

#define LCD_CMD_BITS         (8)
#define LCD_PARAM_BITS       (8)
#define LCD_LEDC_CH          (CONFIG_BSP_DISPLAY_BRIGHTNESS_LEDC_CH)
#define LVGL_TICK_PERIOD_MS  (CONFIG_BSP_DISPLAY_LVGL_TICK)
#define LVGL_MAX_SLEEP_MS    (CONFIG_BSP_DISPLAY_LVGL_MAX_SLEEP)

static i2c_master_bus_handle_t i2c_bus_handle;

static esp_codec_dev_handle_t pdm_dev_handle;
static i2s_chan_handle_t i2s_pdm_rx_chan;
static const audio_codec_data_if_t *i2s_pdm_data_if = NULL;  /* Codec data interface */

static sd_pwr_ctrl_handle_t pwr_ctrl_handle = NULL;
static sdmmc_card_t *bsp_sdcard = NULL;    // Global uSD card handler
static bool i2c_initialized = false;

static esp_lcd_panel_io_handle_t io_handle = NULL;
static esp_lcd_panel_handle_t panel_handle = NULL;

static esp_cam_sensor_xclk_handle_t xclk_handle = NULL;

static knob_handle_t knob = NULL;

static lv_disp_t *disp = NULL;

static bool led_status = false;

/**
 * @brief LCD panel initialization commands.
 *
 */
typedef struct {
    int cmd;                /*<! The specific LCD command */
    const void *data;       /*<! Buffer that holds the command specific data */
    size_t data_bytes;      /*<! Size of `data` in memory, in bytes */
    unsigned int delay_ms;  /*<! Delay in milliseconds after this command */
} st7789_lcd_init_cmd_t;

static const st7789_lcd_init_cmd_t vendor_specific_init[] = {
    {0x11, (uint8_t []){0x00}, 1, 120},
    {0xB2, (uint8_t []){0x0C, 0x0C, 0x00, 0x33, 0x33}, 5, 0},
    {0x35, (uint8_t []){0x00}, 1, 0},
    {0x36, (uint8_t []){0x00}, 1, 0},
    {0x3A, (uint8_t []){0x05}, 1, 0},
    {0xB7, (uint8_t []){0x35}, 1, 0},
    {0xBB, (uint8_t []){0x2D}, 1, 0},
    {0xC0, (uint8_t []){0x2C}, 1, 0},
    {0xC2, (uint8_t []){0x01}, 1, 0},
    {0xC3, (uint8_t []){0x15}, 1, 0},
    {0xC4, (uint8_t []){0x20}, 1, 0},
    {0xC6, (uint8_t []){0x0F}, 1, 0},
    {0xD0, (uint8_t []){0xA4, 0xA1}, 2, 0},
    {0xD6, (uint8_t []){0xA1}, 1, 0},
    {0xE0, (uint8_t []){0x70, 0x05, 0x0A, 0x0B, 0x0A, 0x27, 0x2F, 0x44, 0x47, 0x37, 0x14, 0x14, 0x29, 0x2F}, 14, 0},
    {0xE1, (uint8_t []){0x70, 0x07, 0x0C, 0x08, 0x08, 0x04, 0x2F, 0x33, 0x46, 0x18, 0x15, 0x15, 0x2B, 0x2D}, 14, 0},
    {0x21, (uint8_t []){0x00}, 1, 0},
    {0x29, (uint8_t []){0x00}, 1, 0},
    {0x2C, (uint8_t []){0x00}, 1, 0},
};

static const button_config_t bsp_button_config[BSP_BUTTON_NUM] = {
    {
        .type = BUTTON_TYPE_GPIO,
        .gpio_button_config.active_level = 0,
        .gpio_button_config.gpio_num = BSP_BUTTON_NUM1
    },
    {
        .type = BUTTON_TYPE_GPIO,
        .gpio_button_config.active_level = 0,
        .gpio_button_config.gpio_num = BSP_BUTTON_NUM2
    },
    {
        .type = BUTTON_TYPE_GPIO,
        .gpio_button_config.active_level = 0,
        .gpio_button_config.gpio_num = BSP_BUTTON_NUM3
    },
    {
        .type = BUTTON_TYPE_GPIO,
        .gpio_button_config.active_level = 0,
        .gpio_button_config.gpio_num = BSP_BUTTON_ENCODER
    },
};

esp_err_t bsp_p4_eye_init(void)
{
    esp_cam_sensor_xclk_config_t cam_xclk_config = {
        .esp_clock_router_cfg = {
            .xclk_pin = BSP_CAMERA_XCLK_PIN,
            .xclk_freq_hz = BSP_MIPI_CAMERA_XCLK_FREQUENCY,
        }
    };
    ESP_ERROR_CHECK(esp_cam_sensor_xclk_allocate(ESP_CAM_SENSOR_XCLK_ESP_CLOCK_ROUTER, &xclk_handle));
    ESP_ERROR_CHECK(esp_cam_sensor_xclk_start(xclk_handle, &cam_xclk_config));

    const gpio_config_t sdcard_io_config = {
        .pin_bit_mask = BIT64(BSP_SD_EN_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    ESP_ERROR_CHECK(gpio_config(&sdcard_io_config));

    const gpio_config_t rst_io_config = {
        .pin_bit_mask = BIT64(BSP_CAMERA_RST_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    ESP_ERROR_CHECK(gpio_config(&rst_io_config));

    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
    esp_sleep_pd_config(ESP_PD_DOMAIN_VDDSDIO, ESP_PD_OPTION_ON);

    rtc_gpio_init(BSP_CAMERA_EN_PIN);
    rtc_gpio_set_direction(BSP_CAMERA_EN_PIN, RTC_GPIO_MODE_OUTPUT_ONLY);
    rtc_gpio_pulldown_dis(BSP_CAMERA_EN_PIN);
    rtc_gpio_pullup_dis(BSP_CAMERA_EN_PIN);
    rtc_gpio_hold_dis(BSP_CAMERA_EN_PIN);

#if CONFIG_BSP_CONTROL_C6_EN_PIN
    rtc_gpio_init(BSP_C6_EN_PIN);
    rtc_gpio_set_direction(BSP_C6_EN_PIN, RTC_GPIO_MODE_OUTPUT_ONLY);
    rtc_gpio_pulldown_dis(BSP_C6_EN_PIN);
    rtc_gpio_pullup_dis(BSP_C6_EN_PIN);
    rtc_gpio_hold_dis(BSP_C6_EN_PIN);
#endif

    gpio_set_level(BSP_SD_EN_PIN, 0);

    rtc_gpio_set_level(BSP_CAMERA_EN_PIN, 1);
    rtc_gpio_hold_en(BSP_CAMERA_EN_PIN);

    gpio_set_level(BSP_CAMERA_RST_PIN, 1);

    return ESP_OK;
}

esp_err_t bsp_enter_sleep_init(void)
{
    rtc_gpio_hold_dis(BSP_CAMERA_EN_PIN);
    rtc_gpio_set_level(BSP_CAMERA_EN_PIN, 0);
    rtc_gpio_hold_en(BSP_CAMERA_EN_PIN);

#if CONFIG_BSP_CONTROL_C6_EN_PIN
    rtc_gpio_hold_dis(BSP_C6_EN_PIN);
    rtc_gpio_set_level(BSP_C6_EN_PIN, 0);
    rtc_gpio_hold_en(BSP_C6_EN_PIN);
#endif

    ESP_ERROR_CHECK(esp_cam_sensor_xclk_stop(xclk_handle));

    return ESP_OK;
}

esp_err_t bsp_i2c_init(void)
{
    /* I2C was initialized before */
    if (i2c_initialized) {
        return ESP_OK;
    }

    i2c_master_bus_config_t i2c_mst_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = BSP_I2C_NUM,
        .scl_io_num = BSP_I2C_SCL,
        .sda_io_num = BSP_I2C_SDA,
        .flags.enable_internal_pullup = false,  // no pull-up
    };
    BSP_ERROR_CHECK_RETURN_ERR(i2c_new_master_bus(&i2c_mst_config, &i2c_bus_handle));

    i2c_initialized = true;

    return ESP_OK;
}

esp_err_t bsp_i2c_deinit(void)
{
    BSP_ERROR_CHECK_RETURN_ERR(i2c_del_master_bus(i2c_bus_handle));
    return ESP_OK;
}

esp_err_t bsp_get_i2c_bus_handle(i2c_master_bus_handle_t *handle)
{
    if (handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    *handle = i2c_bus_handle;
    return ESP_OK;
}

esp_err_t bsp_spiffs_mount(void)
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = CONFIG_BSP_SPIFFS_MOUNT_POINT,
        .partition_label = CONFIG_BSP_SPIFFS_PARTITION_LABEL,
        .max_files = CONFIG_BSP_SPIFFS_MAX_FILES,
#ifdef CONFIG_BSP_SPIFFS_FORMAT_ON_MOUNT_FAIL
        .format_if_mount_failed = true,
#else
        .format_if_mount_failed = false,
#endif
    };

    esp_err_t ret_val = esp_vfs_spiffs_register(&conf);

    BSP_ERROR_CHECK_RETURN_ERR(ret_val);

    size_t total = 0, used = 0;
    ret_val = esp_spiffs_info(conf.partition_label, &total, &used);
    if (ret_val != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret_val));
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }

    return ret_val;
}

esp_err_t bsp_spiffs_unmount(void)
{
    return esp_vfs_spiffs_unregister(CONFIG_BSP_SPIFFS_PARTITION_LABEL);
}

esp_err_t bsp_sdcard_mount(void)
{
    if (bsp_sdcard != NULL) {
        return ESP_OK;
    }

    const esp_vfs_fat_sdmmc_mount_config_t mount_config = {
#ifdef CONFIG_BSP_SD_FORMAT_ON_MOUNT_FAIL
        .format_if_mount_failed = true,
#else
        .format_if_mount_failed = false,
#endif
        .max_files = 5,
        .allocation_unit_size = 64 * 1024
    };

    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    host.slot = SDMMC_HOST_SLOT_0;
    host.max_freq_khz = SDMMC_FREQ_HIGHSPEED;

    sd_pwr_ctrl_ldo_config_t ldo_config = {
        .ldo_chan_id = 4,
    };

    esp_err_t ret = sd_pwr_ctrl_new_on_chip_ldo(&ldo_config, &pwr_ctrl_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create a new on-chip LDO power control driver");
        return ret;
    }
    host.pwr_ctrl_handle = pwr_ctrl_handle;

    const sdmmc_slot_config_t slot_config = {
        BSP_SD_SLOT_0_DEFAULT_INIT,
        .cd = SDMMC_SLOT_NO_CD,
        .wp = SDMMC_SLOT_NO_WP,
        .width = 4,
        .flags = 0,
    };

    return esp_vfs_fat_sdmmc_mount(BSP_SD_MOUNT_POINT, &host, &slot_config, &mount_config, &bsp_sdcard);
}

esp_err_t bsp_sdcard_unmount(void)
{
    if (bsp_sdcard == NULL) {
        return ESP_OK;
    }

    esp_vfs_fat_sdcard_unmount(BSP_SD_MOUNT_POINT, bsp_sdcard);
    bsp_sdcard = NULL;

    esp_err_t ret = sd_pwr_ctrl_del_on_chip_ldo(pwr_ctrl_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to delete the on-chip LDO power control driver");
    }

    return ret;
}

esp_err_t bsp_sdcard_detect_init(void)
{
    const gpio_config_t sd_detect_config = {
        .pin_bit_mask = BIT64(BSP_SD_DETECT_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    BSP_ERROR_CHECK_RETURN_ERR(gpio_config(&sd_detect_config));

    return ESP_OK;
}

bool bsp_sdcard_is_present(void)
{
    return gpio_get_level(BSP_SD_DETECT_PIN) == 0;
}

esp_err_t bsp_get_sdcard_handle(sdmmc_card_t **card)
{
    if (card == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    *card = bsp_sdcard;
    return ESP_OK;
}

esp_err_t bsp_knob_init(void)
{
    // Initialize knob
    knob_config_t cfg = {
        .default_direction = 0,
        .gpio_encoder_a = BSP_KNOB_A,
        .gpio_encoder_b = BSP_KNOB_B,
    };

    // Create knob instance
    knob = iot_knob_create(&cfg);
    if (knob == NULL) {
        ESP_LOGE(TAG, "Failed to create knob");
    }

    return ESP_OK;
}

esp_err_t bsp_knob_register_cb(knob_event_t event, knob_cb_t cb, void *usr_data)
{
    if (knob == NULL) {
        ESP_LOGE(TAG, "Knob not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    return iot_knob_register_cb(knob, event, cb, usr_data);
}

esp_err_t bsp_display_brightness_init(void)
{
    // Setup LEDC peripheral for PWM backlight control
    const ledc_channel_config_t LCD_backlight_channel = {
        .gpio_num = BSP_LCD_BACKLIGHT,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LCD_LEDC_CH,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = 1,
        .duty = 0,
        .hpoint = 0,
        .flags.output_invert = true
    };
    const ledc_timer_config_t LCD_backlight_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num = 1,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK
    };

    BSP_ERROR_CHECK_RETURN_ERR(ledc_timer_config(&LCD_backlight_timer));
    BSP_ERROR_CHECK_RETURN_ERR(ledc_channel_config(&LCD_backlight_channel));

    return ESP_OK;
}

esp_err_t bsp_display_brightness_set(int brightness_percent)
{
    if (brightness_percent > 100) {
        brightness_percent = 100;
    } else if (brightness_percent < 0) {
        brightness_percent = 0;
    }

    ESP_LOGI(TAG, "Setting LCD backlight: %d%%", brightness_percent);
    // LEDC resolution set to 10bits, thus: 100% = 1023
    uint32_t duty_cycle = (1023 * brightness_percent) / 100;
    BSP_ERROR_CHECK_RETURN_ERR(ledc_set_duty(LEDC_LOW_SPEED_MODE, LCD_LEDC_CH, duty_cycle));
    BSP_ERROR_CHECK_RETURN_ERR(ledc_update_duty(LEDC_LOW_SPEED_MODE, LCD_LEDC_CH));

    return ESP_OK;
}

esp_err_t bsp_display_backlight_off(void)
{
    return bsp_display_brightness_set(0);
}

esp_err_t bsp_display_backlight_on(void)
{
    return bsp_display_brightness_set(100);
}

esp_err_t bsp_display_new(const bsp_display_config_t *config, esp_lcd_panel_handle_t *ret_panel, esp_lcd_panel_io_handle_t *ret_io)
{
    esp_err_t ret = ESP_OK;
    assert(config != NULL && config->max_transfer_sz > 0);

    ESP_RETURN_ON_ERROR(bsp_display_brightness_init(), TAG, "Brightness init failed");

    ESP_LOGD(TAG, "Initialize SPI bus");
    const spi_bus_config_t buscfg = {
        .sclk_io_num = BSP_LCD_SPI_CLK,
        .mosi_io_num = BSP_LCD_SPI_MOSI,
        .miso_io_num = GPIO_NUM_NC,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .max_transfer_sz = config->max_transfer_sz,
    };
    ESP_RETURN_ON_ERROR(spi_bus_initialize(BSP_LCD_SPI_NUM, &buscfg, SPI_DMA_CH_AUTO), TAG, "SPI init failed");

    ESP_LOGD(TAG, "Install panel IO");
    const esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = BSP_LCD_DC,
        .cs_gpio_num = BSP_LCD_SPI_CS,
        .pclk_hz = BSP_LCD_PIXEL_CLOCK_HZ,
        .lcd_cmd_bits = LCD_CMD_BITS,
        .lcd_param_bits = LCD_PARAM_BITS,
        .spi_mode = 3,
        .trans_queue_depth = 2,
    };
    if (io_config.pclk_hz != 20 * 1000 * 1000) {
        ESP_LOGW(TAG, "If the pixel clock is not set to 20 MHz, you need to temporarily apply the patch 0001-fix-spi-default-clock-source.patch. For details on how to apply the patch, please refer to the README.");
    }
    ESP_GOTO_ON_ERROR(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)BSP_LCD_SPI_NUM, &io_config, ret_io), err, TAG, "New panel IO failed");

    ESP_LOGD(TAG, "Install LCD driver");
    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = BSP_LCD_RST,
        .color_space = BSP_LCD_COLOR_SPACE,
        .bits_per_pixel = BSP_LCD_BITS_PER_PIXEL,
    };
    ESP_GOTO_ON_ERROR(esp_lcd_new_panel_st7789(*ret_io, &panel_config, ret_panel), err, TAG, "New panel failed");

    const st7789_lcd_init_cmd_t *cmd = vendor_specific_init;
    uint16_t cmd_size = sizeof(vendor_specific_init) / sizeof(st7789_lcd_init_cmd_t);
    for (uint16_t i = 0; i < cmd_size; i++) {
        ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(*ret_io, cmd[i].cmd, cmd[i].data, cmd[i].data_bytes), TAG, "send command failed");
        vTaskDelay(pdMS_TO_TICKS(cmd[i].delay_ms));
    }

    esp_lcd_panel_reset(*ret_panel);
    esp_lcd_panel_init(*ret_panel);
    esp_lcd_panel_invert_color(*ret_panel, true);
    return ret;

err:
    if (*ret_panel) {
        esp_lcd_panel_del(*ret_panel);
    }
    if (*ret_io) {
        esp_lcd_panel_io_del(*ret_io);
    }
    spi_bus_free(BSP_LCD_SPI_NUM);
    return ret;
}

esp_err_t bsp_display_del(void)
{
    esp_lcd_panel_del(panel_handle);
    esp_lcd_panel_io_del(io_handle);
    spi_bus_free(BSP_LCD_SPI_NUM);
    return ESP_OK;
}

static lv_disp_t *bsp_display_lcd_init(const bsp_display_cfg_t *cfg)
{
    assert(cfg != NULL);
    const bsp_display_config_t bsp_disp_cfg = {
        .max_transfer_sz = BSP_LCD_H_RES * 10 * sizeof(uint16_t),
    };
    BSP_ERROR_CHECK_RETURN_NULL(bsp_display_new(&bsp_disp_cfg, &panel_handle, &io_handle));

    esp_lcd_panel_disp_on_off(panel_handle, true);

    /* Add LCD screen */
    ESP_LOGD(TAG, "Add LCD screen");
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = io_handle,
        .panel_handle = panel_handle,
        .buffer_size = cfg->buffer_size,
        .double_buffer = cfg->double_buffer,
        .hres = BSP_LCD_H_RES,
        .vres = BSP_LCD_V_RES,
        .monochrome = false,
        /* Rotation values must be same as used in esp_lcd for initial settings of the screen */
        .rotation = {
            .swap_xy = false,
            .mirror_x = false,
            .mirror_y = false,
        },
        .flags = {
            .buff_dma = cfg->flags.buff_dma,
            .buff_spiram = cfg->flags.buff_spiram,
            .sw_rotate = true,
        }
    };

    return lvgl_port_add_disp(&disp_cfg);
}

lv_disp_t *bsp_display_start(void)
{
    bsp_display_cfg_t cfg = {
        .lvgl_port_cfg = {
            .task_priority = CONFIG_BSP_DISPLAY_LVGL_TASK_PRIORITY,
            .task_stack = 4096,
            .task_affinity = 0,
            .timer_period_ms = LVGL_TICK_PERIOD_MS,
            .task_max_sleep_ms = LVGL_MAX_SLEEP_MS,
        },
        .buffer_size = BSP_LCD_DRAW_BUFF_SIZE,
        .double_buffer = BSP_LCD_DRAW_BUFF_DOUBLE,
        .flags = {
            .buff_dma = false,
            .buff_spiram = true,
        }
    };
    return bsp_display_start_with_config(&cfg);
}

esp_err_t bsp_display_enter_sleep(void)
{
    bsp_display_backlight_off();
    esp_lcd_panel_disp_sleep(panel_handle, true);

    return ESP_OK;
}

lv_disp_t *bsp_display_start_with_config(const bsp_display_cfg_t *cfg)
{
    assert(cfg != NULL);
    BSP_ERROR_CHECK_RETURN_NULL(lvgl_port_init(&cfg->lvgl_port_cfg));
    BSP_NULL_CHECK(disp = bsp_display_lcd_init(cfg), NULL);

    return disp;
}

lv_indev_t *bsp_display_get_input_dev(void)
{
    return NULL;
}

esp_err_t bsp_display_rotate(lv_disp_rot_t rotation)
{
    lv_disp_set_rotation(disp, rotation);
    return ESP_OK;
}

bool bsp_display_lock(uint32_t timeout_ms)
{
    return lvgl_port_lock(timeout_ms);
}

void bsp_display_unlock(void)
{
    lvgl_port_unlock();
}

esp_err_t bsp_flashlight_init(void)
{
    const gpio_config_t led_io_config = {
        .pin_bit_mask = BIT64(BSP_LED_FLASHLIGHT),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    BSP_ERROR_CHECK_RETURN_ERR(gpio_config(&led_io_config));

    return ESP_OK;
}

esp_err_t bsp_flashlight_set(const bool on)
{
    BSP_ERROR_CHECK_RETURN_ERR(gpio_set_level(BSP_LED_FLASHLIGHT, on));

    led_status = on;

    return ESP_OK;
}

bool bsp_get_flashlight_status(void)
{
    return led_status;
}

esp_err_t bsp_extra_pdm_i2s_read(void *audio_buffer, size_t len, size_t *bytes_read, uint32_t timeout_ms)
{
    esp_err_t ret = ESP_OK;
    ret = esp_codec_dev_read(pdm_dev_handle, audio_buffer, len);
    *bytes_read = len;
    return ret;
}

esp_err_t bsp_pdm_audio_init(const i2s_pdm_rx_config_t *i2s_config)
{
    if (i2s_pdm_rx_chan) {
        /* microphone was initialized before */
        return ESP_OK;
    }

    ESP_LOGI(TAG, "bsp_microphone_init: %d", I2S_NUM_0);

    /* Setup I2S peripheral */
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, NULL, &i2s_pdm_rx_chan));

    /* Setup I2S channels */
    const i2s_pdm_rx_config_t pdm_cfg_default = BSP_I2S_PDM_DUPLEX_MONO_CFG(16000);
    const i2s_pdm_rx_config_t *p_i2s_pdm_cfg = &pdm_cfg_default;
    if (i2s_config != NULL) {
        p_i2s_pdm_cfg = i2s_config;
    }

    if (i2s_pdm_rx_chan != NULL) {
        ESP_ERROR_CHECK(i2s_channel_init_pdm_rx_mode(i2s_pdm_rx_chan, p_i2s_pdm_cfg));
        ESP_ERROR_CHECK(i2s_channel_enable(i2s_pdm_rx_chan));
    }

    audio_codec_i2s_cfg_t i2s_cfg = {
        .port = I2S_NUM_0,
        .rx_handle = i2s_pdm_rx_chan,
        .tx_handle = NULL,
    };
    i2s_pdm_data_if = audio_codec_new_i2s_data(&i2s_cfg);
    BSP_NULL_CHECK(i2s_pdm_data_if, NULL);

    return ESP_OK;
}

esp_codec_dev_handle_t bsp_audio_pdm_microphone_init(void)
{
    if (i2s_pdm_data_if == NULL) {
        /* Configure I2S PDM peripheral */
        BSP_ERROR_CHECK_RETURN_ERR(bsp_pdm_audio_init(NULL));
    }
    assert(i2s_pdm_data_if);

    esp_codec_dev_cfg_t codec_dev_cfg = {
        .dev_type = ESP_CODEC_DEV_TYPE_IN,
        .codec_if = NULL,
        .data_if = i2s_pdm_data_if,
    };
    return esp_codec_dev_new(&codec_dev_cfg);
}

esp_err_t bsp_extra_pdm_codec_init()
{
    pdm_dev_handle = bsp_audio_pdm_microphone_init();
    assert((pdm_dev_handle) && "pdm_dev_handle not initialized");

    esp_codec_dev_sample_info_t pdm_fs = {
        .sample_rate = 16000,
        .channel = 2,
        .bits_per_sample = 16,
    };
    esp_codec_dev_open(pdm_dev_handle, &pdm_fs);

    return ESP_OK;
}

esp_err_t bsp_iot_button_create(button_handle_t btn_array[], int *btn_cnt, int btn_array_size)
{
    esp_err_t ret = ESP_OK;
    if ((btn_array_size < BSP_BUTTON_NUM) ||
            (btn_array == NULL)) {
        return ESP_ERR_INVALID_ARG;
    }

    if (btn_cnt) {
        *btn_cnt = 0;
    }
    for (int i = 0; i < BSP_BUTTON_NUM; i++) {
        btn_array[i] = iot_button_create(&bsp_button_config[i]);
        if (btn_array[i] == NULL) {
            ret = ESP_FAIL;
            break;
        }
        if (btn_cnt) {
            (*btn_cnt)++;
        }
    }
    return ret;
}
