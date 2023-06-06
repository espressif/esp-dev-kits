/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "esp_timer.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/rmt_tx.h"
#include "led_strip.h"
#include "led_strip_interface.h"

#include "esp32-c3-lcdkit.h"
#include "esp_lcd_gc9a01.h"
#include "iot_knob.h"
#include "esp_lvgl_port.h"

static const char *TAG = "C3-LCD-EV-BOARD";

/* Assert on error, if selected in menuconfig. Otherwise return error code. */
//TODO: Move this code into common header
#if CONFIG_BSP_ERROR_CHECK
#define BSP_ERROR_CHECK_RETURN_ERR(x)    ESP_ERROR_CHECK(x)
#define BSP_ERROR_CHECK_RETURN_NULL(x)   ESP_ERROR_CHECK(x)
#define BSP_NULL_CHECK(x, ret)           assert(x)
#define BSP_NULL_CHECK_GOTO(x, goto_tag) assert(x)
#else
#define BSP_ERROR_CHECK_RETURN_ERR(x) do { \
        esp_err_t err_rc_ = (x);            \
        if (unlikely(err_rc_ != ESP_OK)) {  \
            return err_rc_;                 \
        }                                   \
    } while(0)

#define BSP_ERROR_CHECK_RETURN_NULL(x)  do { \
        if (unlikely((x) != ESP_OK)) {      \
            return NULL;                    \
        }                                   \
    } while(0)

#define BSP_NULL_CHECK(x, ret) do { \
        if ((x) == NULL) {          \
            return ret;             \
        }                           \
    } while(0)

#define BSP_NULL_CHECK_GOTO(x, goto_tag) do { \
        if ((x) == NULL) {      \
            goto goto_tag;      \
        }                       \
    } while(0)
#endif

static lv_disp_t *disp;
static led_strip_handle_t led_strip;
static i2s_chan_handle_t i2s_tx_chan;

/**
 * @brief led configuration structure
 *
 * This configuration is used by default in bsp_led_init()
 */
led_strip_config_t strip_config = {
    .strip_gpio_num = BSP_RGB_CTRL,
    .max_leds = 1,
    .led_pixel_format = LED_PIXEL_FORMAT_GRB,
    .led_model = LED_MODEL_WS2812,
    .flags.invert_out = false,
};

led_strip_rmt_config_t rmt_config = {
    .clk_src = RMT_CLK_SRC_DEFAULT,
    .resolution_hz = 10 * 1000 * 1000,
    .flags.with_dma = false,
};

esp_err_t bsp_led_init()
{
    ESP_LOGI(TAG, "BLINK_GPIO setting %d", strip_config.strip_gpio_num);

    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    led_strip_set_pixel(led_strip, 0, 0x00, 0x00, 0x00);
    led_strip_refresh(led_strip);

    return ESP_OK;
}

esp_err_t bsp_led_RGB_set(uint8_t r, uint8_t g, uint8_t b)
{
    esp_err_t ret = ESP_OK;
    uint32_t index = 0;

    ret |= led_strip_set_pixel(led_strip, index, r, g, b);
    ret |= led_strip_refresh(led_strip);
    return ret;
}

esp_err_t bsp_audio_write(void *audio_buffer, size_t len, size_t *bytes_written, uint32_t timeout_ms)
{
    esp_err_t ret = ESP_OK;
    ret = i2s_channel_write(i2s_tx_chan, (char *)audio_buffer, len, bytes_written, timeout_ms);
    return ret;
}

esp_err_t bsp_audio_reconfig_clk(uint32_t rate, uint32_t bits_cfg, i2s_slot_mode_t ch)
{
    esp_err_t ret = ESP_OK;

    i2s_pdm_tx_config_t pdm_cfg = {
        .clk_cfg = I2S_PDM_TX_CLK_DEFAULT_CONFIG(rate),
        .slot_cfg = I2S_PDM_TX_SLOT_DEFAULT_CONFIG((i2s_data_bit_width_t)bits_cfg, (i2s_slot_mode_t)ch),
        .gpio_cfg = BSP_I2S_GPIO_CFG,
    };

    ret |= i2s_channel_disable(i2s_tx_chan);
    ret |= i2s_channel_reconfig_pdm_tx_clock(i2s_tx_chan, &pdm_cfg.clk_cfg);
    ret |= i2s_channel_reconfig_pdm_tx_slot(i2s_tx_chan, &pdm_cfg.slot_cfg);
    ret |= i2s_channel_enable(i2s_tx_chan);

    vTaskDelay(pdMS_TO_TICKS(10));
    return ret;
}

static esp_err_t bsp_audio_init(const i2s_pdm_tx_config_t *i2s_config, i2s_chan_handle_t *tx_channel)
{
    /* Setup I2S peripheral */
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    chan_cfg.auto_clear = true; // Auto clear the legacy data in the DMA buffer
    BSP_ERROR_CHECK_RETURN_ERR(i2s_new_channel(&chan_cfg, tx_channel, NULL));

    /* Setup I2S channels */
    const i2s_pdm_tx_config_t pdm_cfg_default = BSP_I2S_DUPLEX_MONO_CFG(44100);
    const i2s_pdm_tx_config_t *p_i2s_cfg = &pdm_cfg_default;
    if (i2s_config != NULL) {
        p_i2s_cfg = i2s_config;
    }

    if (tx_channel != NULL) {
        BSP_ERROR_CHECK_RETURN_ERR(i2s_channel_init_pdm_tx_mode(*tx_channel, p_i2s_cfg));
        BSP_ERROR_CHECK_RETURN_ERR(i2s_channel_enable(*tx_channel));
    }

#if HARDWARE_V1_0
    const gpio_config_t pa_io_config = {
        .pin_bit_mask = BIT64(BSP_I2S_CTRL),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&pa_io_config);
    gpio_set_level(BSP_I2S_CTRL, 1);
#endif

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

static void encoder_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    static int32_t last_v = 0;

    assert(indev_drv);
    assert(indev_drv->user_data);
    knob_handle_t knob_handle = (knob_handle_t)indev_drv->user_data;

    int32_t invd = iot_knob_get_count_value(knob_handle);
    knob_event_t event = iot_knob_get_event(knob_handle);

    if (last_v ^ invd) {
        last_v = invd;
        data->enc_diff = (KNOB_LEFT == event) ? (-1) : ((KNOB_RIGHT == event) ? (1) : (0));
    } else {
        data->enc_diff = 0;
    }
    data->state = (true == bsp_button_get(BSP_BTN_PRESS)) ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}

// Bit number used to represent command and parameter
#define LCD_CMD_BITS           8
#define LCD_PARAM_BITS         8
#define LCD_LEDC_DUTY_RES      (LEDC_TIMER_10_BIT)
#define LCD_LEDC_CH            CONFIG_BSP_DISPLAY_BRIGHTNESS_LEDC_CH

static bool bsp_dispaly_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    lv_disp_t **disp = (lv_disp_t **)user_ctx;

    lvgl_port_flush_ready(*disp);
    return false;
}

static lv_disp_t *bsp_display_lcd_init(void)
{
    ESP_LOGD(TAG, "Initialize SPI bus");
    const spi_bus_config_t buscfg = {
        .sclk_io_num = BSP_LCD_PCLK,
        .mosi_io_num = BSP_LCD_DATA0,
        .miso_io_num = GPIO_NUM_NC,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .max_transfer_sz = BSP_LCD_H_RES * 80 * sizeof(uint16_t),
    };
    BSP_ERROR_CHECK_RETURN_NULL(spi_bus_initialize(BSP_LCD_SPI_NUM, &buscfg, SPI_DMA_CH_AUTO));

    ESP_LOGD(TAG, "Install panel IO");
    esp_lcd_panel_io_handle_t io_handle = NULL;
    const esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = BSP_LCD_DC,
        .cs_gpio_num = BSP_LCD_CS,
        .pclk_hz = BSP_LCD_PIXEL_CLOCK_HZ,
        .lcd_cmd_bits = LCD_CMD_BITS,
        .lcd_param_bits = LCD_PARAM_BITS,
        .spi_mode = 0,
        .trans_queue_depth = 10,
        .on_color_trans_done = bsp_dispaly_flush_ready,
        .user_ctx = &disp,
    };

    // Attach the LCD to the SPI bus
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)BSP_LCD_SPI_NUM, &io_config, &io_handle));

    ESP_LOGD(TAG, "Install LCD driver of gc9a01");
    esp_lcd_panel_handle_t panel_handle = NULL;
    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = BSP_LCD_RST,
        .color_space = ESP_LCD_COLOR_SPACE_BGR,
        .bits_per_pixel = 16,
    };
    BSP_ERROR_CHECK_RETURN_NULL(esp_lcd_new_panel_gc9a01(io_handle, &panel_config, &panel_handle));

    BSP_ERROR_CHECK_RETURN_ERR(esp_lcd_panel_reset(panel_handle));
    BSP_ERROR_CHECK_RETURN_ERR(esp_lcd_panel_init(panel_handle));
    BSP_ERROR_CHECK_RETURN_ERR(esp_lcd_panel_invert_color(panel_handle, true));
    BSP_ERROR_CHECK_RETURN_ERR(esp_lcd_panel_mirror(panel_handle, true, false));
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
    BSP_ERROR_CHECK_RETURN_ERR(esp_lcd_panel_disp_on_off(panel_handle, true));
#else
    BSP_ERROR_CHECK_RETURN_ERR(esp_lcd_panel_disp_off(panel_handle, false));
#endif

    /* Add LCD screen */
    ESP_LOGD(TAG, "Add LCD screen");
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = io_handle,
        .panel_handle = panel_handle,
        .buffer_size = BSP_LCD_H_RES * BSP_LCD_DRAW_BUF_HEIGHT,
        .double_buffer = BSP_LCD_DRAW_BUF_DOUBLE,
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
            .buff_dma = true,
        }
    };

    return lvgl_port_add_disp(&disp_cfg);
}

static lv_indev_t *bsp_display_indev_init(lv_disp_t *disp)
{
    static lv_indev_drv_t indev_drv;
    lv_indev_t *indev_encoder;
    knob_handle_t knob_handle;

    bsp_button_init(BSP_BTN_PRESS);

    knob_config_t *cfg = calloc(1, sizeof(knob_config_t));
    cfg->default_direction = 0;
    cfg->gpio_encoder_a = BSP_ENCODER_A;
    cfg->gpio_encoder_b = BSP_ENCODER_B;
    knob_handle = iot_knob_create(cfg);

    /* Register a touchpad input device */
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_ENCODER;
    indev_drv.read_cb = encoder_read;
    indev_drv.user_data = knob_handle;
    indev_encoder = lv_indev_drv_register(&indev_drv);
    BSP_NULL_CHECK(indev_encoder, ESP_ERR_NO_MEM);

    return indev_encoder;
}

static esp_err_t bsp_display_brightness_init(void)
{
    // Setup LEDC peripheral for PWM backlight control
    const ledc_channel_config_t LCD_backlight_channel = {
        .gpio_num = BSP_LCD_BACKLIGHT,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LCD_LEDC_CH,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_1,
        .duty = 0,
        .hpoint = 0
    };
    const ledc_timer_config_t LCD_backlight_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LCD_LEDC_DUTY_RES,
        .timer_num = LEDC_TIMER_1,
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
    }
    if (brightness_percent < 0) {
        brightness_percent = 0;
    }
#if HARDWARE_V1_0
    brightness_percent = 100 - brightness_percent;
#endif
    ESP_LOGD(TAG, "Setting LCD backlight: %d%%", brightness_percent);
    uint32_t duty_cycle = (BIT(LCD_LEDC_DUTY_RES) * (brightness_percent)) / 100;
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

lv_disp_t *bsp_display_start(void)
{
    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    BSP_ERROR_CHECK_RETURN_NULL(lvgl_port_init(&lvgl_cfg));

    BSP_ERROR_CHECK_RETURN_NULL(bsp_display_brightness_init());

    BSP_NULL_CHECK(disp = bsp_display_lcd_init(), NULL);

    BSP_NULL_CHECK(bsp_display_indev_init(disp), NULL);

    return disp;
}

void bsp_display_rotate(lv_disp_t *disp, lv_disp_rot_t rotation)
{
    lv_disp_set_rotation(disp, rotation);
}

bool bsp_display_lock(uint32_t timeout_ms)
{
    return lvgl_port_lock(timeout_ms);
}

void bsp_display_unlock(void)
{
    lvgl_port_unlock();
}

esp_err_t bsp_button_init(const bsp_button_t btn)
{
    const gpio_config_t button_io_config = {
        .pin_bit_mask = BIT64(btn),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    return gpio_config(&button_io_config);
}

bool bsp_button_get(const bsp_button_t btn)
{
    return !(bool)gpio_get_level(btn);
}

esp_err_t bsp_board_init(void)
{
    ESP_ERROR_CHECK(bsp_led_init());

    ESP_ERROR_CHECK(bsp_spiffs_mount());

    /* Configure I2S peripheral */
    i2s_pdm_tx_config_t pdm_cfg = {
        .clk_cfg = I2S_PDM_TX_CLK_DEFAULT_CONFIG(44100),
        .slot_cfg = I2S_PDM_TX_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = BSP_I2S_GPIO_CFG,
    };
    ESP_ERROR_CHECK(bsp_audio_init(&pdm_cfg, &i2s_tx_chan));

    return ESP_OK;
}
