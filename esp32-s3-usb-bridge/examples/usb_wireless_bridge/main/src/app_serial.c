/* SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/ringbuf.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "tusb_config.h"
#include "tusb.h"
#include "app_util.h"
#include "app_serial.h"
#include "app_io.h"
#include "app_jtag.h"
#include "app_helper.h"
#include "esp32_port.h"
#include "esp_timer.h"
#include "esp_idf_version.h"
#include "esp_loader.h"
#include "esp_log.h"

static const char *TAG = "bridge_serial";

static QueueHandle_t uart_queue;
static RingbufHandle_t sendbuf = NULL;
static SemaphoreHandle_t tx_requested = NULL;
static SemaphoreHandle_t tx_done = NULL;

static esp_timer_handle_t state_change_timer;

static atomic_bool serial_read_enabled = false;

static usb_bridge_mode_t *usb_bridge_mode = NULL;

static void uart_event_task(void *pvParameters)
{
    uart_event_t event;
    uint8_t dtmp[SLAVE_UART_BUF_SIZE];

    while (1) {
        if (xQueueReceive(uart_queue, (void *) &event, portMAX_DELAY)) {
            switch (event.type) {
            case UART_DATA:
                if (serial_read_enabled) {
                    size_t buffered_len;
                    uart_get_buffered_data_len(SLAVE_UART_NUM, &buffered_len);
                    const int read = uart_read_bytes(SLAVE_UART_NUM, dtmp, MIN(buffered_len, SLAVE_UART_BUF_SIZE), portMAX_DELAY);
                    ESP_LOGD(TAG, "UART -> CDC ringbuffer (%d bytes)", read);
                    ESP_LOG_BUFFER_HEXDUMP("UART -> CDC", dtmp, read, ESP_LOG_DEBUG);

                    // We cannot wait it here because UART events would overflow and have to copy the data into
                    // another buffer and wait until it can be sent.
                    if (xRingbufferSend(sendbuf, dtmp, read, pdMS_TO_TICKS(10)) != pdTRUE) {
                        ESP_LOGV(TAG, "Cannot write to ringbuffer (free %d of %d)!",
                                 xRingbufferGetCurFreeSize(sendbuf),
                                 USB_SEND_RINGBUFFER_SIZE);
                        vTaskDelay(pdMS_TO_TICKS(10));
                    }
                }
                break;
            case UART_FIFO_OVF:
                ESP_LOGW(TAG, "UART FIFO overflow");
                uart_flush_input(SLAVE_UART_NUM);
                xQueueReset(uart_queue);
                break;
            case UART_BUFFER_FULL:
                ESP_LOGW(TAG, "UART ring buffer full");
                uart_flush_input(SLAVE_UART_NUM);
                xQueueReset(uart_queue);
                break;
            case UART_BREAK:
                ESP_LOGW(TAG, "UART RX break");
                break;
            case UART_PARITY_ERR:
                ESP_LOGW(TAG, "UART parity error");
                break;
            case UART_FRAME_ERR:
                ESP_LOGW(TAG, "UART frame error");
                break;
            default:
                ESP_LOGW(TAG, "UART event type: %d", event.type);
                break;
            }
            taskYIELD();
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    vTaskDelete(NULL);
}

static esp_err_t usb_wait_for_tx(const uint32_t block_time_ms)
{
    if (xSemaphoreTake(tx_done, pdMS_TO_TICKS(block_time_ms)) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }
    return ESP_OK;
}

static void usb_sender_task(void *pvParameters)
{
    while (1) {
        size_t ringbuf_received;
        uint8_t *buf = (uint8_t *) xRingbufferReceiveUpTo(sendbuf, &ringbuf_received, pdMS_TO_TICKS(100),
                       CFG_TUD_CDC_TX_BUFSIZE);

        if (buf) {
            uint8_t int_buf[CFG_TUD_CDC_TX_BUFSIZE];
            memcpy(int_buf, buf, ringbuf_received);
            vRingbufferReturnItem(sendbuf, (void *) buf);

            for (int transferred = 0, to_send = ringbuf_received; transferred < ringbuf_received;) {
                xSemaphoreGive(tx_requested);
                const int wr_len = tud_cdc_write(int_buf + transferred, to_send);
                /* tinyusb might have been flushed the data. In case not flushed, we are flushing here.
                    2nd atttempt might return zero, meaning there is no data to transfer. So it is safe to call it again.
                */
                tud_cdc_write_flush();
                if (usb_wait_for_tx(50) != ESP_OK) {
                    xSemaphoreTake(tx_requested, 0);
                    tud_cdc_write_clear(); /* host might be disconnected. drop the buffer */
                    ESP_LOGV(TAG, "usb tx timeout");
                    break;
                }
                ESP_LOGD(TAG, "CDC ringbuffer -> CDC (%d bytes)", wr_len);
                transferred += wr_len;
                to_send -= wr_len;
            }
        } else {
            ESP_LOGD(TAG, "usb_sender_task: nothing to send");
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }
    }
    vTaskDelete(NULL);
}

void tud_cdc_tx_complete_cb(const uint8_t itf)
{
    if (xSemaphoreTake(tx_requested, 0) != pdTRUE) {
        /* Semaphore should have been given before write attempt.
            Sometimes tinyusb can send one more cb even xfer_complete len is zero
        */
        return;
    }

    xSemaphoreGive(tx_done);
}

void tud_cdc_rx_cb(const uint8_t itf)
{
    uint8_t buf[CFG_TUD_CDC_RX_BUFSIZE];

    const uint32_t rx_size = tud_cdc_n_read(itf, buf, CFG_TUD_CDC_RX_BUFSIZE);
    if (rx_size > 0) {
        ESP_LOGD(TAG, "CDC -> UART (%" PRId32 " bytes)", rx_size);
        ESP_LOG_BUFFER_HEXDUMP("CDC -> UART", buf, rx_size, ESP_LOG_DEBUG);
        ERROR_CHECK_RETURN_VOID(NULL != usb_bridge_mode, "usb_bridge_mode can't be NULL");
        if (usb_bridge_mode->write_cdc_rx_data!= NULL){
            usb_bridge_mode->write_cdc_rx_data(buf, rx_size);
        }
    } else {
        ESP_LOGW(TAG, "tud_cdc_rx_cb receive error");
    }
}

void tud_cdc_line_coding_cb(const uint8_t itf, cdc_line_coding_t const *p_line_coding)
{
    static int last_bit_rate = -1;
    if (last_bit_rate != p_line_coding->bit_rate) {
        ERROR_CHECK_RETURN_VOID(NULL != usb_bridge_mode, "usb_bridge_mode can't be NULL");
        // printf("%p\n", usb_bridge_mode->set_baudrate);
        usb_bridge_mode->set_baudrate(p_line_coding->bit_rate);
        last_bit_rate = p_line_coding->bit_rate;
    }
    printf("tud_cdc_line_coding_cb: %"PRIu32"\n", p_line_coding->bit_rate);
}

void serial_boot_reset(const bool dtr, const bool rts)
{
    // The following transformation of DTR & RTS signals to BOOT & RST is done based on auto reset circutry shown in
    // schematics of ESP boards.

    // defaults for ((dtr && rts) || (!dtr && !rts))
    bool rst = true;
    bool boot = true;

    if (!dtr && rts) {
        rst = false;
        boot = true;
    } else if (dtr && !rts) {
        rst = true;
        boot = false;
    }

    esp_timer_stop(state_change_timer);  // maybe it is not started so not check the exit value

    if (dtr & rts) {
        // The assignment of BOOT=1 and RST=1 is postponed and it is done only if no other state change occurs in time
        // period set by the timer.
        // This is a patch for Esptool. Esptool generates DTR=0 & RTS=1 followed by DTR=1 & RTS=0. However, a callback
        // with DTR = 1 & RTS = 1 is received between. This would prevent to put the target chip into download mode.
        ESP_ERROR_CHECK(esp_timer_start_once(state_change_timer, 10 * 1000 /*us*/));
        
        // Enable serial read on the first connection. It can be kept enabled after disconnection because that is not
        // causing crash of the USB subsystem.
        serial_read_enabled = true;
    } else {
        ESP_LOGI(TAG, "DTR = %d, RTS = %d -> BOOT = %d, RST = %d", dtr, rts, boot, rst);

        gpio_set_level(GPIO_BOOT, boot);
        gpio_set_level(GPIO_RST, rst);

        serial_set_baudrate(SLAVE_UART_DEFAULT_BAUD);

        // On ESP32, TDI jtag signal is on GPIO12, which is also a strapping pin that determines flash voltage.
        // If TDI is high when ESP32 is released from external reset, the flash voltage is set to 1.8V, and the chip will fail to boot.
        // As a solution, MTDI signal forced to be low when RST is about to go high.
        static bool tdi_bootstrapping = false;
        if (jtag_get_target_model() == CHIP_ESP32 && !tdi_bootstrapping && boot && !rst) {
            jtag_task_suspend();
            tdi_bootstrapping = true;
            gpio_set_level(CONFIG_BRIDGE_GPIO_TDO, 0);
            ESP_LOGW(TAG, "jtag task suspended");
        }
        if (tdi_bootstrapping && boot && rst) {
            esp_rom_delay_us(1000); /* wait for reset */
            jtag_task_resume();
            tdi_bootstrapping = false;
            ESP_LOGW(TAG, "jtag task resumed");
        }
    }
}

void tud_cdc_line_state_cb(const uint8_t itf, const bool dtr, const bool rts)
{
    ERROR_CHECK_RETURN_VOID(NULL != usb_bridge_mode, "usb_bridge_mode can't be NULL");
    usb_bridge_mode->set_boot_reset(dtr, rts);
}

static void state_change_timer_cb(void *arg)
{
    ESP_LOGI(TAG, "BOOT = 1, RST = 1");
    gpio_set_level(GPIO_BOOT, true);
    gpio_set_level(GPIO_RST, true);
}

static void init_state_change_timer(void)
{
    const esp_timer_create_args_t timer_args = {
        .callback = state_change_timer_cb,
        .name = "serial_state_change"
    };
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &state_change_timer));
}

esp_err_t mode_wired_serial_init(RingbufHandle_t *ringbuf)
{
    usb_bridge_mode = usb_bridge_get_handle();
    ERROR_CHECK(NULL != usb_bridge_mode, "usb_bridge_mode can't be NULL", ESP_ERR_INVALID_STATE);
    const loader_esp32_config_t serial_conf = {
        .baud_rate = SLAVE_UART_DEFAULT_BAUD,
        .uart_port = SLAVE_UART_NUM,
        .uart_rx_pin = GPIO_RXD,
        .uart_tx_pin = GPIO_TXD,
        .rx_buffer_size = SLAVE_UART_BUF_SIZE * 2,
        .tx_buffer_size = 0,
        .uart_queue = &uart_queue,
        .queue_size = 20,
        .reset_trigger_pin = GPIO_RST,
        .gpio0_trigger_pin = GPIO_BOOT,
    };

    if (loader_port_esp32_init(&serial_conf) == ESP_LOADER_SUCCESS) {
        ESP_LOGI(TAG, "UART & GPIO have been initialized");

        gpio_set_level(GPIO_RST, 1);
        gpio_set_level(GPIO_BOOT, 1);

        init_state_change_timer();

        sendbuf = xRingbufferCreate(USB_SEND_RINGBUFFER_SIZE, RINGBUF_TYPE_BYTEBUF);
        if (sendbuf) {
            *ringbuf = sendbuf;
            tx_done = xSemaphoreCreateBinary();
            tx_requested = xSemaphoreCreateBinary();
            xTaskCreate(usb_sender_task, "usb_sender_task", 3 * 1024, NULL, 5, NULL);
            xTaskCreate(uart_event_task, "uart_event_task", 10 * 1024, NULL, 5, NULL);
        } else {
            ESP_LOGE(TAG, "Cannot create ringbuffer for USB sender");
            eub_abort();
        }

        // Serial read is enabled only with the first connection. This is done in order to
        // avoid crashing the USB subsystem.
        serial_read_enabled = false;
    } else {
        ESP_LOGE(TAG, "loader_port_serial_init failed");
        eub_abort();
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t mode_wireless_host_serial_init(RingbufHandle_t *ringbuf)
{
    usb_bridge_mode = usb_bridge_get_handle();
    ERROR_CHECK(NULL != usb_bridge_mode, "usb_bridge_mode can't be NULL", ESP_ERR_INVALID_STATE);
    sendbuf = xRingbufferCreate(USB_SEND_RINGBUFFER_SIZE, RINGBUF_TYPE_BYTEBUF);
    if (sendbuf) {
        *ringbuf = sendbuf;
        tx_done = xSemaphoreCreateBinary();
        tx_requested = xSemaphoreCreateBinary();
        xTaskCreate(usb_sender_task, "usb_sender_task", 3 * 1024, NULL, 5, NULL);
    } else {
        ESP_LOGE(TAG, "Cannot create ringbuffer for USB sender");
        eub_abort();
        return ESP_FAIL;
    }
    // Serial read is enabled only with the first connection. This is done in order to
    // avoid crashing the USB subsystem.
    serial_read_enabled = false;
    return ESP_OK;
}

esp_err_t mode_wireless_slave_serial_init(RingbufHandle_t *ringbuf)
{
    usb_bridge_mode = usb_bridge_get_handle();
    ERROR_CHECK(NULL != usb_bridge_mode, "usb_bridge_mode can't be NULL", ESP_ERR_INVALID_STATE);
    const loader_esp32_config_t serial_conf = {
        .baud_rate = SLAVE_UART_DEFAULT_BAUD,
        .uart_port = SLAVE_UART_NUM,
        .uart_rx_pin = GPIO_RXD,
        .uart_tx_pin = GPIO_TXD,
        .rx_buffer_size = SLAVE_UART_BUF_SIZE * 2,
        .tx_buffer_size = 0,
        .uart_queue = &uart_queue,
        .queue_size = 20,
        .reset_trigger_pin = GPIO_RST,
        .gpio0_trigger_pin = GPIO_BOOT,
    };

    if (loader_port_esp32_init(&serial_conf) == ESP_LOADER_SUCCESS) {
        ESP_LOGI(TAG, "UART & GPIO have been initialized");

        gpio_set_level(GPIO_RST, 1);
        gpio_set_level(GPIO_BOOT, 1);

        init_state_change_timer();
        sendbuf = xRingbufferCreate(ESPNOW_SEND_RINGBUFFER_SIZE, RINGBUF_TYPE_BYTEBUF);
        if (sendbuf) {
            *ringbuf = sendbuf;
            tx_done = xSemaphoreCreateBinary();
            tx_requested = xSemaphoreCreateBinary();
            xTaskCreate(uart_event_task, "uart_event_task", 8 * 1024, NULL, 5, NULL);
        } else {
            ESP_LOGE(TAG, "Cannot create ringbuffer for USB sender");
            eub_abort();
        }
        // Serial read is enabled only with the first connection. This is done in order to
        // avoid crashing the USB subsystem.
        serial_read_enabled = false;
    } else {
        ESP_LOGE(TAG, "loader_port_serial_init failed");
        eub_abort();
        return ESP_FAIL;
    }
    return ESP_OK;
}

void serial_set(const bool enable)
{
    serial_read_enabled = enable;
}

bool serial_set_baudrate(const int baud)
{
    return uart_set_baudrate(SLAVE_UART_NUM, baud) == ESP_OK;
}
