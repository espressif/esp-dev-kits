/*
 * SPDX-FileCopyrightText: 2020-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "esp_log.h"
#include "i2c_bus.h"
#include "spi_bus.h"
#include "bsp_esp32_s3_usb_otg_ev.h"
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_defs.h"
#include "driver/sdmmc_types.h"
#include "sdmmc_cmd.h"

#ifdef SOC_SDMMC_HOST_SUPPORTED
#include "driver/sdmmc_host.h"
#else
#include "driver/spi_common.h"
#include "driver/sdspi_host.h"
#endif


static const char *TAG = "Board";

#if CONFIG_IDF_TARGET_ESP32S2
#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   16          //Multisampling
static esp_adc_cal_characteristics_t *adc1_chars = NULL;
static const adc_unit_t BOARD_ADC_UNIT = ADC_UNIT_1;
static const adc_bits_width_t BOARD_ADC_WIDTH = ADC_WIDTH_MAX - 1;
static const adc_atten_t BOARD_ADC_ATTEN = ADC_ATTEN_DB_12;
#endif

static bool s_board_is_init = false;
static bool s_board_gpio_is_init = false;
static bool s_board_adc_isinit = false;
static bool s_board_lcd_isinit = false;
static bool s_board_sdcard_isinit = false;

#define BOARD_CHECK(a, str, ret) if(!(a)) { \
        ESP_LOGE(TAG,"%s:%d (%s):%s", __FILE__, __LINE__, __FUNCTION__, str); \
        return (ret); \
    }

/****Private board level API ****/
static i2c_bus_handle_t s_i2c0_bus_handle = NULL;
static i2c_bus_handle_t s_spi2_bus_handle = NULL;
static i2c_bus_handle_t s_spi3_bus_handle = NULL;
static button_handle_t s_btn_ok_hdl = NULL;
static button_handle_t s_btn_up_hdl = NULL;
static button_handle_t s_btn_dw_hdl = NULL;
static button_handle_t s_btn_menu_hdl = NULL;
static board_res_handle_t s_lcd_handle = NULL;
static board_res_handle_t s_sdcard_handle = NULL;
static scr_driver_t s_lcd;
static sdmmc_card_t *s_sdcard;

#if CONFIG_IDF_TARGET_ESP32S2
static void _check_efuse(void)
{
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
        printf("eFuse Two Point: Supported\n");
    } else {
        printf("Cannot retrieve eFuse Two Point calibration values. Default calibration values will be used.\n");
    }
}

static void _print_char_val_type(esp_adc_cal_value_t val_type)
{
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        printf("Characterized using Two Point Value\n");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        printf("Characterized using eFuse Vref\n");
    } else {
        printf("Characterized using Default Vref\n");
    }
}
#endif

static esp_err_t board_i2c_bus_init(void)
{
#if(CONFIG_BOARD_I2C0_INIT)
    i2c_config_t board_i2c_conf = {
        .mode = BOARD_I2C0_MODE,
        .sda_io_num = BOARD_IO_I2C0_SDA,
        .sda_pullup_en = BOARD_I2C0_SDA_PULLUP_EN,
        .scl_io_num = BOARD_IO_I2C0_SCL,
        .scl_pullup_en = BOARD_I2C0_SCL_PULLUP_EN,
        .master.clk_speed = BOARD_I2C0_SPEED,
    };
    i2c_bus_handle_t handle = i2c_bus_create(I2C_NUM_0, &board_i2c_conf);
    BOARD_CHECK(handle != NULL, "i2c_bus creat failed", ESP_FAIL);
    s_i2c0_bus_handle = handle;
#else
    s_i2c0_bus_handle = NULL;
#endif
    return ESP_OK;
}

static esp_err_t board_i2c_bus_deinit(void)
{
    if (s_i2c0_bus_handle != NULL) {
        i2c_bus_delete(&s_i2c0_bus_handle);
        BOARD_CHECK(s_i2c0_bus_handle == NULL, "i2c_bus delete failed", ESP_FAIL);
    }
    return ESP_OK;
}

static esp_err_t board_spi_bus_init(void)
{
#if(CONFIG_BOARD_SPI3_INIT)
    spi_config_t bus3_conf = {
        .miso_io_num = BOARD_IO_SPI3_MISO,
        .mosi_io_num = BOARD_IO_SPI3_MOSI,
        .sclk_io_num = BOARD_IO_SPI3_SCK,
    };
    s_spi3_bus_handle = spi_bus_create(SPI3_HOST, &bus3_conf);
    BOARD_CHECK(s_spi3_bus_handle != NULL, "spi_bus3 creat failed", ESP_FAIL);
#endif
#if(CONFIG_BOARD_SPI2_INIT)
    spi_config_t bus2_conf = {
        .miso_io_num = BOARD_IO_SPI2_MISO,
        .mosi_io_num = BOARD_IO_SPI2_MOSI,
        .sclk_io_num = BOARD_IO_SPI2_SCK,
        .max_transfer_sz = BOARD_LCD_WIDTH * BOARD_LCD_HEIGHT * 2 + 64,
    };
    s_spi2_bus_handle = spi_bus_create(SPI2_HOST, &bus2_conf);
    BOARD_CHECK(s_spi2_bus_handle != NULL, "spi_bus2 creat failed", ESP_FAIL);
#endif
    return ESP_OK;
}

static esp_err_t board_spi_bus_deinit(void)
{
    if (s_spi2_bus_handle != NULL) {
        spi_bus_delete(&s_spi2_bus_handle);
        BOARD_CHECK(s_spi2_bus_handle == NULL, "spi bus2 delete failed", ESP_FAIL);
    }
    if (s_spi3_bus_handle != NULL) {
        spi_bus_delete(&s_spi3_bus_handle);
        BOARD_CHECK(s_spi3_bus_handle == NULL, "spi bus3 delete failed", ESP_FAIL);
    }
    return ESP_OK;
}

static esp_err_t board_gpio_init(void)
{
    if (s_board_gpio_is_init) {
        return ESP_OK;
    }
    esp_err_t ret;
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set
    io_conf.pin_bit_mask = BOARD_IO_PIN_SEL_OUTPUT;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    ret = gpio_config(&io_conf);

    if (ret != ESP_OK) {
        return ret;
    }

    //set as output mode
    io_conf.mode = GPIO_MODE_INPUT;
    //bit mask of the pins that you want to set
    io_conf.pin_bit_mask = BOARD_IO_PIN_SEL_INPUT;
    //configure GPIO with the given settings
    ret = gpio_config(&io_conf);

    if (ret == ESP_OK) {
        s_board_gpio_is_init = true;
    }

    return ret;
}

static esp_err_t board_gpio_deinit(void)
{
    if (!s_board_gpio_is_init) {
        return ESP_OK;
    }
    s_board_gpio_is_init = false;
    return ESP_OK;
}


static esp_err_t board_adc_init(void)
{
#ifdef CONFIG_IDF_TARGET_ESP32S2
    //Configure ADC
    adc1_config_width(BOARD_ADC_WIDTH);
    adc1_config_channel_atten(BOARD_ADC_HOST_VOL_CHAN, BOARD_ADC_ATTEN);

    //Check if Two Point or Vref are burned into eFuse
    _check_efuse();
    //Characterize ADC
    adc1_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(BOARD_ADC_UNIT, BOARD_ADC_ATTEN, BOARD_ADC_WIDTH, DEFAULT_VREF, adc1_chars);
    _print_char_val_type(val_type);

    s_board_adc_isinit = true;
#endif
    return ESP_OK;
}

static esp_err_t board_adc_deinit()
{
    return ESP_OK;//TODO:
}

static esp_err_t board_button_init()
{
    button_config_t cfg = {
        .type = BUTTON_TYPE_GPIO,
        .gpio_button_config = {
            .gpio_num = BOARD_IO_BUTTON_OK,
            .active_level = 0,
        },
    };
    s_btn_ok_hdl = iot_button_create(&cfg);
    BOARD_CHECK(s_btn_ok_hdl != NULL, "button ok create failed", ESP_FAIL);

    cfg.gpio_button_config.gpio_num = BOARD_IO_BUTTON_DW;
    s_btn_dw_hdl = iot_button_create(&cfg);
    BOARD_CHECK(s_btn_dw_hdl != NULL, "button down create failed", ESP_FAIL);

    cfg.gpio_button_config.gpio_num = BOARD_IO_BUTTON_UP;
    s_btn_up_hdl = iot_button_create(&cfg);
    BOARD_CHECK(s_btn_up_hdl != NULL, "button up create failed", ESP_FAIL);

    cfg.gpio_button_config.gpio_num = BOARD_IO_BUTTON_MENU;
    s_btn_menu_hdl = iot_button_create(&cfg);
    BOARD_CHECK(s_btn_menu_hdl != NULL, "button menu create failed", ESP_FAIL);
    return ESP_OK;
}

static esp_err_t board_button_deinit()
{
    return ESP_OK;//TODO:
}

static void lcd_screen_clear(scr_driver_t *lcd, int color)
{
    scr_info_t lcd_info;
    lcd->get_info(&lcd_info);
    uint16_t *buffer = malloc(lcd_info.width * sizeof(uint16_t));

    if (NULL == buffer) {
        for (size_t y = 0; y < lcd_info.height; y++) {
            for (size_t x = 0; x < lcd_info.width; x++) {
                lcd->draw_pixel(x, y, color);
            }
        }
    } else {
        for (size_t i = 0; i < lcd_info.width; i++) {
            buffer[i] = color;
        }

        for (int y = 0; y < lcd_info.height; y++) {
            lcd->draw_bitmap(0, y, lcd_info.width, 1, buffer);
        }

        free(buffer);
    }
}

static esp_err_t board_lcd_init(void)
{
#ifdef CONFIG_BOARD_LCD_INIT
    BOARD_CHECK(s_board_lcd_isinit != true, "lcd has inited", ESP_OK);
    spi_bus_handle_t spi_bus = iot_board_get_handle(BOARD_SPI2_ID);
    BOARD_CHECK(spi_bus != NULL, "spi_bus2 not created", ESP_ERR_INVALID_STATE);

    scr_interface_spi_config_t spi_lcd_cfg = {
        .spi_bus = spi_bus,
        .pin_num_cs = BOARD_LCD_SPI_CS_PIN,
        .pin_num_dc = BOARD_LCD_SPI_DC_PIN,
        .clk_freq = BOARD_LCD_SPI_CLOCK_FREQ,
        .swap_data = true,
    };

    scr_interface_driver_t *iface_drv;
    scr_interface_create(SCREEN_IFACE_SPI, &spi_lcd_cfg, &iface_drv);
    esp_err_t ret = scr_find_driver(BOARD_LCD_TYPE, &s_lcd);
    BOARD_CHECK(ret == ESP_OK, "screen find failed", ESP_ERR_NOT_SUPPORTED);

    scr_controller_config_t lcd_cfg = {
        .interface_drv = iface_drv,
        .pin_num_rst = BOARD_LCD_SPI_RESET_PIN,
        .pin_num_bckl = BOARD_LCD_SPI_BL_PIN,
        .rst_active_level = 0,
        .bckl_active_level = 1,
        .offset_hor = 0,
        .offset_ver = 0,
        .width = BOARD_LCD_WIDTH,
        .height = BOARD_LCD_HEIGHT,
        .rotate = SCR_DIR_LRTB,
    };

    ret = s_lcd.init(&lcd_cfg);
    BOARD_CHECK(ret == ESP_OK, "screen initialize failed", ESP_FAIL);
    lcd_screen_clear(&s_lcd, COLOR_BLACK);
    s_lcd_handle = (board_res_handle_t)&s_lcd;
    s_board_lcd_isinit = true;
#endif
    return ESP_OK;
}

static esp_err_t board_sdcard_init(void)
{
#ifdef CONFIG_BOARD_SD_CARD_INIT
    BOARD_CHECK(s_board_sdcard_isinit != true, "sdcard has inited", ESP_OK);
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = BOARD_SDCARD_FORMAT_IF_MOUNT_FAILED,
        .max_files = BOARD_SDCARD_MAX_OPENED_FILE,
        .allocation_unit_size = BOARD_SDCARD_DISK_BLOCK_SIZE
    };
    esp_err_t ret = ESP_OK;
#ifdef SOC_SDMMC_HOST_SUPPORTED
    ESP_LOGI(TAG, "Using SDIO Interface");
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    //host.max_freq_khz = SDMMC_FREQ_HIGHSPEED;

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    slot_config.clk = BOARD_SDCARD_SDIO_CLK;
    slot_config.cmd = BOARD_SDCARD_SDIO_CMD;
    slot_config.d0 = BOARD_SDCARD_SDIO_DATA0;
    slot_config.d1 = BOARD_SDCARD_SDIO_DATA1;
    slot_config.d2 = BOARD_SDCARD_SDIO_DATA2;
    slot_config.d3 = BOARD_SDCARD_SDIO_DATA3;
    // To use 1-line SD mode, change this to 1:
    slot_config.width = BOARD_SDCARD_SDIO_DATA_WIDTH;
    // Enable internal pullups on enabled pins. The internal pullups
    // are insufficient however, please make sure 10k external pullups are
    // connected on the bus. This is for debug / example purpose only.
    //slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

    ret = esp_vfs_fat_sdmmc_mount(BOARD_SDCARD_BASE_PATH, &host, &slot_config, &mount_config, &s_sdcard);
#else
    ESP_LOGI(TAG, "Using SPI Interface");
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = BOARD_SDCARD_SPI_HOST;
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = BOARD_SDCARD_CD;
    slot_config.host_id = host.slot;
    ret = esp_vfs_fat_sdspi_mount(BOARD_SDCARD_BASE_PATH, &host, &slot_config, &mount_config, &s_sdcard);
#endif

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                     "If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                     "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return ret;
    }

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, s_sdcard);
    s_sdcard_handle = s_sdcard;
    s_board_sdcard_isinit = true;
#endif
    return ESP_OK;
}

esp_err_t board_lcd_draw_image(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t *img)
{
    BOARD_CHECK(s_board_lcd_isinit == true, "lcd not inited", ESP_ERR_INVALID_STATE);
    BOARD_CHECK(NULL != img, "Image pointer invalid", ESP_ERR_INVALID_ARG);
    return s_lcd.draw_bitmap(x, y, width, height, img);
}

#if CONFIG_IDF_TARGET_ESP32S3
static void usb_otg_router_to_internal_phy()
{
    uint32_t *usb_phy_sel_reg = (uint32_t *)(0x60008000 + 0x120);
    *usb_phy_sel_reg |= BIT(19) | BIT(20);
}
#endif

/****General board level API ****/
esp_err_t iot_board_init(void)
{
    BOARD_CHECK(s_board_is_init != true, "board has inited", ESP_OK);

#if CONFIG_IDF_TARGET_ESP32S3
    /* router USB PHY from USB-JTAG-Serial to USB OTG */
    usb_otg_router_to_internal_phy();
#endif

    esp_err_t ret = board_gpio_init();
    BOARD_CHECK(ret == ESP_OK, "gpio init failed", ret);

    ret = board_button_init();
    BOARD_CHECK(ret == ESP_OK, "button init failed", ret);

    ret = board_adc_init();
    BOARD_CHECK(ret == ESP_OK, "ADC init failed", ret);

    iot_board_led_all_set_state(false);

    ret = board_i2c_bus_init();
    BOARD_CHECK(ret == ESP_OK, "i2c init failed", ret);

    ret = board_spi_bus_init();
    BOARD_CHECK(ret == ESP_OK, "spi init failed", ret);

    ret = board_lcd_init();
    BOARD_CHECK(ret == ESP_OK, "lcd init failed", ret);

    ret = board_sdcard_init();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "No sdcard found, or sdcard init failed");
    };
    s_board_is_init = true;
    ESP_LOGI(TAG, "Board Info: %s", iot_board_get_info());
    ESP_LOGI(TAG, "Board Init Done ...");
    return ESP_OK;
}

esp_err_t iot_board_deinit(void)
{
    if (!s_board_is_init) {
        return ESP_OK;
    }

    BOARD_CHECK(s_board_is_init == true, "board not inited", ESP_OK);

    esp_err_t ret = board_gpio_deinit();
    BOARD_CHECK(ret == ESP_OK, "gpio de-init failed", ret);

    ret = board_button_deinit();
    BOARD_CHECK(ret == ESP_OK, "button de-init failed", ret);

    ret = board_adc_deinit();
    BOARD_CHECK(ret == ESP_OK, "ADC de-init failed", ret);

    iot_board_led_all_set_state(false);

    ret = board_i2c_bus_deinit();
    BOARD_CHECK(ret == ESP_OK, "i2c de-init failed", ret);

    ret = board_spi_bus_deinit();
    BOARD_CHECK(ret == ESP_OK, "spi de-init failed", ret);

    s_board_is_init = false;
    ESP_LOGI(TAG, " Board Deinit Done ...");
    return ESP_OK;
}

bool iot_board_is_init(void)
{
    return s_board_is_init;
}

board_res_handle_t iot_board_get_handle(board_res_id_t id)
{
    board_res_handle_t handle;
    switch (id) {
    case BOARD_I2C0_ID:
        handle = (board_res_handle_t)s_i2c0_bus_handle;
        break;
    case BOARD_SPI2_ID:
        handle = (board_res_handle_t)s_spi2_bus_handle;
        break;
    case BOARD_SPI3_ID:
        handle = (board_res_handle_t)s_spi3_bus_handle;
        break;
    case BOARD_LCD_ID:
        handle = (board_res_handle_t)s_lcd_handle;
        break;
    case BOARD_SDCARD_ID:
        handle = (board_res_handle_t)s_sdcard_handle;
        break;
    case BOARD_BTN_OK_ID:
        handle = (board_res_handle_t)s_btn_ok_hdl;
        break;
    case BOARD_BTN_UP_ID:
        handle = (board_res_handle_t)s_btn_up_hdl;
        break;
    case BOARD_BTN_DW_ID:
        handle = (board_res_handle_t)s_btn_dw_hdl;
        break;
    case BOARD_BTN_MN_ID:
        handle = (board_res_handle_t)s_btn_menu_hdl;
        break;
    default:
        handle = NULL;
        break;
    }
    return handle;
}

char *iot_board_get_info()
{
    static char *info = BOARD_NAME;
    return info;
}

/****Extended board level API ****/
esp_err_t iot_board_usb_device_set_power(bool on_off, bool from_battery)
{
    BOARD_CHECK(s_board_is_init == true, "board not inited", ESP_FAIL);

    if (from_battery) {
        gpio_set_level(BOARD_IO_HOST_BOOST_EN, on_off); //BOOST_EN
        gpio_set_level(BOARD_IO_DEV_VBUS_EN, !on_off);//DEV_VBUS_EN
        gpio_set_level(BOARD_IO_IDEV_LIMIT_EN, on_off);//DEV_VBUS_EN
    } else {
        gpio_set_level(BOARD_IO_HOST_BOOST_EN, !on_off); //BOOST_EN
        gpio_set_level(BOARD_IO_DEV_VBUS_EN, on_off);//DEV_VBUS_EN
        gpio_set_level(BOARD_IO_IDEV_LIMIT_EN, on_off);//DEV_VBUS_EN
    }
    return ESP_OK;
}

bool iot_board_usb_device_get_power(void)
{
    if (!s_board_gpio_is_init) {
        return 0;
    }
    return gpio_get_level(BOARD_IO_IDEV_LIMIT_EN);
}

float iot_board_get_host_voltage(void)
{
    BOARD_CHECK(s_board_adc_isinit == true, "ADC not inited", 0.0);
    uint32_t adc_reading = 0;
#ifdef CONFIG_IDF_TARGET_ESP32S2
    //Multisampling
    for (int i = 0; i < NO_OF_SAMPLES; i++) {
        adc_reading += adc1_get_raw((adc1_channel_t)BOARD_ADC_HOST_VOL_CHAN);
    }
    adc_reading /= NO_OF_SAMPLES;
    //Convert adc_reading to voltage in mV
    adc_reading = esp_adc_cal_raw_to_voltage(adc_reading, adc1_chars);
#endif
    return adc_reading * 370.0 / 100.0 / 1000.0;
}

float iot_board_get_battery_voltage(void)
{
    BOARD_CHECK(s_board_adc_isinit == true, "ADC not inited", 0.0);
    uint32_t adc_reading = 0;
#ifdef CONFIG_IDF_TARGET_ESP32S2
    //Multisampling
    for (int i = 0; i < NO_OF_SAMPLES; i++) {
        adc_reading += adc1_get_raw((adc1_channel_t)BOARD_ADC_HOST_VOL_CHAN);
    }
    adc_reading /= NO_OF_SAMPLES;
    //Convert adc_reading to voltage in mV
    adc_reading = esp_adc_cal_raw_to_voltage(adc_reading, adc1_chars);
#endif
    return adc_reading * 200.0 / 100.0 / 1000.0;
}

esp_err_t iot_board_button_register_cb(board_res_handle_t btn_handle, button_event_t event, button_cb_t cb)
{
    BOARD_CHECK(btn_handle != NULL, "invalid button handle", ESP_ERR_INVALID_ARG);
    return iot_button_register_cb((board_res_handle_t)btn_handle, event, cb, NULL);
}

esp_err_t iot_board_button_unregister_cb(board_res_handle_t btn_handle, button_event_t event)
{
    BOARD_CHECK(btn_handle != NULL, "invalid button handle", ESP_ERR_INVALID_ARG);
    return iot_button_unregister_cb((board_res_handle_t)btn_handle, event);
}

esp_err_t iot_board_usb_set_mode(usb_mode_t mode)
{
    BOARD_CHECK(s_board_is_init == true, "board not inited", ESP_FAIL);
    switch (mode) {
    case USB_DEVICE_MODE:
        gpio_set_level(BOARD_IO_USB_SEL, false); //USB_SEL
        break;
    case USB_HOST_MODE:
        gpio_set_level(BOARD_IO_USB_SEL, true); //USB_SEL
        break;
    default:
        assert(0);
        break;
    }
    return ESP_OK;
}

usb_mode_t iot_board_usb_get_mode(void)
{
    if (!s_board_gpio_is_init) {
        return USB_DEVICE_MODE;
    }

    int value = gpio_get_level(BOARD_IO_USB_SEL);
    if (value) {
        return USB_HOST_MODE;
    }
    return USB_DEVICE_MODE;
}

static int s_led_io[BOARD_LED_NUM] = {BOARD_IO_LED_1, BOARD_IO_LED_2};
static int s_led_polarity[BOARD_LED_NUM] = {BOARD_LED_POLARITY_1, BOARD_LED_POLARITY_2};
static bool s_led_state[BOARD_LED_NUM] = {0};

esp_err_t iot_board_led_set_state(int gpio_num, bool if_on)
{
    int i = 0;

    for (i = 0; i < BOARD_LED_NUM; i++) {
        if (s_led_io[i] == gpio_num) {
            break;
        }
    }

    if (i >= BOARD_LED_NUM) {
        ESP_LOGE(TAG, "GPIO %d is not a valid LED io = %d", gpio_num, i);
        return ESP_FAIL;
    }

    if (!s_led_polarity[i]) { /*check led polarity*/
        if_on = !if_on;
    }

    gpio_set_level(gpio_num, if_on);
    return ESP_OK;
}

esp_err_t iot_board_led_all_set_state(bool if_on)
{
    for (size_t i = 0; i < BOARD_LED_NUM; i++) {
        iot_board_led_set_state(s_led_io[i], if_on);
    }
    return ESP_OK;
}

esp_err_t iot_board_led_toggle_state(int gpio_num)
{
    int i = 0;

    for (i = 0; i < BOARD_LED_NUM; i++) {
        if (s_led_io[i] == gpio_num) {
            break;
        }
    }

    if (i >= BOARD_LED_NUM) {
        ESP_LOGE(TAG, "GPIO %d is not a valid LED io", gpio_num);
        return ESP_FAIL;
    }

    s_led_state[i] = !s_led_state[i];
    iot_board_led_set_state(gpio_num, s_led_state[i]);
    return ESP_OK;
}
