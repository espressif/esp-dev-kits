/*
 * SPDX-FileCopyrightText: 2015-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include "esp_log.h"
#include "xpt2046.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "touch_calibration.h"

static const char *TAG = "XPT2046";

#define gpio_pad_select_gpio(g) esp_rom_gpio_pad_select_gpio(g)
#define TOUCH_CHECK(a, str, ret)  if(!(a)) {                                      \
        ESP_LOGE(TAG,"%s:%d (%s):%s", __FILE__, __LINE__, __FUNCTION__, str);   \
        return (ret);                                                                   \
    }

#define XPT2046_TOUCH_CMD_X   0xD0
#define XPT2046_TOUCH_CMD_Y   0x90
#define XPT2046_TOUCH_CMD_Z1  0b10110000
#define XPT2046_TOUCH_CMD_Z2  0b11000000
#define XPT2046_SMP_SIZE  8

#define TOUCH_SAMPLE_MAX 4000
#define TOUCH_SAMPLE_MIN 100

#define XPT2046_THRESHOLD_Z  150

typedef struct {
    uint16_t x;
    uint16_t y;
} position_t;

typedef struct {
    spi_device_handle_t spi_dev;
    int io_irq;
    touch_panel_dir_t direction;
    uint16_t width;
    uint16_t height;
} xpt2046_dev_t;

static xpt2046_dev_t g_dev = {0};

static void spi_transfer_bytes(spi_device_handle_t spi, const uint8_t *data_out, uint8_t *data_in, uint32_t data_len)
{
    spi_transaction_t trans = {
        .length = data_len * 8,
        .tx_buffer = NULL,
        .rx_buffer = NULL,
    };
    if (data_out) {
        trans.tx_buffer = data_out;
    }
    if (data_in) {
        trans.rx_buffer = data_in;
    }
    spi_device_acquire_bus(spi, portMAX_DELAY);
    ESP_ERROR_CHECK(spi_device_polling_transmit(spi, &trans));
    spi_device_release_bus(spi);
}

static uint16_t xpt2046_readdata(spi_device_handle_t spi, const uint8_t command)
{
    uint8_t rev[3];
    uint8_t send[3];
    send[0] = command;
    send[1] = 0xff;
    send[2] = 0xff;
    spi_transfer_bytes(spi, send, rev, 3);
    return (rev[1] << 8 | rev[2]) >> 3;
}

static esp_err_t xpt2046_get_sample(uint8_t command, uint16_t *out_data)
{
    uint16_t data = xpt2046_readdata(g_dev.spi_dev, command);

    if (0xffff != data) {
        *out_data = data;
        return ESP_OK;
    }

    return ESP_FAIL;
}

touch_panel_config_t config = {
    .interface = {
        .clk_freq = 10 * 1000 * 1000,
        .host_id = SPI2_HOST,
        .pin_num_cs = GPIO_NUM_46,
    },
    .pin_num_int = -1,
    .direction = TOUCH_DIR_LRTB,
    .width = 240,
    .height = 320,
};

esp_err_t xpt2046_init(void)
{
    //Initialize non-SPI GPIOs
    if (config.pin_num_int >= 0) {
        gpio_pad_select_gpio(config.pin_num_int);
        gpio_set_direction(config.pin_num_int, GPIO_MODE_INPUT);
    }

    spi_device_interface_config_t dev_cfg = {
        .clock_speed_hz = config.interface.clk_freq,
        .mode = 0,
        .spics_io_num = config.interface.pin_num_cs,
        .queue_size = 3,
        .cs_ena_posttrans = 3,
    };
    ESP_ERROR_CHECK(spi_bus_add_device(config.interface.host_id, &dev_cfg, &(g_dev.spi_dev)));

    xpt2046_set_direction(config.direction);
    g_dev.io_irq = config.pin_num_int;
    g_dev.width = config.width;
    g_dev.height = config.height;

    ESP_LOGI(TAG, "Touch panel size width: %d, height: %d", g_dev.width, g_dev.height);
    ESP_LOGI(TAG, "Initial successful | GPIO INT:%d | GPIO CS:%d | dir:%d", config.pin_num_int, config.interface.pin_num_cs, config.direction);

    return ESP_OK;
}

esp_err_t xpt2046_deinit(void)
{
    spi_bus_remove_device(g_dev.spi_dev);
    memset(&g_dev, 0, sizeof(xpt2046_dev_t));
    return ESP_OK;
}

int xpt2046_is_pressed(void)
{
    /**
     * @note There are two ways to determine weather the touch panel is pressed
     * 1. Read the IRQ line of touch controller
     * 2. Read value of z axis
     * Only the second method is used here, so the IRQ line is not used.
     */
    uint16_t z;
    esp_err_t ret = xpt2046_get_sample(XPT2046_TOUCH_CMD_Z1, &z);
    TOUCH_CHECK(ret == ESP_OK, "Z sample failed", 0);
    if (z > XPT2046_THRESHOLD_Z) { /** If z more than threshold, it is considered as pressed */
        return 1;
    }

    return 0;
}

esp_err_t xpt2046_set_direction(touch_panel_dir_t dir)
{
    if (TOUCH_DIR_MAX < dir) {
        dir >>= 5;
    }
    g_dev.direction = dir;
    return ESP_OK;
}

esp_err_t xpt2046_get_rawdata(uint16_t *x, uint16_t *y)
{
    position_t samples[XPT2046_SMP_SIZE];
    esp_err_t ret;
    uint32_t aveX = 0;
    uint32_t aveY = 0;

    for (int i = 0; i < XPT2046_SMP_SIZE; i++) {
        ret = xpt2046_get_sample(XPT2046_TOUCH_CMD_X, &(samples[i].x));
        TOUCH_CHECK(ret == ESP_OK, "X sample failed", ESP_FAIL);
        ret = xpt2046_get_sample(XPT2046_TOUCH_CMD_Y, &(samples[i].y));
        TOUCH_CHECK(ret == ESP_OK, "Y sample failed", ESP_FAIL);

        aveX += samples[i].x;
        aveY += samples[i].y;
    }

    aveX /= XPT2046_SMP_SIZE;
    aveY /= XPT2046_SMP_SIZE;

    *x = aveX;
    *y = aveY;

    return ESP_OK;
}

static void xpt2046_apply_rotate(uint16_t *x, uint16_t *y)
{
    uint16_t _x = *x;
    uint16_t _y = *y;

    switch (g_dev.direction) {
    case TOUCH_DIR_LRTB:
        *x = _x;
        *y = _y;
        break;
    case TOUCH_DIR_LRBT:
        *x = _x;
        *y = g_dev.height - _y;
        break;
    case TOUCH_DIR_RLTB:
        *x = g_dev.width - _x;
        *y = _y;
        break;
    case TOUCH_DIR_RLBT:
        *x = g_dev.width - _x;
        *y = g_dev.height - _y;
        break;
    case TOUCH_DIR_TBLR:
        *x = _y;
        *y = _x;
        break;
    case TOUCH_DIR_BTLR:
        *x = _y;
        *y = g_dev.width - _x;
        break;
    case TOUCH_DIR_TBRL:
        *x = g_dev.height - _y;
        *y = _x;
        break;
    case TOUCH_DIR_BTRL:
        *x = g_dev.height - _y;
        *y = g_dev.width - _x;
        break;

    default:
        break;
    }
}

esp_err_t xpt2046_sample(touch_panel_points_t *info)
{
    TOUCH_CHECK(NULL != info, "Pointer invalid", ESP_FAIL);
    TOUCH_CHECK(NULL != g_dev.spi_dev, "Uninitialized", ESP_ERR_INVALID_STATE);

    esp_err_t ret;
    uint16_t x, y;
    info->curx[0] = 0;
    info->cury[0] = 0;
    info->event = TOUCH_EVT_RELEASE;
    info->point_num = 0;

    int state = xpt2046_is_pressed();
    if (0 == state) {
        return ESP_OK;
    }

    ret = xpt2046_get_rawdata(&x, &y);
    TOUCH_CHECK(ret == ESP_OK, "Get raw data failed", ESP_FAIL);

    /**< If the data is not in the normal range, it is considered that it is not pressed */
    if ((x < TOUCH_SAMPLE_MIN) || (x > TOUCH_SAMPLE_MAX) ||
            (y < TOUCH_SAMPLE_MIN) || (y > TOUCH_SAMPLE_MAX)) {
        return ESP_OK;
    }

    int32_t _x = x;
    int32_t _y = y;
    ret = touch_calibration_transform(&_x, &_y);
    if (ESP_OK != ret) {
        ESP_LOGE(TAG, "Transform raw data failed. Maybe the xxx.calibration_run() function has been not call");
        _x = _y = 0;
        return ESP_FAIL;
    }

    x = _x;
    y = _y;

    xpt2046_apply_rotate(&x, &y);

    info->curx[0] = x;
    info->cury[0] = y;
    info->event = state ? TOUCH_EVT_PRESS : TOUCH_EVT_RELEASE;
    info->point_num = 1;

    return ESP_OK;
}

esp_err_t xpt2046_calibration_run(bool recalibrate)
{
    return touch_calibration_run(xpt2046_is_pressed, xpt2046_get_rawdata, recalibrate);
}
