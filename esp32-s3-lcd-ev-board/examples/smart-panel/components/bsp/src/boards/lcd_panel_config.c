#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "bsp_board.h"

#define USE_IO_EXPANDER 1

#if USE_IO_EXPANDER
#define gpio_set_value(_p, _v) io_ops->multi_write_new_level(_p, _v)
#define IO_EXP_MULTI_WRITE_START() io_ops->multi_write_start()
#define IO_EXP_MULTI_WRITE_END() io_ops->multi_write_end()
#else
#include "driver/gpio.h"
#define gpio_set_value(_p, _v) gpio_set_level(_p, _v)
#endif

#define Delay(t) vTaskDelay(pdMS_TO_TICKS(t))
#define udelay(_t) ets_delay_us(_t)
#define RESET(n) gpio_set_value(s_io.rst, n)
#define CS(n) gpio_set_value(s_io.cs, n)
#define SCK(n) gpio_set_value(s_io.clk, n)
#define SDO(n) gpio_set_value(s_io.sdo, n)

#define EXPANDER_IO_SPI_CS 1
#define EXPANDER_IO_SPI_SCK 2
#define EXPANDER_IO_SPI_MOSI 3

struct interface_io {
    int rst;
    int cs;
    int clk;
    int sdo;
};

static const struct interface_io s_io = {
#if USE_IO_EXPANDER
    .rst = -1,
    .cs = EXPANDER_IO_SPI_CS,
    .clk = EXPANDER_IO_SPI_SCK,
    .sdo = EXPANDER_IO_SPI_MOSI
#else
    .rst = -1,
    .cs = BOARD_SDMMC_SPI_CS_GPIO,
    .clk = BOARD_SDMMC_SPI_CLK_GPIO,
    .sdo = BOARD_SDMMC_SPI_MOSI_GPIO
#endif
};
static io_expander_ops_t *io_ops = NULL;

static int init_gpios(const struct interface_io *io)
{
#if USE_IO_EXPANDER
    io_ops = bsp_board_get_description()->io_expander_ops;
    io_ops->set_direction(io->cs, 1);
    io_ops->set_direction(io->clk, 1);
    io_ops->set_direction(io->sdo, 1);
#else
    gpio_config_t io_conf = {};
    uint64_t mask = 0;

    if (io->rst >= 0) {
        mask |= (1ULL << io->rst);
    }

    mask |= (1ULL << io->cs);
    mask |= (1ULL << io->clk);
    mask |= (1ULL << io->sdo);

    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = mask;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    if (io->rst >= 0) {
        gpio_set_value(io->rst, 1);
    }
#endif

    IO_EXP_MULTI_WRITE_START();
    gpio_set_value(io->cs, 1);
    gpio_set_value(io->clk, 1);
    gpio_set_value(io->sdo, 1);
    IO_EXP_MULTI_WRITE_END();

    return 0;
}

static void SPI_SendData(unsigned short i)
{
    unsigned char n;
    for (n = 0; n < 9; n++) {
        if (i & 0x0100) {
            SDO(1);
        } else {
            SDO(0);
        }
        i = i << 1;

        SCK(1);
        udelay(10);
        SCK(0);
        udelay(10);
    }
}

static void SPI_WriteComm(unsigned short c)
{
    CS(0);
    udelay(10);
    SCK(0);
    udelay(10);

    SPI_SendData(((c >> 8) & 0x00FF) | 0x2000);

    SCK(1);
    udelay(10);
    SCK(0);

    CS(1);
    udelay(10);
    CS(0);
    udelay(10);

    SPI_SendData((c & 0x00FF));
    CS(1);
    udelay(10);

}

static void SPI_WriteData(unsigned short d)
{
    CS(0);
    udelay(10);
    SCK(0);
    udelay(10);

    d &= 0x00FF;
    d |= 0x0100;
    SPI_SendData(d);

    SCK(1);
    udelay(10);
    SCK(0);
    udelay(10);

    CS(1);
    udelay(10);
}

/**
 * @attention
 * Don't write too much data at one time by I2C, otherwise the I2C cmd linked list will occupy a lot of memory.
 * So every time you write some data, you need to restart a new transmission
 */
void lcd_panel_gc9503np_init(void)
{
    init_gpios(&s_io);

    IO_EXP_MULTI_WRITE_START();

    SPI_WriteComm(0xF0);
    SPI_WriteData(0x55);
    SPI_WriteData(0xAA);
    SPI_WriteData(0x52);
    SPI_WriteData(0x08);
    SPI_WriteData(0x00);

    SPI_WriteComm(0xF6);
    SPI_WriteData(0x5A);
    SPI_WriteData(0x87);

    SPI_WriteComm(0xC1);
    SPI_WriteData(0x3F);

    SPI_WriteComm(0xC2);
    SPI_WriteData(0x0E);

    SPI_WriteComm(0xC6);
    SPI_WriteData(0xF8);

    SPI_WriteComm(0xC9);
    SPI_WriteData(0x10);

    SPI_WriteComm(0xCD);
    SPI_WriteData(0x25);

    SPI_WriteComm(0xF8);
    SPI_WriteData(0x8A);

    SPI_WriteComm(0xAC);
    SPI_WriteData(0x45);

    SPI_WriteComm(0xA0);
    SPI_WriteData(0xDD);

    SPI_WriteComm(0xA7);
    SPI_WriteData(0x47);

    SPI_WriteComm(0xFA);
    SPI_WriteData(0x00);
    SPI_WriteData(0x00);
    SPI_WriteData(0x00);
    SPI_WriteData(0x04);

    SPI_WriteComm(0x86);
    SPI_WriteData(0x99);
    SPI_WriteData(0xa3);
    SPI_WriteData(0xa3);
    SPI_WriteData(0x51);

    SPI_WriteComm(0xA3);
    SPI_WriteData(0xEE);

    SPI_WriteComm(0xFD);
    SPI_WriteData(0x3c);
    SPI_WriteData(0x3c);
    SPI_WriteData(0x00);

    SPI_WriteComm(0x71);
    SPI_WriteData(0x48);

    SPI_WriteComm(0x72);
    SPI_WriteData(0x48);

    SPI_WriteComm(0x73);
    SPI_WriteData(0x00);
    SPI_WriteData(0x44);

    SPI_WriteComm(0x97);
    SPI_WriteData(0xEE);

    SPI_WriteComm(0x83);
    SPI_WriteData(0x93);

    SPI_WriteComm(0x9A);
    SPI_WriteData(0x72);

    SPI_WriteComm(0x9B);
    SPI_WriteData(0x5a);

    SPI_WriteComm(0x82);
    SPI_WriteData(0x2c);
    SPI_WriteData(0x2c);

    SPI_WriteComm(0xB1);
    SPI_WriteData(0x10);
    IO_EXP_MULTI_WRITE_END();
    IO_EXP_MULTI_WRITE_START();

    SPI_WriteComm(0x6D);
    SPI_WriteData(0x00);
    SPI_WriteData(0x1F);
    SPI_WriteData(0x19);
    SPI_WriteData(0x1A);
    SPI_WriteData(0x10);
    SPI_WriteData(0x0e);
    SPI_WriteData(0x0c);
    SPI_WriteData(0x0a);
    SPI_WriteData(0x02);
    SPI_WriteData(0x07);
    SPI_WriteData(0x1E);
    SPI_WriteData(0x1E);
    SPI_WriteData(0x1E);
    SPI_WriteData(0x1E);
    SPI_WriteData(0x1E);
    SPI_WriteData(0x1E);
    SPI_WriteData(0x1E);
    SPI_WriteData(0x1E);
    SPI_WriteData(0x1E);
    SPI_WriteData(0x1E);
    SPI_WriteData(0x1E);
    SPI_WriteData(0x1E);
    SPI_WriteData(0x08);
    SPI_WriteData(0x01);
    SPI_WriteData(0x09);
    SPI_WriteData(0x0b);
    SPI_WriteData(0x0D);
    SPI_WriteData(0x0F);
    SPI_WriteData(0x1a);
    SPI_WriteData(0x19);
    SPI_WriteData(0x1f);
    SPI_WriteData(0x00);

    SPI_WriteComm(0x64);
    SPI_WriteData(0x38);
    SPI_WriteData(0x05);
    SPI_WriteData(0x01);
    SPI_WriteData(0xdb);
    SPI_WriteData(0x03);
    SPI_WriteData(0x03);
    SPI_WriteData(0x38);
    SPI_WriteData(0x04);
    SPI_WriteData(0x01);
    SPI_WriteData(0xdc);
    SPI_WriteData(0x03);
    SPI_WriteData(0x03);
    SPI_WriteData(0x7A);
    SPI_WriteData(0x7A);
    SPI_WriteData(0x7A);
    SPI_WriteData(0x7A);
    IO_EXP_MULTI_WRITE_END();
    IO_EXP_MULTI_WRITE_START();

    SPI_WriteComm(0x65);
    SPI_WriteData(0x38);
    SPI_WriteData(0x03);
    SPI_WriteData(0x01);
    SPI_WriteData(0xdd);
    SPI_WriteData(0x03);
    SPI_WriteData(0x03);
    SPI_WriteData(0x38);
    SPI_WriteData(0x02);
    SPI_WriteData(0x01);
    SPI_WriteData(0xde);
    SPI_WriteData(0x03);
    SPI_WriteData(0x03);
    SPI_WriteData(0x7A);
    SPI_WriteData(0x7A);
    SPI_WriteData(0x7A);
    SPI_WriteData(0x7A);

    SPI_WriteComm(0x66);
    SPI_WriteData(0x38);
    SPI_WriteData(0x01);
    SPI_WriteData(0x01);
    SPI_WriteData(0xdf);
    SPI_WriteData(0x03);
    SPI_WriteData(0x03);
    SPI_WriteData(0x38);
    SPI_WriteData(0x00);
    SPI_WriteData(0x01);
    SPI_WriteData(0xe0);
    SPI_WriteData(0x03);
    SPI_WriteData(0x03);
    SPI_WriteData(0x7A);
    SPI_WriteData(0x7A);
    SPI_WriteData(0x7A);
    SPI_WriteData(0x7A);

    SPI_WriteComm(0x67);
    SPI_WriteData(0x30);
    SPI_WriteData(0x01);
    SPI_WriteData(0x01);
    SPI_WriteData(0xe1);
    SPI_WriteData(0x03);
    SPI_WriteData(0x03);
    SPI_WriteData(0x30);
    SPI_WriteData(0x02);
    SPI_WriteData(0x01);
    SPI_WriteData(0xe2);
    SPI_WriteData(0x03);
    SPI_WriteData(0x03);
    SPI_WriteData(0x7A);
    SPI_WriteData(0x7A);
    SPI_WriteData(0x7A);
    SPI_WriteData(0x7A);
    IO_EXP_MULTI_WRITE_END();
    IO_EXP_MULTI_WRITE_START();

    SPI_WriteComm(0x68);
    SPI_WriteData(0x00);
    SPI_WriteData(0x08);
    SPI_WriteData(0x15);
    SPI_WriteData(0x08);
    SPI_WriteData(0x15);
    SPI_WriteData(0x7A);
    SPI_WriteData(0x7A);
    SPI_WriteData(0x08);
    SPI_WriteData(0x15);
    SPI_WriteData(0x08);
    SPI_WriteData(0x15);
    SPI_WriteData(0x7A);
    SPI_WriteData(0x7A);

    SPI_WriteComm(0x60);
    SPI_WriteData(0x38);
    SPI_WriteData(0x08);
    SPI_WriteData(0x7A);
    SPI_WriteData(0x7A);
    SPI_WriteData(0x38);
    SPI_WriteData(0x09);
    SPI_WriteData(0x7A);
    SPI_WriteData(0x7A);

    SPI_WriteComm(0x63);
    SPI_WriteData(0x31);
    SPI_WriteData(0xe4);
    SPI_WriteData(0x7A);
    SPI_WriteData(0x7A);
    SPI_WriteData(0x31);
    SPI_WriteData(0xe5);
    SPI_WriteData(0x7A);
    SPI_WriteData(0x7A);

    SPI_WriteComm(0x69);
    SPI_WriteData(0x04);
    SPI_WriteData(0x22);
    SPI_WriteData(0x14);
    SPI_WriteData(0x22);
    SPI_WriteData(0x14);
    SPI_WriteData(0x22);
    SPI_WriteData(0x08);
    IO_EXP_MULTI_WRITE_END();
    IO_EXP_MULTI_WRITE_START();

    SPI_WriteComm(0x6B);
    SPI_WriteData(0x07);

    SPI_WriteComm(0x7A);
    SPI_WriteData(0x08);
    SPI_WriteData(0x13);

    SPI_WriteComm(0x7B);
    SPI_WriteData(0x08);
    SPI_WriteData(0x13);

    SPI_WriteComm(0xD1);
    SPI_WriteData(0x00);
    SPI_WriteData(0x00);
    SPI_WriteData(0x00);
    SPI_WriteData(0x04);
    SPI_WriteData(0x00);
    SPI_WriteData(0x12);
    SPI_WriteData(0x00);
    SPI_WriteData(0x18);
    SPI_WriteData(0x00);
    SPI_WriteData(0x21);
    SPI_WriteData(0x00);
    SPI_WriteData(0x2a);
    SPI_WriteData(0x00);
    SPI_WriteData(0x35);
    SPI_WriteData(0x00);
    SPI_WriteData(0x47);
    SPI_WriteData(0x00);
    SPI_WriteData(0x56);
    SPI_WriteData(0x00);
    SPI_WriteData(0x90);
    SPI_WriteData(0x00);
    SPI_WriteData(0xe5);
    SPI_WriteData(0x01);
    SPI_WriteData(0x68);
    SPI_WriteData(0x01);
    SPI_WriteData(0xd5);
    SPI_WriteData(0x01);
    SPI_WriteData(0xd7);
    SPI_WriteData(0x02);
    SPI_WriteData(0x36);
    SPI_WriteData(0x02);
    SPI_WriteData(0xa6);
    SPI_WriteData(0x02);
    SPI_WriteData(0xee);
    SPI_WriteData(0x03);
    SPI_WriteData(0x48);
    SPI_WriteData(0x03);
    SPI_WriteData(0xa0);
    SPI_WriteData(0x03);
    SPI_WriteData(0xba);
    SPI_WriteData(0x03);
    SPI_WriteData(0xc5);
    SPI_WriteData(0x03);
    SPI_WriteData(0xd0);
    SPI_WriteData(0x03);
    SPI_WriteData(0xE0);
    SPI_WriteData(0x03);
    SPI_WriteData(0xea);
    SPI_WriteData(0x03);
    SPI_WriteData(0xFa);
    SPI_WriteData(0x03);
    SPI_WriteData(0xFF);
    IO_EXP_MULTI_WRITE_END();
    IO_EXP_MULTI_WRITE_START();

    SPI_WriteComm(0xD2);
    SPI_WriteData(0x00);
    SPI_WriteData(0x00);
    SPI_WriteData(0x00);
    SPI_WriteData(0x04);
    SPI_WriteData(0x00);
    SPI_WriteData(0x12);
    SPI_WriteData(0x00);
    SPI_WriteData(0x18);
    SPI_WriteData(0x00);
    SPI_WriteData(0x21);
    SPI_WriteData(0x00);
    SPI_WriteData(0x2a);
    SPI_WriteData(0x00);
    SPI_WriteData(0x35);
    SPI_WriteData(0x00);
    SPI_WriteData(0x47);
    SPI_WriteData(0x00);
    SPI_WriteData(0x56);
    SPI_WriteData(0x00);
    SPI_WriteData(0x90);
    SPI_WriteData(0x00);
    SPI_WriteData(0xe5);
    SPI_WriteData(0x01);
    SPI_WriteData(0x68);
    SPI_WriteData(0x01);
    SPI_WriteData(0xd5);
    SPI_WriteData(0x01);
    SPI_WriteData(0xd7);
    SPI_WriteData(0x02);
    SPI_WriteData(0x36);
    SPI_WriteData(0x02);
    SPI_WriteData(0xa6);
    SPI_WriteData(0x02);
    SPI_WriteData(0xee);
    SPI_WriteData(0x03);
    SPI_WriteData(0x48);
    SPI_WriteData(0x03);
    SPI_WriteData(0xa0);
    SPI_WriteData(0x03);
    SPI_WriteData(0xba);
    SPI_WriteData(0x03);
    SPI_WriteData(0xc5);
    SPI_WriteData(0x03);
    SPI_WriteData(0xd0);
    SPI_WriteData(0x03);
    SPI_WriteData(0xE0);
    SPI_WriteData(0x03);
    SPI_WriteData(0xea);
    SPI_WriteData(0x03);
    SPI_WriteData(0xFa);
    SPI_WriteData(0x03);
    SPI_WriteData(0xFF);
    IO_EXP_MULTI_WRITE_END();
    IO_EXP_MULTI_WRITE_START();

    SPI_WriteComm(0xD3);
    SPI_WriteData(0x00);
    SPI_WriteData(0x00);
    SPI_WriteData(0x00);
    SPI_WriteData(0x04);
    SPI_WriteData(0x00);
    SPI_WriteData(0x12);
    SPI_WriteData(0x00);
    SPI_WriteData(0x18);
    SPI_WriteData(0x00);
    SPI_WriteData(0x21);
    SPI_WriteData(0x00);
    SPI_WriteData(0x2a);
    SPI_WriteData(0x00);
    SPI_WriteData(0x35);
    SPI_WriteData(0x00);
    SPI_WriteData(0x47);
    SPI_WriteData(0x00);
    SPI_WriteData(0x56);
    SPI_WriteData(0x00);
    SPI_WriteData(0x90);
    SPI_WriteData(0x00);
    SPI_WriteData(0xe5);
    SPI_WriteData(0x01);
    SPI_WriteData(0x68);
    SPI_WriteData(0x01);
    SPI_WriteData(0xd5);
    SPI_WriteData(0x01);
    SPI_WriteData(0xd7);
    SPI_WriteData(0x02);
    SPI_WriteData(0x36);
    SPI_WriteData(0x02);
    SPI_WriteData(0xa6);
    SPI_WriteData(0x02);
    SPI_WriteData(0xee);
    SPI_WriteData(0x03);
    SPI_WriteData(0x48);
    SPI_WriteData(0x03);
    SPI_WriteData(0xa0);
    SPI_WriteData(0x03);
    SPI_WriteData(0xba);
    SPI_WriteData(0x03);
    SPI_WriteData(0xc5);
    SPI_WriteData(0x03);
    SPI_WriteData(0xd0);
    SPI_WriteData(0x03);
    SPI_WriteData(0xE0);
    SPI_WriteData(0x03);
    SPI_WriteData(0xea);
    SPI_WriteData(0x03);
    SPI_WriteData(0xFa);
    SPI_WriteData(0x03);
    SPI_WriteData(0xFF);
    IO_EXP_MULTI_WRITE_END();
    IO_EXP_MULTI_WRITE_START();

    SPI_WriteComm(0xD4);
    SPI_WriteData(0x00);
    SPI_WriteData(0x00);
    SPI_WriteData(0x00);
    SPI_WriteData(0x04);
    SPI_WriteData(0x00);
    SPI_WriteData(0x12);
    SPI_WriteData(0x00);
    SPI_WriteData(0x18);
    SPI_WriteData(0x00);
    SPI_WriteData(0x21);
    SPI_WriteData(0x00);
    SPI_WriteData(0x2a);
    SPI_WriteData(0x00);
    SPI_WriteData(0x35);
    SPI_WriteData(0x00);
    SPI_WriteData(0x47);
    SPI_WriteData(0x00);
    SPI_WriteData(0x56);
    SPI_WriteData(0x00);
    SPI_WriteData(0x90);
    SPI_WriteData(0x00);
    SPI_WriteData(0xe5);
    SPI_WriteData(0x01);
    SPI_WriteData(0x68);
    SPI_WriteData(0x01);
    SPI_WriteData(0xd5);
    SPI_WriteData(0x01);
    SPI_WriteData(0xd7);
    SPI_WriteData(0x02);
    SPI_WriteData(0x36);
    SPI_WriteData(0x02);
    SPI_WriteData(0xa6);
    SPI_WriteData(0x02);
    SPI_WriteData(0xee);
    SPI_WriteData(0x03);
    SPI_WriteData(0x48);
    SPI_WriteData(0x03);
    SPI_WriteData(0xa0);
    SPI_WriteData(0x03);
    SPI_WriteData(0xba);
    SPI_WriteData(0x03);
    SPI_WriteData(0xc5);
    SPI_WriteData(0x03);
    SPI_WriteData(0xd0);
    SPI_WriteData(0x03);
    SPI_WriteData(0xE0);
    SPI_WriteData(0x03);
    SPI_WriteData(0xea);
    SPI_WriteData(0x03);
    SPI_WriteData(0xFa);
    SPI_WriteData(0x03);
    SPI_WriteData(0xFF);
    IO_EXP_MULTI_WRITE_END();
    IO_EXP_MULTI_WRITE_START();

    SPI_WriteComm(0xD5);
    SPI_WriteData(0x00);
    SPI_WriteData(0x00);
    SPI_WriteData(0x00);
    SPI_WriteData(0x04);
    SPI_WriteData(0x00);
    SPI_WriteData(0x12);
    SPI_WriteData(0x00);
    SPI_WriteData(0x18);
    SPI_WriteData(0x00);
    SPI_WriteData(0x21);
    SPI_WriteData(0x00);
    SPI_WriteData(0x2a);
    SPI_WriteData(0x00);
    SPI_WriteData(0x35);
    SPI_WriteData(0x00);
    SPI_WriteData(0x47);
    SPI_WriteData(0x00);
    SPI_WriteData(0x56);
    SPI_WriteData(0x00);
    SPI_WriteData(0x90);
    SPI_WriteData(0x00);
    SPI_WriteData(0xe5);
    SPI_WriteData(0x01);
    SPI_WriteData(0x68);
    SPI_WriteData(0x01);
    SPI_WriteData(0xd5);
    SPI_WriteData(0x01);
    SPI_WriteData(0xd7);
    SPI_WriteData(0x02);
    SPI_WriteData(0x36);
    SPI_WriteData(0x02);
    SPI_WriteData(0xa6);
    SPI_WriteData(0x02);
    SPI_WriteData(0xee);
    SPI_WriteData(0x03);
    SPI_WriteData(0x48);
    SPI_WriteData(0x03);
    SPI_WriteData(0xa0);
    SPI_WriteData(0x03);
    SPI_WriteData(0xba);
    SPI_WriteData(0x03);
    SPI_WriteData(0xc5);
    SPI_WriteData(0x03);
    SPI_WriteData(0xd0);
    SPI_WriteData(0x03);
    SPI_WriteData(0xE0);
    SPI_WriteData(0x03);
    SPI_WriteData(0xea);
    SPI_WriteData(0x03);
    SPI_WriteData(0xFa);
    SPI_WriteData(0x03);
    SPI_WriteData(0xFF);
    IO_EXP_MULTI_WRITE_END();
    IO_EXP_MULTI_WRITE_START();

    SPI_WriteComm(0xD6);
    SPI_WriteData(0x00);
    SPI_WriteData(0x00);
    SPI_WriteData(0x00);
    SPI_WriteData(0x04);
    SPI_WriteData(0x00);
    SPI_WriteData(0x12);
    SPI_WriteData(0x00);
    SPI_WriteData(0x18);
    SPI_WriteData(0x00);
    SPI_WriteData(0x21);
    SPI_WriteData(0x00);
    SPI_WriteData(0x2a);
    SPI_WriteData(0x00);
    SPI_WriteData(0x35);
    SPI_WriteData(0x00);
    SPI_WriteData(0x47);
    SPI_WriteData(0x00);
    SPI_WriteData(0x56);
    SPI_WriteData(0x00);
    SPI_WriteData(0x90);
    SPI_WriteData(0x00);
    SPI_WriteData(0xe5);
    SPI_WriteData(0x01);
    SPI_WriteData(0x68);
    SPI_WriteData(0x01);
    SPI_WriteData(0xd5);
    SPI_WriteData(0x01);
    SPI_WriteData(0xd7);
    SPI_WriteData(0x02);
    SPI_WriteData(0x36);
    SPI_WriteData(0x02);
    SPI_WriteData(0xa6);
    SPI_WriteData(0x02);
    SPI_WriteData(0xee);
    SPI_WriteData(0x03);
    SPI_WriteData(0x48);
    SPI_WriteData(0x03);
    SPI_WriteData(0xa0);
    SPI_WriteData(0x03);
    SPI_WriteData(0xba);
    SPI_WriteData(0x03);
    SPI_WriteData(0xc5);
    SPI_WriteData(0x03);
    SPI_WriteData(0xd0);
    SPI_WriteData(0x03);
    SPI_WriteData(0xE0);
    SPI_WriteData(0x03);
    SPI_WriteData(0xea);
    SPI_WriteData(0x03);
    SPI_WriteData(0xFa);
    SPI_WriteData(0x03);
    SPI_WriteData(0xFF);

    SPI_WriteComm(0x3a);
    SPI_WriteData(0x66);
    IO_EXP_MULTI_WRITE_END();

    IO_EXP_MULTI_WRITE_START();
    SPI_WriteComm(0x11);
    IO_EXP_MULTI_WRITE_END();
    vTaskDelay(pdMS_TO_TICKS(120));

    IO_EXP_MULTI_WRITE_START();
    SPI_WriteComm(0x29);
    IO_EXP_MULTI_WRITE_END();
    vTaskDelay(pdMS_TO_TICKS(20));

    IO_EXP_MULTI_WRITE_START();
    CS(1);
    SCK(1);
    SDO(1);
    IO_EXP_MULTI_WRITE_END();
}

void lcd_panel_st7701s_init(void)
{
    init_gpios(&s_io);

    IO_EXP_MULTI_WRITE_START();
    SPI_WriteComm(0x11);
    IO_EXP_MULTI_WRITE_END();

    Delay(120);

    IO_EXP_MULTI_WRITE_START();
    SPI_WriteComm(0xFF);
    SPI_WriteData(0x77);
    SPI_WriteData(0x01);
    SPI_WriteData(0x00);
    SPI_WriteData(0x00);
    SPI_WriteData(0x10);

    SPI_WriteComm(0xC0);
    SPI_WriteData(0x3B); //480*480
    SPI_WriteData(0x00);

    SPI_WriteComm(0xC1);
    SPI_WriteData(0x0D);
    SPI_WriteData(0x02);

    SPI_WriteComm(0xC2);
    SPI_WriteData(0x21);
    SPI_WriteData(0x08);

    SPI_WriteComm(0xB0);
    SPI_WriteData(0x00);
    SPI_WriteData(0x11);
    SPI_WriteData(0x18);
    SPI_WriteData(0x0E);
    SPI_WriteData(0x11);
    SPI_WriteData(0x06);
    SPI_WriteData(0x07);
    SPI_WriteData(0x08);
    SPI_WriteData(0x07);
    SPI_WriteData(0x22);
    SPI_WriteData(0x04);
    SPI_WriteData(0x12);
    SPI_WriteData(0x0F);
    SPI_WriteData(0xAA);
    SPI_WriteData(0x31);
    SPI_WriteData(0x18);
    IO_EXP_MULTI_WRITE_END();
    IO_EXP_MULTI_WRITE_START();

    SPI_WriteComm(0xB1);
    SPI_WriteData(0x00);
    SPI_WriteData(0x11);
    SPI_WriteData(0x19);
    SPI_WriteData(0x0E);
    SPI_WriteData(0x12);
    SPI_WriteData(0x07);
    SPI_WriteData(0x08);
    SPI_WriteData(0x08);
    SPI_WriteData(0x08);
    SPI_WriteData(0x22);
    SPI_WriteData(0x04);
    SPI_WriteData(0x11);
    SPI_WriteData(0x11);
    SPI_WriteData(0xA9);
    SPI_WriteData(0x32);
    SPI_WriteData(0x18);

    SPI_WriteComm(0xFF);
    SPI_WriteData(0x77);
    SPI_WriteData(0x01);
    SPI_WriteData(0x00);
    SPI_WriteData(0x00);
    SPI_WriteData(0x11);

    SPI_WriteComm(0xB0);
    SPI_WriteData(0x60);

    SPI_WriteComm(0xB1);
    SPI_WriteData(0x30);

    SPI_WriteComm(0xB2);
    SPI_WriteData(0x87);

    SPI_WriteComm(0xB3);
    SPI_WriteData(0x80);

    SPI_WriteComm(0xB5);
    SPI_WriteData(0x49);

    SPI_WriteComm(0xB7);
    SPI_WriteData(0x85);

    SPI_WriteComm(0xB8);
    SPI_WriteData(0x21);

    SPI_WriteComm(0xC1);
    SPI_WriteData(0x78);

    SPI_WriteComm(0xC2);
    SPI_WriteData(0x78);
    IO_EXP_MULTI_WRITE_END();

    Delay(20);
    IO_EXP_MULTI_WRITE_START();
    SPI_WriteComm(0xE0);
    SPI_WriteData(0x00);
    SPI_WriteData(0x1B);
    SPI_WriteData(0x02);

    SPI_WriteComm(0xE1);
    SPI_WriteData(0x08);
    SPI_WriteData(0xA0);
    SPI_WriteData(0x00);
    SPI_WriteData(0x00);
    SPI_WriteData(0x07);
    SPI_WriteData(0xA0);
    SPI_WriteData(0x00);
    SPI_WriteData(0x00);
    SPI_WriteData(0x00);
    SPI_WriteData(0x44);
    SPI_WriteData(0x44);

    SPI_WriteComm(0xE2);
    SPI_WriteData(0x11);
    SPI_WriteData(0x11);
    SPI_WriteData(0x44);
    SPI_WriteData(0x44);
    SPI_WriteData(0xED);
    SPI_WriteData(0xA0);
    SPI_WriteData(0x00);
    SPI_WriteData(0x00);
    SPI_WriteData(0xEC);
    SPI_WriteData(0xA0);
    SPI_WriteData(0x00);
    SPI_WriteData(0x00);
    IO_EXP_MULTI_WRITE_END();
    IO_EXP_MULTI_WRITE_START();

    SPI_WriteComm(0xE3);
    SPI_WriteData(0x00);
    SPI_WriteData(0x00);
    SPI_WriteData(0x11);
    SPI_WriteData(0x11);

    SPI_WriteComm(0xE4);
    SPI_WriteData(0x44);
    SPI_WriteData(0x44);

    SPI_WriteComm(0xE5);
    SPI_WriteData(0x0A);
    SPI_WriteData(0xE9);
    SPI_WriteData(0xD8);
    SPI_WriteData(0xA0);
    SPI_WriteData(0x0C);
    SPI_WriteData(0xEB);
    SPI_WriteData(0xD8);
    SPI_WriteData(0xA0);
    SPI_WriteData(0x0E);
    SPI_WriteData(0xED);
    SPI_WriteData(0xD8);
    SPI_WriteData(0xA0);
    SPI_WriteData(0x10);
    SPI_WriteData(0xEF);
    SPI_WriteData(0xD8);
    SPI_WriteData(0xA0);

    SPI_WriteComm(0xE6);
    SPI_WriteData(0x00);
    SPI_WriteData(0x00);
    SPI_WriteData(0x11);
    SPI_WriteData(0x11);

    SPI_WriteComm(0xE7);
    SPI_WriteData(0x44);
    SPI_WriteData(0x44);

    IO_EXP_MULTI_WRITE_END();
    IO_EXP_MULTI_WRITE_START();

    SPI_WriteComm(0xE8);
    SPI_WriteData(0x09);
    SPI_WriteData(0xE8);
    SPI_WriteData(0xD8);
    SPI_WriteData(0xA0);
    SPI_WriteData(0x0B);
    SPI_WriteData(0xEA);
    SPI_WriteData(0xD8);
    SPI_WriteData(0xA0);
    SPI_WriteData(0x0D);
    SPI_WriteData(0xEC);
    SPI_WriteData(0xD8);
    SPI_WriteData(0xA0);
    SPI_WriteData(0x0F);
    SPI_WriteData(0xEE);
    SPI_WriteData(0xD8);
    SPI_WriteData(0xA0);

    SPI_WriteComm(0xEB);
    SPI_WriteData(0x02);
    SPI_WriteData(0x00);
    SPI_WriteData(0xE4);
    SPI_WriteData(0xE4);
    SPI_WriteData(0x88);
    SPI_WriteData(0x00);
    SPI_WriteData(0x40);
    SPI_WriteComm(0xEC);
    SPI_WriteData(0x3C);
    SPI_WriteData(0x00);

    SPI_WriteComm(0xED);
    SPI_WriteData(0xAB);
    SPI_WriteData(0x89);
    SPI_WriteData(0x76);
    SPI_WriteData(0x54);
    SPI_WriteData(0x02);
    SPI_WriteData(0xFF);
    SPI_WriteData(0xFF);
    SPI_WriteData(0xFF);
    SPI_WriteData(0xFF);
    SPI_WriteData(0xFF);
    SPI_WriteData(0xFF);
    SPI_WriteData(0x20);
    SPI_WriteData(0x45);
    SPI_WriteData(0x67);
    SPI_WriteData(0x98);
    SPI_WriteData(0xBA);
    IO_EXP_MULTI_WRITE_END();
    IO_EXP_MULTI_WRITE_START();

    SPI_WriteComm(0xFF);
    SPI_WriteData(0x77);
    SPI_WriteData(0x01);
    SPI_WriteData(0x00);
    SPI_WriteData(0x00);
    SPI_WriteData(0x00);

    SPI_WriteComm(0x3A);
    SPI_WriteData(0x70);

    SPI_WriteComm(0x36);
    SPI_WriteData(0x00);


    SPI_WriteComm(0x11);
    IO_EXP_MULTI_WRITE_END();
    Delay(120);

    IO_EXP_MULTI_WRITE_START();
    SPI_WriteComm(0x29);
    IO_EXP_MULTI_WRITE_END();
    Delay(120);

    IO_EXP_MULTI_WRITE_START();
    CS(1);
    SCK(1);
    SDO(1);
    IO_EXP_MULTI_WRITE_END();
}
