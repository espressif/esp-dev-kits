/**
 * @file ring_buffer.h
 * @brief Ring buffer header file. Used for audio hal.
 * @version 0.1
 * @date 2021-02-28
 * 
 * @copyright Copyright 2021 Espressif Systems (Shanghai) Co. Ltd.
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *               http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "esp_attr.h"
#include "esp_heap_caps.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t *buf;   /*!< Data buffer    */
    uint32_t head;  /*!< Write pointer  */
    uint32_t tail;  /*!< Read pointer   */
    uint32_t size;  /*!< Buffer size    */
} ringBuf;
typedef ringBuf *ringbuf_handle_t;

/**
 * @brief Create ring buffer with size and malloc caps.
 * 
 * @param size Size of ring buffer.
 * @param malloc_caps Malloca caps of ring buffer.
 * @return Handle of ring buffer. NULL if failed.
 */
ringbuf_handle_t rb_create(uint32_t size, uint32_t malloc_caps);

/**
 * @brief Delete ring buffer and free the memories.
 * 
 * @param rb handle of ring buffer.
 * @return ESP_OK on success, otherwise see esp_err_t
 */
esp_err_t rb_delete(ringbuf_handle_t rb);

/**
 * @brief Get used bytes of ring buffer.
 * 
 * @param rb Handle of ring buffer.
 * @return  Bytes exists in ring buffer. 
 */
uint32_t rb_get_count(ringbuf_handle_t rb);

/**
 * @brief Get free space of ring buffer.
 * 
 * @param rb Handle of ring buffer.
 * @return Free space of ring buffer. 
 */
uint32_t rb_get_free(ringbuf_handle_t rb);

/**
 * @brief Clear ring buffer.
 * 
 * @param rb Handle of ring buffer.
 * @return ESP_OK on success, otherwise see esp_err_t.
 */
esp_err_t rb_flush(ringbuf_handle_t rb);

/**
 * @brief Read a byte to ring buffer
 * 
 * @param rb Handle of ring buffer.
 * @param outdata Address to save the byte read from ring buffer.
 * @return ESP_OK on success, otherwise see esp_err_t.
 */
esp_err_t rb_read_byte(ringbuf_handle_t rb, uint8_t *outdata);

/**
 * @brief Write a byte to ring buffer
 * 
 * @param rb Handle of ring buffer.
 * @param indata Data written to ring buffer.
 * @return ESP_OK on success, otherwise see esp_err_t.
 */
esp_err_t rb_write_byte(ringbuf_handle_t rb, uint8_t indata);

#ifdef __cplusplus
extern "C" {
#endif
