#include <vector>
#include <string.h>
#include <stdio.h>

#include "bsp/esp-bsp.h"
#include "esp_log.h"
#include "esp_private/esp_cache_private.h"

#include "app_pedestrian_detect.h"
#include "app_humanface_detect.h"
#include "app_coco_detect.h"

#include "app_camera_pipeline.hpp"

#include "app_drawing_utils.h"

#include "app_ai_detect.h"

#include "esp_painter.h"

#include "ui_extra.h"

static const char *TAG = "app_ai_detect";

#define DETECT_HEIGHT    240
#define DETECT_WIDTH     240
#define AI_BUFFER_COUNT  5            // Number of AI detection buffers

#define ALIGN_UP(num, align)    (((num) + ((align) - 1)) & ~((align) - 1))

static esp_painter_handle_t painter = NULL;

static pipeline_handle_t feed_pipeline;
static pipeline_handle_t detect_pipeline;

static TaskHandle_t detect_task_handle = NULL;

static PedestrianDetect *ped_detect = NULL;
static HumanFaceDetect *hum_detect = NULL;
static COCODetect *coco_od_detect = NULL;

static std::list<dl::detect::result_t> detect_results;

/**
 * @brief Structure for managing AI detection buffers
 */
typedef struct {
    void *ai_buffers[AI_BUFFER_COUNT];        // Buffers for AI detection
    size_t ai_buffer_size;                    // Size of each AI buffer
    int current_ai_buffer_index;              // Index of the current buffer being used
    bool ai_buffers_initialized;              // Flag to check if buffers are initialized
} ai_detection_buffers_t;

// Static instance of AI detection buffers
static ai_detection_buffers_t ai_buffers = {
    .ai_buffers = {nullptr},              // Initialize all buffer pointers to null
    .ai_buffer_size = 0,                  // Initialize buffer size to 0
    .current_ai_buffer_index = 0,         // Initialize current index to 0
    .ai_buffers_initialized = false       // Initialize flag to false
};

void camera_dectect_task(void);

esp_err_t app_ai_detect_init(void)
{
    ESP_LOGI(TAG, "Initialize the AI detect");
    ped_detect = get_pedestrian_detect();
    assert(ped_detect != NULL);
    
    hum_detect = get_humanface_detect();
    assert(hum_detect != NULL);

    coco_od_detect = get_coco_detect();
    assert(coco_od_detect != NULL);

    // Initialize esp_painter
    esp_painter_config_t painter_config = {
        .canvas = {
            .width = BSP_LCD_H_RES,
            .height = BSP_LCD_V_RES
        },
        .color_format = ESP_PAINTER_COLOR_FORMAT_RGB565,
        .default_font = &esp_painter_basic_font_20,
        .swap_rgb565 = true
        
    };
    ESP_ERROR_CHECK(esp_painter_init(&painter_config, &painter));

    camera_pipeline_cfg_t feed_cfg = {
        .elem_num = 5,
        .elements = NULL,
        .align_size = 1,
        .caps = MALLOC_CAP_SPIRAM,
        .buffer_size = DETECT_WIDTH * DETECT_HEIGHT / 8,
    };

    camera_element_pipeline_new(&feed_cfg, &feed_pipeline);

    camera_pipeline_cfg_t detect_cfg = {
        .elem_num = 5,
        .elements = NULL,
        .align_size = 1,
        .caps = MALLOC_CAP_SPIRAM,
        .buffer_size = 20 * sizeof(int),
    };
    camera_element_pipeline_new(&detect_cfg, &detect_pipeline);

    xTaskCreatePinnedToCore((TaskFunction_t)camera_dectect_task, "Camera Detect", 1024 * 8, NULL, 5, &detect_task_handle, 1);

    return ESP_OK;
}

/**
 * @brief Initialize AI detection buffers
 * 
 * @param cache_line_size Size of cache line for alignment
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t app_ai_detection_init_buffers(size_t cache_line_size)
{
    esp_err_t ret = ESP_OK;
    
    // Calculate buffer size with alignment
    ai_buffers.ai_buffer_size = ALIGN_UP(BSP_LCD_H_RES * BSP_LCD_V_RES * 2, cache_line_size);
    
    // Allocate buffers for AI detection
    for (int i = 0; i < AI_BUFFER_COUNT; i++) {
        ai_buffers.ai_buffers[i] = heap_caps_aligned_calloc(
            cache_line_size, 1,
            ai_buffers.ai_buffer_size,
            MALLOC_CAP_SPIRAM
        );
        
        if (ai_buffers.ai_buffers[i] == NULL) {
            ESP_LOGE(TAG, "Failed to allocate AI detection buffer %d", i);
            
            // Clean up already allocated buffers
            for (int j = 0; j < i; j++) {
                if (ai_buffers.ai_buffers[j]) {
                    heap_caps_free(ai_buffers.ai_buffers[j]);
                    ai_buffers.ai_buffers[j] = NULL;
                }
            }
            
            return ESP_ERR_NO_MEM;
        }
    }
    
    ai_buffers.ai_buffers_initialized = true;
    ai_buffers.current_ai_buffer_index = 0;
    
    ESP_LOGI(TAG, "AI detection buffers initialized successfully");
    return ret;
}

/**
 * @brief Process frame for AI detection
 * 
 * @param detect_buf Buffer containing the frame to detect
 * @param width Frame width
 * @param height Frame height
 * @param ai_detect_mode Current AI detection mode
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t app_ai_detection_process_frame(uint8_t *detect_buf, uint32_t width, uint32_t height, int ai_detect_mode)
{
    if (!ai_buffers.ai_buffers_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    esp_err_t ret = ESP_OK;
    
    // Get current AI buffer
    void *current_ai_buffer = ai_buffers.ai_buffers[ai_buffers.current_ai_buffer_index];
    
    // First make a copy of the camera buffer as it may be released after this function returns
    memcpy(current_ai_buffer, detect_buf, width * height * 2);
    
    // Submit buffer for AI detection
    if(ai_detect_mode == AI_DETECT_FACE) {
        ret = app_humanface_ai_detect((uint16_t*)current_ai_buffer, (uint16_t*)detect_buf, width, height);
        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "Human face detection failed: 0x%x", ret);
        }
    } else if(ai_detect_mode == AI_DETECT_PEDESTRIAN) {
        ret = app_pedestrian_ai_detect((uint16_t*)current_ai_buffer, (uint16_t*)detect_buf, width, height);
        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "Pedestrian detection failed: 0x%x", ret);
        }
    }
    
    // Move to next buffer in circular fashion
    ai_buffers.current_ai_buffer_index = (ai_buffers.current_ai_buffer_index + 1) % AI_BUFFER_COUNT;
    
    return ret;
}

/**
 * @brief Free AI detection buffers and resources
 * 
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t app_ai_detection_deinit(void)
{
    if (!ai_buffers.ai_buffers_initialized) {
        return ESP_OK; // Nothing to clean up
    }
    
    // Free all allocated buffers
    for (int i = 0; i < AI_BUFFER_COUNT; i++) {
        if (ai_buffers.ai_buffers[i] != NULL) {
            heap_caps_free(ai_buffers.ai_buffers[i]);
            ai_buffers.ai_buffers[i] = NULL;
        }
    }
    
    ai_buffers.ai_buffers_initialized = false;
    
    ESP_LOGI(TAG, "AI detection buffers freed successfully");
    return ESP_OK;
}

void camera_dectect_task(void)
{
    while (1) {        
        camera_pipeline_buffer_element *p = camera_pipeline_recv_element(feed_pipeline, portMAX_DELAY);
        if (p && ui_extra_get_current_page() == UI_PAGE_AI_DETECT && ui_extra_is_ui_init()) {
            if (ui_extra_get_ai_detect_mode() == AI_DETECT_PEDESTRIAN) {
                detect_results = app_pedestrian_detect((uint16_t *)p->buffer, DETECT_WIDTH, DETECT_HEIGHT);
            } else if (ui_extra_get_ai_detect_mode() == AI_DETECT_FACE) {
                detect_results = app_humanface_detect((uint16_t *)p->buffer, DETECT_WIDTH, DETECT_HEIGHT);
            }

            camera_pipeline_queue_element_index(feed_pipeline, p->index);

            camera_pipeline_buffer_element *element = camera_pipeline_get_queued_element(detect_pipeline);
            if (element) {
                element->detect_results = &detect_results;

                camera_pipeline_done_element(detect_pipeline, element);
            }
        } else {
            vTaskDelay(pdMS_TO_TICKS(50));
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

esp_err_t app_coco_od_detect(uint16_t *data, int width, int height)
{
    ESP_LOGI(TAG, "Detecting COCO objects");
    detect_results = app_coco_detect(data, width, height);
    if (detect_results.size() > 0) {
        uint16_t *rgb_buf = data;
        for (const auto& res : detect_results) {
            const auto& box = res.box;
            
            // Get confidence score
            float score = res.score;
            
            // Only process detections with confidence higher than 30%
            if (score <= 0.30f) {
                continue;  // Skip this detection if confidence is too low
            }
            
            // Check if bounding box is valid
            if (box.size() >= 4 && std::any_of(box.begin(), box.end(), [](int v) { return v != 0; })) {
                draw_rectangle_rgb(rgb_buf, width, height,
                                box[0], box[1], box[2], box[3],
                                0, 0, 255, 0, 0, 5, true);

                // Display COCO detection class name with confidence score
                int category = res.category;
                
                const char* class_name = get_coco_class_name(category);
                char label[64];
                snprintf(label, sizeof(label), "%s", class_name);
                
                // Ensure text is displayed inside the bounding box at the top-left corner
                int text_x = box[0] + 5;  // 5 pixels offset from left edge
                int text_y = box[1] + 15; // 15 pixels offset from top edge
                
                // Use esp_painter to draw text
                esp_painter_draw_string(painter, (uint8_t*)rgb_buf, 
                                        width * height * 2,
                                        text_x, text_y, NULL, 
                                        ESP_PAINTER_COLOR_YELLOW, 
                                        label);
            }
        }
    }
    return ESP_OK;
}

esp_err_t app_humanface_ai_detect(uint16_t *detect_buf, uint16_t *draw_buf, int width, int height)
{
    // Process input frame
    camera_pipeline_buffer_element *input_element = camera_pipeline_get_queued_element(feed_pipeline);
    if (input_element) {
        input_element->buffer = reinterpret_cast<uint16_t*>(detect_buf);
        camera_pipeline_done_element(feed_pipeline, input_element);
    }

    // Get detection results
    camera_pipeline_buffer_element *detect_element = camera_pipeline_recv_element(detect_pipeline, 0);
    if (detect_element && detect_element->detect_results) {
        uint16_t *rgb_buf = draw_buf;
        
        // Add safety check for detect_results pointer
        std::list<dl::detect::result_t> *results = detect_element->detect_results;
        if (results && !results->empty()) {
            for (const auto& res : *results) {
                const auto& box = res.box;
                // Add additional safety checks
                if (box.size() >= 4 && std::any_of(box.begin(), box.end(), [](int v) { return v != 0; })) {
                    draw_rectangle_rgb(rgb_buf, width, height,
                                    box[0], box[1], box[2], box[3],
                                    0, 0, 255, 0, 0, 5, false);

                    if(res.keypoint.size() >= 10 && 
                            std::any_of(res.keypoint.begin(), res.keypoint.end(), [](int v) { return v != 0; })) {
                        draw_green_points(rgb_buf, res.keypoint, false);
                    }
                }
            }
        }

        camera_pipeline_queue_element_index(detect_pipeline, detect_element->index);
    }

    return ESP_OK;
}

esp_err_t app_pedestrian_ai_detect(uint16_t *detect_buf, uint16_t *draw_buf, int width, int height)
{
    // Process input frame
    camera_pipeline_buffer_element *input_element = camera_pipeline_get_queued_element(feed_pipeline);
    if (input_element) {
        input_element->buffer = reinterpret_cast<uint16_t*>(detect_buf);
        camera_pipeline_done_element(feed_pipeline, input_element);
    }

    // Get detection results
    camera_pipeline_buffer_element *detect_element = camera_pipeline_recv_element(detect_pipeline, 0);
    if (detect_element && detect_element->detect_results) {
        uint16_t *rgb_buf = draw_buf;
        
        // Add safety check for detect_results pointer
        std::list<dl::detect::result_t> *results = detect_element->detect_results;
        if (results && !results->empty()) {
            for (const auto& res : *results) {
                const auto& box = res.box;
                // Add additional safety checks
                if (box.size() >= 4 && std::any_of(box.begin(), box.end(), [](int v) { return v != 0; })) {
                    draw_rectangle_rgb(rgb_buf, width, height,
                                    box[0], box[1], box[2], box[3],
                                    0, 0, 255, 0, 0, 5, false);
                }
            }
        }
    
        camera_pipeline_queue_element_index(detect_pipeline, detect_element->index);
    }

    return ESP_OK;
}