/* SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "app_mode.h"

#define SLAVE_UART_BUF_SIZE     (2 * 1024)
#define SLAVE_UART_DEFAULT_BAUD 115200
#define SLAVE_UART_NUM          UART_NUM_1

#define USB_SEND_RINGBUFFER_SIZE SLAVE_UART_BUF_SIZE
#define ESPNOW_SEND_RINGBUFFER_SIZE SLAVE_UART_BUF_SIZE

/**
 * @brief Sets the boot and reset pins of the connected chip
 * 
 * @param dtr The Data Terminal Ready pin
 * @param rts The Request To Send pin
 */
void serial_boot_reset(const bool dtr, const bool rts);

/*
 * @brief Set the serial data to be readable or not
 * 
 * @param enable Boolean value to set the serial data to be readable or not 
 */
void serial_set(const bool enable);

/**
 * @brief Set the baud rate of the serial port. 
 * 
 * @param baud The desired baud rate. 
 * @return true If the baud rate was successfully set. 
 *         false If the baud rate could not be set. 
 */
bool serial_set_baudrate(const int baud);

/**
 * @brief Initialize the wired serial mode. 
 * 
 * @param ringbuf A pointer to a RingbufHandle_t object. 
 * @return esp_err_t ESP_OK if successful, otherwise an error code. 
 */
esp_err_t mode_wired_serial_init(RingbufHandle_t *ringbuf);

/**
 * @brief Initialize the wireless host serial mode. 
 * 
 * @param ringbuf A pointer to a RingbufHandle_t object. 
 * @return esp_err_t ESP_OK if successful, otherwise an error code. 
 */
esp_err_t mode_wireless_host_serial_init(RingbufHandle_t *ringbuf);

/**
 * @brief Initialize the wireless slave serial mode. 
 * 
 * @param ringbuf A pointer to a RingbufHandle_t object. 
 * @return esp_err_t ESP_OK if successful, otherwise an error code. 
 */
esp_err_t mode_wireless_slave_serial_init(RingbufHandle_t *ringbuf);