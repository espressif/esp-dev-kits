#include <stdint.h>
#include <stddef.h>
#include <sys/queue.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pedestrian_detect.hpp"
#include "esp_err.h"
#include "linux/videodev2.h"

/**
 * @brief Camera Image Recognition (IR) buffer element node type.
 *
 * Represents a single node in the singly linked list used for managing video buffer elements.
 */
typedef SLIST_ENTRY(camera_pipeline_buffer_element) camera_pipeline_buffer_node_t;

/**
 * @brief Camera Image Recognition (IR) buffer list type.
 *
 * A singly linked list that contains all the video buffer elements.
 */
typedef SLIST_HEAD(camera_pipeline_buffer_list, camera_pipeline_buffer_element) camera_pipeline_buffer_list_t;

/**
 * @brief Camera Image Recognition (IR) configuration structure.
 *
 * Holds configuration parameters for initializing the camera IR pipeline.
 */
typedef struct {
    int elem_num;                                     /*!< Number of element available. */
    void **elements;                                  /*!< Pointer to an array of elements buffers. */
    uint32_t align_size;                              /*!< Buffer align size in byte */
    uint32_t caps;                                    /*!< Memory allocation capabilities (e.g., SPIRAM, DRAM). */
    uint32_t buffer_size;                             /*!< Size of each buffer in pixels. */
} camera_pipeline_cfg_t;

/**
 * @brief Camera Image Recognition (IR) buffer element object.
 *
 * This structure represents a video buffer element, which contains the buffer data and metadata
 * used for image recognition tasks.
 */
struct camera_pipeline_buffer_element {
    bool free;                                        /*!< Indicates if this element is currently free and available for use. */
    bool internal;                                    /*!< Indicates if this element is malloced by internal. */
    camera_pipeline_buffer_node_t node;               /*!< List node used to link elements in the buffer list. */
    uint32_t index;                                   /*!< The index of this buffer element in the list. */
    uint16_t *buffer;                                  /*!< Pointer to the buffer space used to store data. */

    uint32_t valid_size;                              /*!< Valid data size */
    std::list<dl::detect::result_t> *detect_results;   /*!< List of detection results */
};

/**
 * @brief Handle type for Camera Image Recognition (IR) pipeline.
 *
 * This type represents a handle to the camera IR pipeline, which is used to manage image recognition tasks.
 */
typedef void *pipeline_handle_t;

/**
 * @brief Create a new Camera Image Recognition (IR) pipeline.
 *
 * Initializes the camera IR pipeline with the specified configuration.
 *
 * @param cfg Configuration parameters for the pipeline.
 * @param ret_item Pointer to a variable that will receive the pipeline handle.
 *
 * @return ESP_OK on success, an error code otherwise.
 */
esp_err_t camera_element_pipeline_new(camera_pipeline_cfg_t *cfg, pipeline_handle_t *ret_item);

/**
 * @brief Delete a camera image recognition pipeline and release associated resources.
 *
 * This function frees the memory allocated for the camera image recognition pipeline,
 * including the frame buffer elements and the semaphore used for synchronization.
 * It also ensures that all resources are properly released and no other tasks
 * are accessing the pipeline during the cleanup process.
 *
 * @param[in] pipeline The handle to the camera image recognition pipeline to be deleted.
 *                     This handle is obtained from `camera_element_pipeline_new` function.
 *
 * @return
 *  - ESP_OK: Successfully deleted the pipeline and released resources.
 *  - ESP_ERR_INVALID_ARG: The provided pipeline handle is invalid.
 *
 * @note
 * - Ensure that no other tasks are accessing the pipeline before calling this function.
 * - The function assumes that `pipeline` is a valid handle obtained from `camera_element_pipeline_new`.
 * - It is the caller's responsibility to ensure that the pipeline is no longer in use before deletion.
 */
esp_err_t camera_element_pipeline_delete(pipeline_handle_t pipeline);

/**
 * @brief Queue an element index in the Camera Image Recognition (IR) pipeline.
 *
 * Queues a buffer element at the specified index for processing in the pipeline.
 *
 * @param pipline Handle to the pipeline.
 * @param index Index of the buffer element to queue.
 *
 * @return ESP_OK on success, an error code otherwise.
 */
esp_err_t camera_pipeline_queue_element_index(pipeline_handle_t pipline, int index);

/**
 * @brief Get a queued buffer element from the Camera Image Recognition (IR) pipeline.
 *
 * Retrieves the next queued buffer element for processing.
 *
 * @param pipline Handle to the pipeline.
 *
 * @return Pointer to the queued buffer element, or NULL if none are available.
 */
struct camera_pipeline_buffer_element *camera_pipeline_get_queued_element(pipeline_handle_t pipline);

/**
 * @brief Mark a buffer element as done in the Camera Image Recognition (IR) pipeline.
 *
 * Marks the specified buffer element as processed and available for reuse.
 *
 * @param pipline Handle to the pipeline.
 * @param element Pointer to the buffer element to mark as done.
 *
 * @return ESP_OK on success, an error code otherwise.
 */
esp_err_t IRAM_ATTR camera_pipeline_done_element(pipeline_handle_t pipline, struct camera_pipeline_buffer_element *element);

/**
 * @brief Get a processed buffer element from the Camera Image Recognition (IR) pipeline.
 *
 * Retrieves a buffer element that has been processed and marked as done.
 *
 * @param pipline Handle to the pipeline.
 *
 * @return Pointer to the processed buffer element, or NULL if none are available.
 */
struct camera_pipeline_buffer_element *camera_pipeline_get_done_element(pipeline_handle_t pipline);

/**
 * @brief Receive a buffer element from the Camera Image Recognition (IR) pipeline.
 *
 * Blocks until a buffer element is available or the specified timeout period expires.
 *
 * @param pipline Handle to the pipeline.
 * @param ticks Timeout period (in ticks) to wait for a buffer element.
 *
 * @return Pointer to the received buffer element, or NULL if the timeout expires.
 */
struct camera_pipeline_buffer_element *camera_pipeline_recv_element(pipeline_handle_t pipline, uint32_t ticks);
