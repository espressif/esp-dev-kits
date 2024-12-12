/**
 * @file ring_buffer.c
 * @brief Ring buffer src file. Used for audio hal.
 * @version 0.1
 * @date 2021-03-07
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

#include "ring_buffer.h"

ringbuf_handle_t rb_create(uint32_t size, uint32_t malloc_caps)
{
    ringbuf_handle_t rb = NULL;
    uint8_t *buf = NULL;

	rb = heap_caps_malloc(sizeof(ringBuf), MALLOC_CAP_INTERNAL);

    if (rb == NULL) {
        return NULL;
    }

	buf = heap_caps_malloc(size, malloc_caps);

    if (NULL == buf) {
        free(rb);
        return NULL;
    }

	rb->buf = buf;
	rb->head = rb->tail = 0;
	rb->size = size;
	return rb;
}

esp_err_t rb_delete(ringbuf_handle_t rb)
{
    if (rb == NULL) {
        return ESP_FAIL;
    }

    if (NULL != rb->buf) {
        free(rb->buf);
    }

    free(rb);
    rb = NULL;
    return ESP_OK;
}

IRAM_ATTR uint32_t rb_get_count(ringbuf_handle_t rb)
{
    if (rb->head >= rb->tail) {
        return (rb->head - rb->tail);
    }

    return (rb->size - (rb->tail - rb->head));
}

IRAM_ATTR uint32_t rb_get_free(ringbuf_handle_t rb)
{
    if (rb->head >= rb->tail) {
        return rb->size -(rb->head - rb->tail) - 1;
    }

    return rb->tail - rb->head - 1;
    /*!< Free a byte to judge the ringbuffer direction */
    return (rb->size - rb_get_count(rb) - 1);
}

IRAM_ATTR esp_err_t rb_flush(ringbuf_handle_t rb)
{
    rb->tail = rb->head = 0;
    return ESP_OK;
}

IRAM_ATTR bool rb_is_empty(ringbuf_handle_t rb)
{
    return rb->tail == rb->head;
}

IRAM_ATTR bool rb_is_full(ringbuf_handle_t rb)
{
    return rb->tail == (rb->head + 1) % rb->size;
}

IRAM_ATTR esp_err_t rb_read_byte(ringbuf_handle_t rb, uint8_t *outdata)
{
    if (rb_is_empty(rb)) {
        return ESP_FAIL;
    }

    *outdata = rb->buf[rb->tail];

    rb->tail++;

    if (rb->size == rb->tail) {
        rb->tail = 0;
    }

    return ESP_OK;
}

IRAM_ATTR esp_err_t rb_write_byte(ringbuf_handle_t rb, uint8_t indata)
{
    if (rb_is_full(rb)) {
        return ESP_FAIL;
    }

    rb->buf[rb->head] = indata;
    rb->head++;
    if (rb->size == rb->head) {
        rb->head = 0;
    }

    return ESP_OK;
}
