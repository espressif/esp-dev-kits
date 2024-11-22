#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/errno.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_check.h"
#include "linux/videodev2.h"
#include "esp_video_init.h"

#include "app_video.h"
#include "app_camera_pipeline.hpp"

#define ELEMENT_GET_BY_INDEX(vb, i)         (&(vb)->element[i])
#define ELEMENT_SET_FREE(e)                 { (e)->free = true; }
#define ELEMENT_SET_ALLOCATED(e)            { (e)->free = false; }
#define ELEMENT_IS_FREE(e)                  ((e)->free == true)

static const char *TAG = "app_camera_pipeline";

struct camera_pipeline_stream {
    bool started;                           /*!< Indicates whether the video stream has been started. */
    int elem_num;                           /*!< The number of element available for the stream. */

    camera_pipeline_buffer_list_t queued_list; /*!< List of buffer elements that are currently queued for processing. */
    camera_pipeline_buffer_list_t done_list;   /*!< List of buffer elements that have been processed and are done. */

    struct camera_pipeline_buffer_element *element; /*!< Pointer to the array of buffer elements used for storing image data. */

    portMUX_TYPE stream_lock;              /*!< Mutex used for synchronizing access to the video stream's data structures. */
    SemaphoreHandle_t ready_sem;           /*!< Semaphore used for signaling when buffer elements are ready for processing. */
};

esp_err_t camera_element_pipeline_new(camera_pipeline_cfg_t *cfg, pipeline_handle_t *ret_item)
{
    esp_err_t ret = ESP_OK;
    struct camera_pipeline_stream *stream = NULL;

    ESP_GOTO_ON_FALSE(cfg && cfg->elem_num > 0, ESP_ERR_INVALID_ARG, err, TAG, "Invalid configuration: elem_num must be greater than 0.");

    // stream = heap_caps_calloc(1, sizeof(struct camera_pipeline_stream), cfg->caps);
    stream = static_cast<camera_pipeline_stream*>(heap_caps_calloc(1, sizeof(camera_pipeline_stream), cfg->caps));
    ESP_GOTO_ON_FALSE(stream, ESP_ERR_NO_MEM, err, TAG, "Failed to allocate memory for camera_pipeline_stream.");
    memset(stream, 0, sizeof(struct camera_pipeline_stream));

    // stream->element = heap_caps_calloc(cfg->elem_num, sizeof(struct camera_pipeline_buffer_element), cfg->caps);
    stream->element = static_cast<camera_pipeline_buffer_element*>(
        heap_caps_calloc(cfg->elem_num, sizeof(camera_pipeline_buffer_element), cfg->caps)
    );
    ESP_GOTO_ON_FALSE(stream->element, ESP_ERR_NO_MEM, err, TAG, "Failed to allocate memory for camera_pipeline_buffer_element.");

    SLIST_INIT(&stream->queued_list);
    SLIST_INIT(&stream->done_list);

    portMUX_INITIALIZE(&stream->stream_lock);

    stream->ready_sem = xSemaphoreCreateCounting(cfg->elem_num, 0);
    ESP_GOTO_ON_FALSE(stream->ready_sem, ESP_ERR_NO_MEM, err, TAG, "Failed to create done_sem for stream");

    for (int i = 0; i < cfg->elem_num; i++) {
        struct camera_pipeline_buffer_element *element = &stream->element[i];

        if (!cfg->elements || !cfg->elements[i]) {
            uint16_t* elements = static_cast<uint16_t*>(
                heap_caps_aligned_calloc(cfg->align_size, 1, cfg->buffer_size, cfg->caps)
            );
            ESP_GOTO_ON_FALSE(elements, ESP_ERR_NO_MEM, err, TAG, "Failed to allocate memory for elements buffer %d.", i);
            element->buffer = elements;
            element->internal = true;
        } else {
            element->buffer = static_cast<uint16_t*>(cfg->elements[i]);
            element->internal = false;
        }

        element->index = i;
        element->valid_size = cfg->buffer_size;
        ELEMENT_SET_FREE(element);
        camera_pipeline_queue_element_index(stream, i);
        stream->elem_num++;
        ESP_LOGI(TAG, "new elements[%d]:%p, internal:%d", i, element->buffer, element->internal);
    }
    ESP_LOGI(TAG, "new pipeline %p, elem_num:%d", stream, stream->elem_num);

    *ret_item = (pipeline_handle_t)stream;
    return ESP_OK;

err:
    for (int i = 0; i < stream->elem_num; i++) {
        if (stream->element[i].internal) {
            free(stream->element[i].buffer);
        }
    }

    if (stream->ready_sem) {
        vSemaphoreDelete(stream->ready_sem);
    }
    portEXIT_CRITICAL(&stream->stream_lock);

    if (stream->element) {
        free(stream->element);
    }
    if (stream) {
        free(stream);
    }
    return ret;
}

esp_err_t camera_element_pipeline_delete(pipeline_handle_t pipeline)
{
    struct camera_pipeline_stream *stream = (struct camera_pipeline_stream *)pipeline;
    ESP_RETURN_ON_FALSE(stream, ESP_ERR_INVALID_ARG, TAG, "Invalid pipeline handle");

    for (int i = 0; i < stream->elem_num; i++) {
        if (stream->element[i].buffer && stream->element[i].internal) {
            free(stream->element[i].buffer);
        }
    }

    if (stream->ready_sem) {
        vSemaphoreDelete(stream->ready_sem);
    }
    portEXIT_CRITICAL(&stream->stream_lock);

    free(stream->element);
    free(stream);

    return ESP_OK;
}

esp_err_t camera_pipeline_queue_element(pipeline_handle_t pipline, struct camera_pipeline_buffer_element *element)
{
    struct camera_pipeline_stream *stream = (struct camera_pipeline_stream *)pipline;
    if (!stream) {
        return ESP_ERR_INVALID_ARG;
    }

    portENTER_CRITICAL_SAFE(&stream->stream_lock);
    if (!ELEMENT_IS_FREE(element)) {
        portEXIT_CRITICAL_SAFE(&stream->stream_lock);
        return ESP_ERR_INVALID_ARG;
    }

    ELEMENT_SET_ALLOCATED(element);
    SLIST_INSERT_HEAD(&stream->queued_list, element, node);
    portEXIT_CRITICAL_SAFE(&stream->stream_lock);

    return ESP_OK;
}

esp_err_t camera_pipeline_queue_element_index(pipeline_handle_t pipline, int index)
{
    esp_err_t ret;
    struct camera_pipeline_buffer_element *element;

    struct camera_pipeline_stream *stream = (struct camera_pipeline_stream *)pipline;
    if (!stream) {
        return ESP_ERR_INVALID_ARG;
    }

    element = ELEMENT_GET_BY_INDEX(stream, index);
    ret = camera_pipeline_queue_element(stream, element);

    return ret;
}

struct camera_pipeline_buffer_element *camera_pipeline_get_queued_element(pipeline_handle_t pipline)
{
    struct camera_pipeline_buffer_element *element = NULL;

    struct camera_pipeline_stream *stream = (struct camera_pipeline_stream *)pipline;
    if (!stream) {
        return NULL;
    }

    portENTER_CRITICAL_SAFE(&stream->stream_lock);
    if (!SLIST_EMPTY(&stream->queued_list)) {
        element = SLIST_FIRST(&stream->queued_list);
        SLIST_REMOVE(&stream->queued_list, element, camera_pipeline_buffer_element, node);
        ELEMENT_SET_FREE(element);
    }
    portEXIT_CRITICAL_SAFE(&stream->stream_lock);

    return element;
}

struct camera_pipeline_buffer_element *camera_pipeline_get_done_element(pipeline_handle_t pipline)
{
    struct camera_pipeline_buffer_element *element = NULL;

    struct camera_pipeline_stream *stream = (struct camera_pipeline_stream *)pipline;
    if (!stream) {
        return NULL;
    }

    portENTER_CRITICAL_SAFE(&stream->stream_lock);
    if (!SLIST_EMPTY(&stream->done_list)) {
        element = SLIST_FIRST(&stream->done_list);
        SLIST_REMOVE(&stream->done_list, element, camera_pipeline_buffer_element, node);
        ELEMENT_SET_FREE(element);
    }
    portEXIT_CRITICAL_SAFE(&stream->stream_lock);

    return element;
}

esp_err_t IRAM_ATTR camera_pipeline_done_element(pipeline_handle_t pipline, struct camera_pipeline_buffer_element *element)
{
    struct camera_pipeline_stream *stream = (struct camera_pipeline_stream *)pipline;
    if (!stream) {
        return ESP_ERR_INVALID_ARG;
    }

    portENTER_CRITICAL_SAFE(&stream->stream_lock);
    if (!ELEMENT_IS_FREE(element)) {
        portEXIT_CRITICAL_SAFE(&stream->stream_lock);
        return ESP_ERR_INVALID_ARG;
    }

    ELEMENT_SET_ALLOCATED(element);
    SLIST_INSERT_HEAD(&stream->done_list, element, node);
    portEXIT_CRITICAL_SAFE(&stream->stream_lock);

    if (xPortInIsrContext()) {
        BaseType_t wakeup = pdFALSE;

        xSemaphoreGiveFromISR(stream->ready_sem, &wakeup);
        if (wakeup == pdTRUE) {
            portYIELD_FROM_ISR();
        }
    } else {
        xSemaphoreGive(stream->ready_sem);
    }

    return ESP_OK;
}

struct camera_pipeline_buffer_element *camera_pipeline_recv_element(pipeline_handle_t pipline, uint32_t ticks)
{
    BaseType_t ret;
    struct camera_pipeline_buffer_element *element;

    struct camera_pipeline_stream *stream = (struct camera_pipeline_stream *)pipline;
    if (!stream) {
        return NULL;
    }

    ret = xSemaphoreTake(stream->ready_sem, (TickType_t)ticks);
    if (ret != pdTRUE) {
        return NULL;
    }

    element = camera_pipeline_get_done_element(pipline);

    return element;
}
