// Copyright 2019 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_heap_caps.h"
#include "esp32s2/rom/lldesc.h"
#include "soc/system_reg.h"
#include "esp_log.h"
#include "lcd.h"

static const char *TAG = "lcd";

#define LCD_DMA_MAX_SIZE     (4095)

typedef struct {
    uint32_t buffer_size;
    uint32_t half_buffer_size;
    uint32_t node_cnt;
    uint32_t half_node_cnt;
    uint32_t dma_size;
    uint8_t horizontal;
    uint8_t dc_state;
    uint8_t pin_dc;
    uint8_t pin_cs;
    uint8_t pin_rst;
    uint8_t pin_bk;
    lldesc_t *dma;
    uint8_t *buffer;
    bool swap_data;
    QueueHandle_t event_queue;
} lcd_obj_t;

static lcd_obj_t *lcd_obj = NULL;

void static lcd_set_rst(uint8_t state)
{
    gpio_set_level(lcd_obj->pin_rst, state);
}

void static lcd_set_dc(uint8_t state)
{
    gpio_set_level(lcd_obj->pin_dc, state);
}

void static lcd_set_cs(uint8_t state)
{
    gpio_set_level(lcd_obj->pin_cs, state);
}

void static lcd_set_blk(uint8_t state)
{
    gpio_set_level(lcd_obj->pin_bk, state);
}

static void IRAM_ATTR lcd_isr(void *arg)
{
    BaseType_t HPTaskAwoken = pdFALSE;
    typeof(GPSPI3.dma_int_st) int_st = GPSPI3.dma_int_st;
    GPSPI3.dma_int_clr.val = int_st.val;

    if (int_st.out_eof) {
        xQueueSendFromISR(lcd_obj->event_queue, (void *)&int_st.val, &HPTaskAwoken);
    }

    if (HPTaskAwoken == pdTRUE) {
        portYIELD_FROM_ISR();
    }
}

static void spi_write_data(uint8_t *data, size_t len)
{
    int event  = 0;
    int x = 0, cnt = 0, size = 0;
    int end_pos = 0;
    lcd_set_dc(lcd_obj->dc_state);

    /*!< Generate a data DMA linked list */
    for (x = 0; x < lcd_obj->node_cnt; x++) {
        lcd_obj->dma[x].size = lcd_obj->dma_size;
        lcd_obj->dma[x].length = lcd_obj->dma_size;
        lcd_obj->dma[x].buf = (lcd_obj->buffer + lcd_obj->dma_size * x);
        lcd_obj->dma[x].eof = !((x + 1) % lcd_obj->half_node_cnt);
        lcd_obj->dma[x].empty = (uint32_t)&lcd_obj->dma[(x + 1) % lcd_obj->node_cnt];
    }

    lcd_obj->dma[lcd_obj->half_node_cnt - 1].empty = 0;
    lcd_obj->dma[lcd_obj->node_cnt - 1].empty = 0;
    cnt = len / lcd_obj->half_buffer_size;
    /*!< Start the signal */
    xQueueSend(lcd_obj->event_queue, &event, 0);

    /*!< Processing a complete piece of data, ping-pong operation */
    for (x = 0; x < cnt; x++) {
        uint8_t *out = (uint8_t *)lcd_obj->dma[(x % 2) * lcd_obj->half_node_cnt].buf;
        if (lcd_obj->swap_data) {
            for (size_t i = 0; i < lcd_obj->half_buffer_size; i += 2) {
                out[i]   = data[i+1];
                out[i+1] = data[i];
            }
        } else {
            memcpy(out, data, lcd_obj->half_buffer_size);
        }
        
        data += lcd_obj->half_buffer_size;
        xQueueReceive(lcd_obj->event_queue, (void *)&event, portMAX_DELAY);
        GPSPI3.mosi_dlen.usr_mosi_bit_len = lcd_obj->half_buffer_size * 8 - 1;
        GPSPI3.dma_out_link.addr = ((uint32_t)&lcd_obj->dma[(x % 2) * lcd_obj->half_node_cnt]) & 0xfffff;
        GPSPI3.dma_out_link.start = 1;
        ets_delay_us(1);
        GPSPI3.cmd.usr = 1;
    }

    cnt = len % lcd_obj->half_buffer_size;

    /*!< Processing remaining incomplete segment data */
    if (cnt) {
        uint8_t *out = (uint8_t *)lcd_obj->dma[(x % 2) * lcd_obj->half_node_cnt].buf;
        if (lcd_obj->swap_data && cnt > 1) {
            for (size_t i = 0; i < cnt; i += 2) {
                out[i]   = data[i+1];
                out[i+1] = data[i];
            }
        } else {
            memcpy(out, data, cnt);
        }

        /*!< Handle the case where the data length is an integer multiple of lcd_obj->dma_size */
        if (cnt % lcd_obj->dma_size) {
            end_pos = (x % 2) * lcd_obj->half_node_cnt + cnt / lcd_obj->dma_size;
            size = cnt % lcd_obj->dma_size;
        } else {
            end_pos = (x % 2) * lcd_obj->half_node_cnt + cnt / lcd_obj->dma_size - 1;
            size = lcd_obj->dma_size;
        }

        /*!< Handle the tail node to make it a DMA tail */
        lcd_obj->dma[end_pos].size = size;
        lcd_obj->dma[end_pos].length = size;
        lcd_obj->dma[end_pos].eof = 1;
        lcd_obj->dma[end_pos].empty = 0;
        xQueueReceive(lcd_obj->event_queue, (void *)&event, portMAX_DELAY);
        GPSPI3.mosi_dlen.usr_mosi_bit_len = cnt * 8 - 1;
        GPSPI3.dma_out_link.addr = ((uint32_t)&lcd_obj->dma[(x % 2) * lcd_obj->half_node_cnt]) & 0xfffff;
        GPSPI3.dma_out_link.start = 1;
        ets_delay_us(1);
        GPSPI3.cmd.usr = 1;
    }

    xQueueReceive(lcd_obj->event_queue, (void *)&event, portMAX_DELAY);
}

static void lcd_delay_ms(uint32_t time)
{
    vTaskDelay(time / portTICK_RATE_MS);
}

static void lcd_write_cmd(uint8_t data)
{
    lcd_obj->dc_state = 0;
    spi_write_data(&data, 1);
}

static void lcd_write_byte(uint8_t data)
{
    lcd_obj->dc_state = 1;
    spi_write_data(&data, 1);
}

void lcd_write_data(uint8_t *data, size_t len)
{
    if (len <= 0) {
        return;
    }

    lcd_obj->dc_state = 1;
    spi_write_data(data, len);
}

void lcd_rst()
{
    lcd_set_rst(0);
    lcd_delay_ms(100);
    lcd_set_rst(1);
    lcd_delay_ms(100);
}

#ifdef CONFIG_LCD_ILI9341
static void lcd_ili9341_config(lcd_config_t *config)
{
    /* Power contorl B, power control = 0, DC_ENA = 1 */
    lcd_set_cs(0);
    lcd_write_cmd(0xCF);
    lcd_write_byte(0x00);
    lcd_write_byte(0x83);
    lcd_write_byte(0X30);

    /* Power on sequence control,
       * cp1 keeps 1 frame, 1st frame enable
       * vcl = 0, ddvdh=3, vgh=1, vgl=2
       * DDVDH_ENH=1
       */
    lcd_write_cmd(0xED);
    lcd_write_byte(0x64);
    lcd_write_byte(0x03);
    lcd_write_byte(0X12);
    lcd_write_byte(0X81);

    /* Driver timing control A,
    * non-overlap=default +1
    * EQ=default - 1, CR=default
    * pre-charge=default - 1
    */
    lcd_write_cmd(0xE8);
    lcd_write_byte(0x85);
    lcd_write_byte(0x01);
    lcd_write_byte(0x79);

    /* Power control A, Vcore=1.6V, DDVDH=5.6V */
    lcd_write_cmd(0xCB);
    lcd_write_byte(0x39);
    lcd_write_byte(0x2C);
    lcd_write_byte(0x00);
    lcd_write_byte(0x34);
    lcd_write_byte(0x02);

    /* Pump ratio control, DDVDH=2xVCl */
    lcd_write_cmd(0xF7);
    lcd_write_byte(0x20);

    /* Driver timing control, all=0 unit */
    lcd_write_cmd(0xEA);
    lcd_write_byte(0x00);
    lcd_write_byte(0x00);

    /* Power control 1, GVDD=4.75V */
    lcd_write_cmd(0xC0);
    lcd_write_byte(0x26);

    /* Power control 2, DDVDH=VCl*2, VGH=VCl*7, VGL=-VCl*3 */
    lcd_write_cmd(0xC1);
    lcd_write_byte(0x11);

    /* VCOM control 1, VCOMH=4.025V, VCOML=-0.950V */
    lcd_write_cmd(0xC5);
    lcd_write_byte(0x35);
    lcd_write_byte(0x3E);

    /* VCOM control 2, VCOMH=VMH-2, VCOML=VML-2 */
    lcd_write_cmd(0xC7);
    lcd_write_byte(0xBE);

    /* Memory access contorl, MX=MY=0, MV=1, ML=0, BGR=1, MH=0 */
    lcd_write_cmd(0x36);
    lcd_write_byte(0b11000000);

    /* Pixel format, 16bits/pixel for RGB/MCU interface */
    lcd_write_cmd(0x3A);
    lcd_write_byte(0x55);

    /* Frame rate control, f=fosc, 70Hz fps */
    lcd_write_cmd(0xB1);
    lcd_write_byte(0x00);
    lcd_write_byte(0x1B);

    /* Enable 3G, disabled */
    lcd_write_cmd(0xF2);
    lcd_write_byte(0x08);

    /* Gamma set, curve 1 */
    lcd_write_cmd(0x26);
    lcd_write_byte(0x01);

    /* Positive gamma correction */
    lcd_write_cmd(0xE0);
    lcd_write_byte(0x1F);
    lcd_write_byte(0x1A);
    lcd_write_byte(0x18);
    lcd_write_byte(0x0A);
    lcd_write_byte(0x0F);

    lcd_write_byte(0x06);
    lcd_write_byte(0x45);
    lcd_write_byte(0X87);
    lcd_write_byte(0x32);
    lcd_write_byte(0x0A);

    lcd_write_byte(0x07);
    lcd_write_byte(0x02);
    lcd_write_byte(0x07);
    lcd_write_byte(0x05);
    lcd_write_byte(0x00);

    /* Negative gamma correction */
    lcd_write_cmd(0XE1);
    lcd_write_byte(0x00);
    lcd_write_byte(0x25);
    lcd_write_byte(0x27);
    lcd_write_byte(0x05);
    lcd_write_byte(0x10);

    lcd_write_byte(0x09);
    lcd_write_byte(0x3A);
    lcd_write_byte(0X78);
    lcd_write_byte(0x4D);
    lcd_write_byte(0x05);

    lcd_write_byte(0x18);
    lcd_write_byte(0x0D);
    lcd_write_byte(0x38);
    lcd_write_byte(0x3A);
    lcd_write_byte(0x1F);

    /* Column address set, SC=0, EC=0xEF */
    lcd_write_cmd(0x2A);
    lcd_write_byte(0x00);
    lcd_write_byte(0x00);
    lcd_write_byte(0x00);
    lcd_write_byte(0xEF);

    /* Page address set, SP=0, EP=0x013F */
    lcd_write_cmd(0x2B);
    lcd_write_byte(0x00);
    lcd_write_byte(0x00);
    lcd_write_byte(0x01);
    lcd_write_byte(0x3F);

    /* Memory write */
    lcd_write_cmd(0x2C);
    lcd_write_byte(0x00);

    /* Entry mode set, Low vol detect disabled, normal display */
    lcd_write_cmd(0xB7);
    lcd_write_byte(0x07);

    /* Display function control */
    lcd_write_cmd(0xB6);
    lcd_write_byte(0x0A);
    lcd_write_byte(0x82);
    lcd_write_byte(0x27);
    lcd_write_byte(0x00);

    // lcd_write_cmd(0x20); /*!< INVON (21h): Display Inversion On */

    lcd_write_cmd(0x11); /*!< SLPOUT (11h): Sleep Out  */

    vTaskDelay(100 / portTICK_RATE_MS);
    lcd_write_cmd(0x29); /*!< DISPON (29h): Display On */
    vTaskDelay(100 / portTICK_RATE_MS);

}
#endif

#ifdef CONFIG_LCD_ST7789
static void lcd_st7789_config(lcd_config_t *config)
{
    lcd_set_cs(0);

    lcd_write_cmd(0x36); /*!< MADCTL (36h): Memory Data Access Control */

    switch (config->horizontal) {
        case 0: {
            lcd_write_byte(0x00);
        }
        break;

        case 1: {
            lcd_write_byte(0xC0);
        }
        break;

        case 2: {
            lcd_write_byte(0x70);
        }
        break;

        case 3: {
            lcd_write_byte(0xA0);
        }
        break;

        default: {
            lcd_write_byte(0x00);
        }
        break;
    }

    lcd_write_cmd(0x3A);  /*!< COLMOD (3Ah): Interface Pixel Format  */
    lcd_write_byte(0x05);

    lcd_write_cmd(0xB2); /*!< PORCTRL (B2h): Porch Setting  */
    lcd_write_byte(0x0C);
    lcd_write_byte(0x0C);
    lcd_write_byte(0x00);
    lcd_write_byte(0x33);
    lcd_write_byte(0x33);

    lcd_write_cmd(0xB7); /*!< GCTRL (B7h): Gate Control  */
    lcd_write_byte(0x35);

    lcd_write_cmd(0xBB); /*!< VCOMS (BBh): VCOM Setting  */
    lcd_write_byte(0x19);

    lcd_write_cmd(0xC0); /*!< LCMCTRL (C0h): LCM Control  */
    lcd_write_byte(0x2C);

    lcd_write_cmd(0xC2); /*!< VDVVRHEN (C2h): VDV and VRH Command Enable */
    lcd_write_byte(0x01);

    lcd_write_cmd(0xC3); /*!< VRHS (C3h): VRH Set */
    lcd_write_byte(0x12);

    lcd_write_cmd(0xC4); /*!< VDVS (C4h): VDV Set  */
    lcd_write_byte(0x20);

    lcd_write_cmd(0xC6); /*!< FRCTRL2 (C6h): Frame Rate Control in Normal Mode  */
    lcd_write_byte(0x0F);

    lcd_write_cmd(0xD0); /*!< PWCTRL1 (D0h): Power Control 1  */
    lcd_write_byte(0xA4);
    lcd_write_byte(0xA1);

    lcd_write_cmd(0xE0); /*!< PVGAMCTRL (E0h): Positive Voltage Gamma Control */
    lcd_write_byte(0xD0);
    lcd_write_byte(0x04);
    lcd_write_byte(0x0D);
    lcd_write_byte(0x11);
    lcd_write_byte(0x13);
    lcd_write_byte(0x2B);
    lcd_write_byte(0x3F);
    lcd_write_byte(0x54);
    lcd_write_byte(0x4C);
    lcd_write_byte(0x18);
    lcd_write_byte(0x0D);
    lcd_write_byte(0x0B);
    lcd_write_byte(0x1F);
    lcd_write_byte(0x23);

    lcd_write_cmd(0xE1); /*!< NVGAMCTRL (E1h): Negative Voltage Gamma Control */
    lcd_write_byte(0xD0);
    lcd_write_byte(0x04);
    lcd_write_byte(0x0C);
    lcd_write_byte(0x11);
    lcd_write_byte(0x13);
    lcd_write_byte(0x2C);
    lcd_write_byte(0x3F);
    lcd_write_byte(0x44);
    lcd_write_byte(0x51);
    lcd_write_byte(0x2F);
    lcd_write_byte(0x1F);
    lcd_write_byte(0x1F);
    lcd_write_byte(0x20);
    lcd_write_byte(0x23);

    lcd_write_cmd(0x20); /*!< INVON (21h): Display Inversion On */

    lcd_write_cmd(0x11); /*!< SLPOUT (11h): Sleep Out  */

    lcd_write_cmd(0x29); /*!< DISPON (29h): Display On */
}
#endif

static void lcd_config(lcd_config_t *config)
{

    REG_CLR_BIT(DPORT_PERIP_CLK_EN0_REG, DPORT_SPI3_CLK_EN);
    REG_SET_BIT(DPORT_PERIP_CLK_EN0_REG, DPORT_SPI3_CLK_EN);
    REG_SET_BIT(DPORT_PERIP_RST_EN0_REG, DPORT_SPI3_RST);
    REG_CLR_BIT(DPORT_PERIP_RST_EN0_REG, DPORT_SPI3_RST);
    REG_CLR_BIT(DPORT_PERIP_CLK_EN0_REG, DPORT_SPI3_DMA_CLK_EN);
    REG_SET_BIT(DPORT_PERIP_CLK_EN0_REG, DPORT_SPI3_DMA_CLK_EN);
    REG_SET_BIT(DPORT_PERIP_RST_EN0_REG, DPORT_SPI3_DMA_RST);
    REG_CLR_BIT(DPORT_PERIP_RST_EN0_REG, DPORT_SPI3_DMA_RST);

    int div = 2;

    if (config->clk_fre == 80000000) {
        GPSPI3.clock.clk_equ_sysclk = 1;
    } else {
        GPSPI3.clock.clk_equ_sysclk = 0;
        div = 80000000 / config->clk_fre;
    }

    GPSPI3.ctrl1.clk_mode = 0;
    GPSPI3.clock.clkdiv_pre = 1 - 1;
    GPSPI3.clock.clkcnt_n = div - 1;
    GPSPI3.clock.clkcnt_l = div - 1;
    GPSPI3.clock.clkcnt_h = ((div >> 1) - 1);

    GPSPI3.misc.ck_dis = 0;

    GPSPI3.user1.val = 0;
    GPSPI3.slave.val = 0;
    GPSPI3.misc.ck_idle_edge = 0;
    GPSPI3.user.ck_out_edge = 0;
    GPSPI3.ctrl.wr_bit_order = 0;
    GPSPI3.ctrl.rd_bit_order = 0;
    GPSPI3.user.val = 0;
    GPSPI3.user.cs_setup = 1;
    GPSPI3.user.cs_hold = 1;
    GPSPI3.user.usr_mosi = 1;
    GPSPI3.user.usr_mosi_highpart = 0;

    GPSPI3.dma_conf.val = 0;
    GPSPI3.dma_conf.out_rst = 1;
    GPSPI3.dma_conf.out_rst = 0;
    GPSPI3.dma_conf.ahbm_fifo_rst = 1;
    GPSPI3.dma_conf.ahbm_fifo_rst = 0;
    GPSPI3.dma_conf.ahbm_rst = 1;
    GPSPI3.dma_conf.ahbm_rst = 0;
    GPSPI3.dma_out_link.dma_tx_ena = 1;
    GPSPI3.dma_conf.out_eof_mode = 1;
    GPSPI3.cmd.usr = 0;

    GPSPI3.dma_int_clr.val = ~0;
    GPSPI3.dma_int_ena.val = 0;
    GPSPI3.dma_int_ena.out_eof = 1;

    intr_handle_t intr_handle = NULL;
    esp_intr_alloc(ETS_SPI3_DMA_INTR_SOURCE, 0, lcd_isr, NULL, &intr_handle);
}

static void lcd_set_pin(lcd_config_t *config)
{
    PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[config->pin_clk], PIN_FUNC_GPIO);
    gpio_set_direction(config->pin_clk, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(config->pin_clk, GPIO_FLOATING);
    gpio_matrix_out(config->pin_clk, SPI3_CLK_OUT_MUX_IDX, 0, 0);

    PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[config->pin_mosi], PIN_FUNC_GPIO);
    gpio_set_direction(config->pin_mosi, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(config->pin_mosi, GPIO_FLOATING);
    gpio_matrix_out(config->pin_mosi, SPI3_D_OUT_IDX, 0, 0);

    /*!< Initialize non-SPI GPIOs */
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << config->pin_dc) | (1ULL << config->pin_cs);

    if (config->pin_rst <= 46) {
        io_conf.pin_bit_mask |= (1ULL << config->pin_rst);
    }

    if (config->pin_bk <= 46) {
        io_conf.pin_bit_mask |= (1ULL << config->pin_bk);
    }

    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
}

void lcd_dma_config(lcd_config_t *config)
{
    int cnt = 0;

    if (config->max_buffer_size >= LCD_DMA_MAX_SIZE * 2) {
        lcd_obj->dma_size = LCD_DMA_MAX_SIZE;

        for (cnt = 0;; cnt++) { /*!< Find the buffer size that is divisible by dma_size */
            if ((config->max_buffer_size - cnt) % lcd_obj->dma_size == 0) {
                break;
            }
        }

        lcd_obj->buffer_size = config->max_buffer_size - cnt;
    } else {
        lcd_obj->dma_size = config->max_buffer_size / 2;
        lcd_obj->buffer_size = lcd_obj->dma_size * 2;
    }

    lcd_obj->half_buffer_size = lcd_obj->buffer_size / 2;

    lcd_obj->node_cnt = (lcd_obj->buffer_size) / lcd_obj->dma_size; /*!< Number of DMA nodes */
    lcd_obj->half_node_cnt = lcd_obj->node_cnt / 2;

    ESP_LOGI(TAG, "lcd_buffer_size: %d, lcd_dma_size: %d, lcd_dma_node_cnt: %d\n", lcd_obj->buffer_size, lcd_obj->dma_size, lcd_obj->node_cnt);

    lcd_obj->dma    = (lldesc_t *)heap_caps_malloc(lcd_obj->node_cnt * sizeof(lldesc_t), MALLOC_CAP_DMA);
    lcd_obj->buffer = (uint8_t *)heap_caps_malloc(lcd_obj->buffer_size * sizeof(uint8_t), MALLOC_CAP_DMA);
}

int lcd_init(lcd_config_t *config)
{
    lcd_obj = (lcd_obj_t *)heap_caps_calloc(1, sizeof(lcd_obj_t), MALLOC_CAP_DMA);

    if (!lcd_obj) {
        ESP_LOGI(TAG, "lcd object malloc error\n");
        return -1;
    }

    lcd_set_pin(config);
    lcd_config(config);
    lcd_dma_config(config);

    lcd_obj->event_queue = xQueueCreate(1, sizeof(int));

    lcd_obj->buffer_size = config->max_buffer_size;
    lcd_obj->swap_data = config->swap_data;

    lcd_obj->pin_dc = config->pin_dc;
    lcd_obj->pin_cs = config->pin_cs;
    lcd_obj->pin_rst = config->pin_rst;
    lcd_obj->pin_bk = config->pin_bk;
    lcd_set_cs(1);

    if (lcd_obj->pin_rst <= 46) {
        lcd_rst();/*!< lcd_rst before LCD Init. */
    }

    lcd_delay_ms(100);
#ifdef CONFIG_LCD_ST7789
    ESP_LOGI(TAG, "ST7789 init...\n");
    lcd_st7789_config(config);
#endif
#ifdef CONFIG_LCD_ILI9341
    ESP_LOGI(TAG, "ILI19341 init...\n");
    lcd_ili9341_config(config);
#endif

    if (lcd_obj->pin_bk <= 46) {
        lcd_set_blk(0);
    }

    ESP_LOGI(TAG, "lcd init ok\n");
    return 0;
}

void lcd_set_index(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end)
{
    uint16_t start_pos, end_pos;
    lcd_write_cmd(0x2a);    /*!< CASET (2Ah): Column Address Set */

    /*!< Must write byte than byte */
    if (lcd_obj->horizontal == 3) {
        start_pos = x_start + 80;
        end_pos = x_end + 80;
    } else {
        start_pos = x_start;
        end_pos = x_end;
    }

    lcd_write_byte(start_pos >> 8);
    lcd_write_byte(start_pos & 0xFF);
    lcd_write_byte(end_pos >> 8);
    lcd_write_byte(end_pos & 0xFF);

    lcd_write_cmd(0x2b);    /*!< RASET (2Bh): Row Address Set */

    if (lcd_obj->horizontal == 1) {
        start_pos = x_start + 80;
        end_pos = x_end + 80;
    } else {
        start_pos = y_start;
        end_pos = y_end;
    }

    lcd_write_byte(start_pos >> 8);
    lcd_write_byte(start_pos & 0xFF);
    lcd_write_byte(end_pos >> 8);
    lcd_write_byte(end_pos & 0xFF);
    lcd_write_cmd(0x2c);    /*!< RAMWR (2Ch): Memory Write */
}