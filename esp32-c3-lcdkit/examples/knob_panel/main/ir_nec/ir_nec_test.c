/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/rmt_tx.h"
#include "driver/rmt_rx.h"
#include "ir_nec_encoder.h"

#include "esp_wifi.h"

#define IR_TEST_PROBE_ADDR      0xA511
#define IR_TEST_RESP_ADDR       0xA522

#define NEC_IR_RESOLUTION_HZ     1000000 // 1MHz resolution, 1 tick = 1us
#define NEC_IR_TX_GPIO_NUM       4
#define NEC_IR_RX_GPIO_NUM       4
#define NEC_IR_NEC_DECODE_MARGIN 200     // Tolerance for parsing RMT symbols into bit stream

/**
 * @brief NEC timing spec
 */
#define NEC_LEADING_CODE_DURATION_0  9000
#define NEC_LEADING_CODE_DURATION_1  4500
#define NEC_PAYLOAD_ZERO_DURATION_0  560
#define NEC_PAYLOAD_ZERO_DURATION_1  560
#define NEC_PAYLOAD_ONE_DURATION_0   560
#define NEC_PAYLOAD_ONE_DURATION_1   1690
#define NEC_REPEAT_CODE_DURATION_0   9000
#define NEC_REPEAT_CODE_DURATION_1   2250

static const char *TAG = "IR";

/**
 * @brief Saving NEC decode results
 */
static uint16_t s_nec_code_address;
static uint16_t s_nec_code_command;
static uint16_t s_nec_test_id;
static bool s_nec_test_result = false;
/**
 * @brief Check whether a duration is within expected range
 */
static inline bool nec_check_in_range(uint32_t signal_duration, uint32_t spec_duration)
{
    return (signal_duration < (spec_duration + NEC_IR_NEC_DECODE_MARGIN)) &&
           (signal_duration > (spec_duration - NEC_IR_NEC_DECODE_MARGIN));
}

/**
 * @brief Check whether a RMT symbol represents NEC logic zero
 */
static bool nec_parse_logic0(rmt_symbol_word_t *rmt_nec_symbols)
{
    return nec_check_in_range(rmt_nec_symbols->duration0, NEC_PAYLOAD_ZERO_DURATION_0) &&
           nec_check_in_range(rmt_nec_symbols->duration1, NEC_PAYLOAD_ZERO_DURATION_1);
}

/**
 * @brief Check whether a RMT symbol represents NEC logic one
 */
static bool nec_parse_logic1(rmt_symbol_word_t *rmt_nec_symbols)
{
    return nec_check_in_range(rmt_nec_symbols->duration0, NEC_PAYLOAD_ONE_DURATION_0) &&
           nec_check_in_range(rmt_nec_symbols->duration1, NEC_PAYLOAD_ONE_DURATION_1);
}

/**
 * @brief Decode RMT symbols into NEC address and command
 */
static bool nec_parse_frame(rmt_symbol_word_t *rmt_nec_symbols)
{
    rmt_symbol_word_t *cur = rmt_nec_symbols;
    uint16_t address = 0;
    uint16_t command = 0;
    bool valid_leading_code = nec_check_in_range(cur->duration0, NEC_LEADING_CODE_DURATION_0) &&
                              nec_check_in_range(cur->duration1, NEC_LEADING_CODE_DURATION_1);
    if (!valid_leading_code) {
        return false;
    }
    cur++;
    for (int i = 0; i < 16; i++) {
        if (nec_parse_logic1(cur)) {
            address |= 1 << i;
        } else if (nec_parse_logic0(cur)) {
            address &= ~(1 << i);
        } else {
            return false;
        }
        cur++;
    }
    for (int i = 0; i < 16; i++) {
        if (nec_parse_logic1(cur)) {
            command |= 1 << i;
        } else if (nec_parse_logic0(cur)) {
            command &= ~(1 << i);
        } else {
            return false;
        }
        cur++;
    }
    // save address and command
    s_nec_code_address = address;
    s_nec_code_command = command;
    return true;
}

/**
 * @brief Check whether the RMT symbols represent NEC repeat code
 */
static bool nec_parse_frame_repeat(rmt_symbol_word_t *rmt_nec_symbols)
{
    return nec_check_in_range(rmt_nec_symbols->duration0, NEC_REPEAT_CODE_DURATION_0) &&
           nec_check_in_range(rmt_nec_symbols->duration1, NEC_REPEAT_CODE_DURATION_1);
}

/**
 * @brief Decode RMT symbols into NEC scan code and print the result
 */
static void nec_parse_nec_frame(rmt_symbol_word_t *rmt_nec_symbols, size_t symbol_num)
{
    // decode RMT symbols
    switch (symbol_num) {
    case 34: // NEC normal frame
        if (nec_parse_frame(rmt_nec_symbols)) {

            printf("Address=%04X, Command=%04X, %04X.\r\n", \
                   s_nec_code_address, s_nec_code_command, s_nec_test_id);

            if (IR_TEST_RESP_ADDR == s_nec_code_address) {
                if (s_nec_code_command == s_nec_test_id) {
                    printf("@sucesfully, %04X\r\n", s_nec_code_command);
                    s_nec_test_result = true;
                }
            }
        }
        break;
    case 2: // NEC repeat frame
        if (nec_parse_frame_repeat(rmt_nec_symbols)) {
            printf("Address=%04X, Command=%04X, repeat\r\n", s_nec_code_address, s_nec_code_command);
        }
        break;
    default:
        printf("Unknown NEC frame\r\n");
        break;
    }
}

static bool nec_rmt_rx_done_callback(rmt_channel_handle_t channel, const rmt_rx_done_event_data_t *edata, void *user_data)
{
    BaseType_t high_task_wakeup = pdFALSE;
    QueueHandle_t receive_queue = (QueueHandle_t)user_data;
    // send the received RMT symbols to the parser task
    xQueueSendFromISR(receive_queue, edata, &high_task_wakeup);
    return high_task_wakeup == pdTRUE;
}

void nec_test_task(void *arg)
{
    static uint8_t test_loop = 0;

    while (1) {
        test_loop++;
        if (test_loop % 2) {
            ESP_LOGI(TAG, "create RMT RX channel");
            rmt_rx_channel_config_t rx_channel_cfg = {
                .clk_src = RMT_CLK_SRC_DEFAULT,
                .resolution_hz = NEC_IR_RESOLUTION_HZ,
                .mem_block_symbols = 64, // amount of RMT symbols that the channel can store at a time
                .gpio_num = NEC_IR_RX_GPIO_NUM,
            };
            rmt_channel_handle_t rx_channel = NULL;
            ESP_ERROR_CHECK(rmt_new_rx_channel(&rx_channel_cfg, &rx_channel));

            QueueHandle_t receive_queue = xQueueCreate(1, sizeof(rmt_rx_done_event_data_t));
            assert(receive_queue);
            rmt_rx_event_callbacks_t cbs = {
                .on_recv_done = nec_rmt_rx_done_callback,
            };
            ESP_ERROR_CHECK(rmt_rx_register_event_callbacks(rx_channel, &cbs, receive_queue));

            // the following timing requirement is based on NEC protocol
            rmt_receive_config_t receive_config = {
                .signal_range_min_ns = 1250,     // the shortest duration for NEC signal is 560us, 1250ns < 560us, valid signal won't be treated as noise
                .signal_range_max_ns = 12000000, // the longest duration for NEC signal is 9000us, 12000000ns > 9000us, the receive won't stop early
            };
            ESP_ERROR_CHECK(rmt_enable(rx_channel));

            // save the received RMT symbols
            rmt_symbol_word_t raw_symbols[64]; // 64 symbols should be sufficient for a standard NEC frame
            rmt_rx_done_event_data_t rx_data;
            // ready to receive
            ESP_ERROR_CHECK(rmt_receive(rx_channel, raw_symbols, sizeof(raw_symbols), &receive_config));

            uint8_t recv_cn = 0;
            do {
                // wait for RX done signal
                if (xQueueReceive(receive_queue, &rx_data, pdMS_TO_TICKS(500)) == pdPASS) {
                    // parse the receive symbols and print the result
                    nec_parse_nec_frame(rx_data.received_symbols, rx_data.num_symbols);
                    // start receive again
                    ESP_ERROR_CHECK(rmt_receive(rx_channel, raw_symbols, sizeof(raw_symbols), &receive_config));
                } else {
                    recv_cn++;
                }
            } while (recv_cn < 3);

            rmt_disable(rx_channel);
            rmt_del_channel(rx_channel);

        } else {
            ESP_LOGI(TAG, "create RMT TX channel");
            rmt_tx_channel_config_t tx_channel_cfg = {
                .clk_src = RMT_CLK_SRC_DEFAULT,
                .resolution_hz = NEC_IR_RESOLUTION_HZ,
                .mem_block_symbols = 64, // amount of RMT symbols that the channel can store at a time
                .trans_queue_depth = 4,  // number of transactions that allowed to pending in the background, this example won't queue multiple transactions, so queue depth > 1 is sufficient
                .gpio_num = NEC_IR_TX_GPIO_NUM,
            };
            rmt_channel_handle_t tx_channel = NULL;
            ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_channel_cfg, &tx_channel));

            rmt_carrier_config_t carrier_cfg = {
                .duty_cycle = 0.33,
                .frequency_hz = 38000, // 38KHz
            };
            ESP_ERROR_CHECK(rmt_apply_carrier(tx_channel, &carrier_cfg));

            // this example won't send NEC frames in a loop
            rmt_transmit_config_t transmit_config = {
                .loop_count = 0, // no loop
            };

            ir_nec_encoder_config_t nec_encoder_cfg = {
                .resolution = NEC_IR_RESOLUTION_HZ,
            };
            rmt_encoder_handle_t nec_encoder = NULL;
            ESP_ERROR_CHECK(rmt_new_ir_nec_encoder(&nec_encoder_cfg, &nec_encoder));

            ESP_ERROR_CHECK(rmt_enable(tx_channel));

            // timeout, transmit predefined IR NEC packets
            ir_nec_scan_code_t scan_code = {
                .address = IR_TEST_PROBE_ADDR,
            };

            uint8_t send_cn = 0;
            scan_code.command = s_nec_test_id;

            do {
                ESP_LOGI(TAG, "RMT TX:%d, %04X", send_cn, s_nec_test_id);
                ESP_ERROR_CHECK(rmt_transmit(tx_channel, nec_encoder, &scan_code, sizeof(scan_code), &transmit_config));
                send_cn++;
                vTaskDelay(pdMS_TO_TICKS(500));
            } while (send_cn < 3);

            rmt_disable(tx_channel);
            rmt_del_channel(tx_channel);
        }
    }
}

bool nec_test_result()
{
    return s_nec_test_result;
}

esp_err_t nec_test_start()
{
    uint8_t eth_mac[6];
    esp_wifi_get_mac(WIFI_IF_STA, eth_mac);
    s_nec_test_id = (eth_mac[4] << 8) | (eth_mac[5] << 0);

    BaseType_t ret_val = xTaskCreatePinnedToCore(nec_test_task, "nec Task", 4 * 1024, NULL, 5, NULL, 0);
    ESP_ERROR_CHECK_WITHOUT_ABORT((pdPASS == ret_val) ? ESP_OK : ESP_FAIL);

    return ESP_OK;
}
