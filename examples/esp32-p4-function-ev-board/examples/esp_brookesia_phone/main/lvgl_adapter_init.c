/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Custom 800x1280 MIPI-DSI panel bring-up (JD9366 / ILI9881C), replacing the
 * stock BSP EK79007 display. Only the panel/io handles' origin changed: the
 * esp_lv_adapter registration flow stays identical to the upstream example.
 */
#include "lvgl_adapter_init.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_check.h"
#include "esp_log.h"
#include "esp_lv_adapter.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_mipi_dsi.h"
#include "esp_ldo_regulator.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"              /* i2c_master_probe */
#include "esp_lcd_panel_io.h"               /* esp_lcd_new_panel_io_i2c */
#include "esp_lcd_touch.h"
#include "esp_lcd_touch_jd9366.h"
#include "esp_lcd_touch_gt911.h"
#include "bsp/esp32_p4_function_ev_board.h" /* shared I2C bus (bsp_i2c_init) */

/* ------------------------------------------------------------------
 * Panel selection (compile-time via menuconfig "LCD Panel Selection").
 * Both panels are 800x1280 portrait, so the LVGL/UI layer is untouched.
 * ------------------------------------------------------------------ */
#if CONFIG_LCD_PANEL_ILI9881C
#include "esp_lcd_ili9881c.h"
#include "soc/mipi_dsi_host_struct.h" /* dsi_host_dev_t: DCS-ACK workaround below */
#include "soc/reg_base.h"            /* DR_REG_DSI_HOST_BASE */
typedef ili9881c_vendor_config_t panel_vendor_config_t;
#define PANEL_NAME                    "ILI9881C"
#define PANEL_DSI_BUS_CONFIG()        ILI9881C_PANEL_BUS_DSI_2CH_CONFIG()
#define PANEL_IO_DBI_CONFIG()         ILI9881C_PANEL_IO_DBI_CONFIG()
#define PANEL_DPI_CONFIG(px)          ILI9881C_800_1280_PANEL_60HZ_DPI_CONFIG(px)
#define PANEL_NEW_PANEL(io, cfg, out) esp_lcd_new_panel_ili9881c(io, cfg, out)
#define PANEL_DSI_DCS_ACK_DISABLE     1 /* module sends no DCS ACK/EoT response */
#if LV_COLOR_DEPTH != 16
#error "ILI9881C BOE init sequence pins COLMOD=0x55 (RGB565): keep LV_COLOR_DEPTH=16."
#endif
#else /* default: LCD_PANEL_JD9366 */
#include "esp_lcd_jd9366.h"
typedef jd9366_vendor_config_t panel_vendor_config_t;
#define PANEL_NAME                    "JD9366"
#define PANEL_DSI_BUS_CONFIG()        JD9366_PANEL_BUS_DSI_2CH_CONFIG()
#define PANEL_IO_DBI_CONFIG()         JD9366_PANEL_IO_DBI_CONFIG()
#define PANEL_DPI_CONFIG(px)          JD9366_800_1280_PANEL_60HZ_DPI_CONFIG(px)
#define PANEL_NEW_PANEL(io, cfg, out) esp_lcd_new_panel_jd9366(io, cfg, out)
#define PANEL_DSI_DCS_ACK_DISABLE     0 /* panel answers ACK, keep IDF default */
#endif

#if LV_COLOR_DEPTH == 16
#define MIPI_DPI_PX_FORMAT   (LCD_COLOR_PIXEL_FORMAT_RGB565)
#define LVGL_PX_FORMAT       (LV_COLOR_FORMAT_RGB565)
#define PANEL_BITS_PER_PIXEL (16)
#elif LV_COLOR_DEPTH == 24
#define MIPI_DPI_PX_FORMAT   (LCD_COLOR_PIXEL_FORMAT_RGB888)
#define LVGL_PX_FORMAT       (LV_COLOR_FORMAT_RGB888)
#define PANEL_BITS_PER_PIXEL (24)
#endif

#define PANEL_H_RES 800
#define PANEL_V_RES 1280

/* "VDD_MIPI_DPHY" must be powered with 2.5V, sourced here from the
 * internal LDO regulator (LDO_VO3) */
#define MIPI_DSI_PHY_PWR_LDO_CHAN       3
#define MIPI_DSI_PHY_PWR_LDO_VOLTAGE_MV 2500

/* Board pins follow the custom adapter board (GPIO47 is a level-switched
 * backlight enable, no PWM dimming; GPIO5 is the LCD reset, shared with
 * the JTAG TDO mux). */
#define PIN_NUM_BK_LIGHT       47
#define LCD_BK_LIGHT_ON_LEVEL  1
#define LCD_BK_LIGHT_OFF_LEVEL !LCD_BK_LIGHT_ON_LEVEL
#define PIN_NUM_LCD_RST        5

/* Touch — the panels share the same CTP module (Jadard @0x68 or
 * GT911 @0x5D/0x14, run-time probed). SDA/SCL hang on the shared I2C0
 * bus that the BSP codec already brings up; no touch reset line, the
 * address comes from power-on strapping. */
#define PIN_NUM_TOUCH_IIC_SCL 8
#define PIN_NUM_TOUCH_IIC_SDA 7
/* TSIX/GPIO21 exists on the module but never asserts on this adapter board
 * (the Jadard 0x40008081 enable is read-only - see the experiment log in
 * esp_lcd_touch_jd9366.c). Kept for reference only; the touch config below
 * must NOT claim it, or esp_lv_adapter switches to IRQ-gated polling and
 * stops reading the controller entirely. */
#define PIN_NUM_TOUCH_INT     21

#define LVGL_ADAPTER_BUFFER_HEIGHT 20

static const char *TAG = "lvgl_adapter_init";

static void adapter_enable_dsi_phy_power(void)
{
    /* Power up the MIPI DSI PHY from "no power" to "shutdown" state */
    esp_ldo_channel_handle_t ldo_mipi_phy = NULL;
    esp_ldo_channel_config_t ldo_mipi_phy_config = {
        .chan_id = MIPI_DSI_PHY_PWR_LDO_CHAN,
        .voltage_mv = MIPI_DSI_PHY_PWR_LDO_VOLTAGE_MV,
    };
    ESP_ERROR_CHECK(esp_ldo_acquire_channel(&ldo_mipi_phy_config, &ldo_mipi_phy));
    ESP_LOGI(TAG, "MIPI DSI PHY powered on");
}

static void adapter_init_lcd_backlight(void)
{
    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << PIN_NUM_BK_LIGHT,
    };
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));
}

static void adapter_set_lcd_backlight(uint32_t level)
{
    gpio_set_level(PIN_NUM_BK_LIGHT, level);
}

static void adapter_init_touch(lv_display_t *disp)
{
    /* The codec init has already brought up the shared I2C0 bus
     * (SDA=GPIO7 / SCL=GPIO8); bsp_i2c_init() is idempotent. */
    ESP_ERROR_CHECK(bsp_i2c_init());
    i2c_master_bus_handle_t i2c_bus = bsp_i2c_get_handle();
    if (i2c_bus == NULL) {
        ESP_LOGE(TAG, "I2C bus not available for touch");
        return;
    }

    /* Probe the CTP: Jadard @0x68 first, then GT911 @0x5D/0x14 (the
     * GT911 address depends on its INT pin level at power-up). */
    static const struct {
        uint16_t addr;
        const char *name;
    } probes[] = {
        { ESP_LCD_TOUCH_IO_I2C_JD9366_ADDRESS,       "JD9366 touch" },
        { ESP_LCD_TOUCH_IO_I2C_GT911_ADDRESS,        "GT911 touch"  },
        { ESP_LCD_TOUCH_IO_I2C_GT911_ADDRESS_BACKUP, "GT911 touch"  },
    };
    int found = -1;
    for (int i = 0; i < (int)(sizeof(probes) / sizeof(probes[0])); i++) {
        if (i2c_master_probe(i2c_bus, probes[i].addr, 100) == ESP_OK) {
            found = i;
            break;
        }
    }
    if (found < 0) {
        ESP_LOGW(TAG, "No CTP on the shared I2C bus (0x68/0x5D/0x14)");
        return;
    }
    ESP_LOGI(TAG, "Detected %s at 0x%02x", probes[found].name, probes[found].addr);

    esp_lcd_panel_io_handle_t tp_io = NULL;
    esp_lcd_panel_io_i2c_config_t tp_io_cfg;
    if (found == 0) {
        tp_io_cfg = (esp_lcd_panel_io_i2c_config_t)ESP_LCD_TOUCH_IO_I2C_JD9366_CONFIG();
        tp_io_cfg.scl_speed_hz = 100000; /* macro leaves it 0; set explicitly */
    } else {
        tp_io_cfg = (esp_lcd_panel_io_i2c_config_t)ESP_LCD_TOUCH_IO_I2C_GT911_CONFIG();
        tp_io_cfg.dev_addr = probes[found].addr; /* 0x5D or 0x14 */
    }
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(i2c_bus, &tp_io_cfg, &tp_io));

    /* Coordinates map 1:1 in the native portrait orientation; adjust
     * swap/mirror flags here if a module ships rotated.
     *
     * int_gpio_num stays GPIO_NUM_NC on purpose: esp_lv_adapter enables
     * IRQ-gated polling whenever the handle claims an INT pin (it reads the
     * controller only after an ISR-signalled semaphore). The INT line is
     * dead on this board, so that mode would hang the pointer forever.
     * Unconditional polling - the mode the yuying-ppa port is proven with -
     * keeps the CTP working without it. */
    const esp_lcd_touch_config_t tp_cfg = {
        .x_max = PANEL_H_RES,
        .y_max = PANEL_V_RES,
        .rst_gpio_num = GPIO_NUM_NC, /* no touch reset line on the board */
        .int_gpio_num = GPIO_NUM_NC, /* INT dead on this board -> poll */
    };
    esp_lcd_touch_handle_t tp = NULL;
    if (found == 0) {
        ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_jd9366(tp_io, &tp_cfg, &tp));
    } else {
        ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_gt911(tp_io, &tp_cfg, &tp));
    }

    esp_lv_adapter_touch_config_t touch_cfg = ESP_LV_ADAPTER_TOUCH_DEFAULT_CONFIG(disp, tp);
    if (esp_lv_adapter_register_touch(&touch_cfg) == NULL) {
        ESP_LOGE(TAG, "Register touch failed");
    }
}

lv_disp_t *lvgl_adapter_init(void)
{
    adapter_enable_dsi_phy_power();
    adapter_init_lcd_backlight();
    adapter_set_lcd_backlight(LCD_BK_LIGHT_OFF_LEVEL);
    /* The Setting app calls bsp_display_brightness_set() on start-up and
     * from its slider. That path drives the stock EV-board LEDC PWM pin,
     * which floats on this adapter board. Bring the LEDC up front so those
     * calls succeed instead of tripping ESP_ERROR_CHECK in the BSP. */
    ESP_ERROR_CHECK(bsp_display_brightness_init());

    /* Create the MIPI DSI bus first; it also initializes the DSI PHY */
    esp_lcd_dsi_bus_handle_t mipi_dsi_bus = NULL;
    esp_lcd_dsi_bus_config_t bus_config = PANEL_DSI_BUS_CONFIG();
    ESP_ERROR_CHECK(esp_lcd_new_dsi_bus(&bus_config, &mipi_dsi_bus));

    ESP_LOGI(TAG, "Install MIPI-DSI %s control panel", PANEL_NAME);
    esp_lcd_panel_io_handle_t mipi_dbi_io = NULL;
    /* DBI interface is used to send LCD commands and parameters */
    esp_lcd_dbi_io_config_t dbi_config = PANEL_IO_DBI_CONFIG();
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_dbi(mipi_dsi_bus, &dbi_config, &mipi_dbi_io));
#if PANEL_DSI_DCS_ACK_DISABLE
    /* The module never sends DCS ACK/EoT responses, while
     * esp_lcd_new_panel_io_dbi() enables ack_rqst_en and IDF sets all DSI
     * timeouts to 0 (infinite wait): the host stops after one DCS write,
     * the GEN CMD FIFO never drains and the next tx_param spins forever in
     * gen_cmd_full. Disable ACK requests so DCS writes become
     * fire-and-forget. */
    ((dsi_host_dev_t *)DR_REG_DSI_HOST_BASE)->cmd_mode_cfg.ack_rqst_en = 0;
    ESP_LOGI(TAG, "%s: DCS ACK request disabled (panel sends no EoT response)", PANEL_NAME);
#endif

    /* Create the DPI panel (pixel format follows LV_COLOR_DEPTH) */
    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_dpi_panel_config_t dpi_config = PANEL_DPI_CONFIG(MIPI_DPI_PX_FORMAT);
    /* esp_lv_adapter registers MIPI DSI displays in TRIPLE_PARTIAL mode and
     * fetches 3 frame buffers from the DPI panel at registration. The panel
     * driver macros ship num_fbs = 2, so the fetch fails (err=258) and the v8
     * bridge flushes through an uninitialized draw_fb -> Store access fault on
     * the first refresh. 3 x 800x1280x2 = 6 MB of the 32 MB PSRAM. */
    dpi_config.num_fbs = 3;

    panel_vendor_config_t vendor_config = {
        .mipi_config = {
            .dsi_bus = mipi_dsi_bus,
            .dpi_config = &dpi_config,
        },
    };
    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = PIN_NUM_LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = PANEL_BITS_PER_PIXEL,
        .vendor_config = &vendor_config,
    };

    /* Give the panel power rails time to settle before reset */
    vTaskDelay(pdMS_TO_TICKS(1000));
    ESP_ERROR_CHECK(PANEL_NEW_PANEL(mipi_dbi_io, &panel_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    /* Turn on the backlight once the panel is streaming video */
    adapter_set_lcd_backlight(LCD_BK_LIGHT_ON_LEVEL);

    const esp_lv_adapter_config_t adapter_cfg = ESP_LV_ADAPTER_DEFAULT_CONFIG();
    esp_err_t err = esp_lv_adapter_init(&adapter_cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "LVGL adapter init failed (%d)", err);
        return NULL;
    }

    esp_lv_adapter_display_config_t disp_cfg = ESP_LV_ADAPTER_DISPLAY_MIPI_DEFAULT_CONFIG(
        panel_handle, mipi_dbi_io, PANEL_H_RES, PANEL_V_RES, ESP_LV_ADAPTER_ROTATE_0);

    disp_cfg.profile.buffer_height = LVGL_ADAPTER_BUFFER_HEIGHT;
    lv_display_t *disp = esp_lv_adapter_register_display(&disp_cfg);
    if (disp == NULL) {
        ESP_LOGE(TAG, "Register display failed");
        return NULL;
    }

    /* Bring up the shared CTP so the Phone gestures work out of the box */
    adapter_init_touch(disp);

    err = esp_lv_adapter_start();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "LVGL adapter start failed (%d)", err);
        return NULL;
    }

    return disp;
}
