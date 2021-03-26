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

#include <string.h>
#include <sys/queue.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "touch_element/touch_element_private.h"

#define TE_BOD_DEFAULT_QTF_THR(obj)                       ((obj)->global_config->vertical_global_config.quantify_lower_threshold)
#define TE_BOD_DEFAULT_POS_FILTER_FACTOR(obj)             ((obj)->global_config->vertical_global_config.position_filter_factor)
#define TE_BOD_DEFAULT_CALCULATE_CHANNEL(obj)             ((obj)->global_config->vertical_global_config.calculate_channel_count)
#define TE_BOD_DEFAULT_BCM_UPDATE_TIME(obj)               ((obj)->global_config->vertical_global_config.benchmark_update_time)
#define TE_BOD_DEFAULT_FILTER_RESET_TIME(obj)             ((obj)->global_config->vertical_global_config.filter_reset_time)
#define TE_BOD_DEFAULT_POS_FILTER_SIZE(obj)               ((obj)->global_config->vertical_global_config.position_filter_size)

typedef struct te_board_handle_list {
    te_board_handle_t board_handle;
    SLIST_ENTRY(te_board_handle_list) next;
} te_board_handle_list_t;

typedef struct {
    SLIST_HEAD(te_board_handle_list_head, te_board_handle_list) handle_list;    //Slider handle (instance) list
    touch_board_global_config_t *global_config;                                  //Slider global configuration
    SemaphoreHandle_t mutex;                                                      //Slider object mutex
} te_board_obj_t;

te_board_obj_t *s_te_bod_obj = NULL;

static void slider_reset_position(te_slider_handle_t slider_handle);

static bool board_channel_check(te_board_handle_t board_handle, touch_pad_t channel_num);
static esp_err_t board_set_threshold(te_board_handle_t board_handle);
static void board_update_benchmark(te_board_handle_t board_handle);
static void board_update_state(te_board_handle_t board_handle, touch_pad_t channel_num, te_state_t channel_state);
static void board_reset_state(te_board_handle_t board_handle);
static void board_proc_state(te_board_handle_t board_handle);
static inline te_state_t board_get_state(te_board_handle_t board_handle);
static void board_event_give(te_board_handle_t board_handle);
static inline void board_dispatch(te_board_handle_t board_handle, touch_elem_dispatch_t dispatch_method);
static void board_update_position(te_board_handle_t board_handle);
static void board_reset_position(te_board_handle_t board_handle);

static esp_err_t board_object_add_instance(te_board_handle_t board_handle);
static esp_err_t board_object_remove_instance(te_board_handle_t board_handle);
static bool board_object_check_channel(touch_pad_t channel_num);
static esp_err_t board_object_set_threshold(void);
static void board_object_process_state(void);
static void board_object_update_state(touch_pad_t channel_num, te_state_t channel_state);

esp_err_t touch_board_install(const touch_board_global_config_t *global_config)
{
    TE_CHECK(te_system_check_state() == true, ESP_ERR_INVALID_STATE);
    TE_CHECK(global_config != NULL, ESP_ERR_INVALID_ARG);
    s_te_bod_obj = (te_board_obj_t *)calloc(1, sizeof(te_board_obj_t));
    TE_CHECK(s_te_bod_obj != NULL, ESP_ERR_NO_MEM);
    s_te_bod_obj->global_config = (touch_board_global_config_t *)calloc(1, sizeof(touch_board_global_config_t));
    s_te_bod_obj->mutex = xSemaphoreCreateMutex();
    TE_CHECK_GOTO(s_te_bod_obj->global_config != NULL && s_te_bod_obj->mutex != NULL, cleanup);

    xSemaphoreTake(s_te_bod_obj->mutex, portMAX_DELAY);
    SLIST_INIT(&s_te_bod_obj->handle_list);
    memcpy(s_te_bod_obj->global_config, global_config, sizeof(touch_board_global_config_t));
    te_object_methods_t board_methods = {
        .handle = s_te_bod_obj,
        .check_channel = board_object_check_channel,
        .set_threshold = board_object_set_threshold,
        .process_state = board_object_process_state,
        .update_state = board_object_update_state
    };
    te_object_method_register(&board_methods, TE_CLS_TYPE_SLIDER);
    xSemaphoreGive(s_te_bod_obj->mutex);
    return ESP_OK;

cleanup:
    TE_FREE_AND_NULL(s_te_bod_obj->global_config);
    if (s_te_bod_obj->mutex != NULL) {
        vSemaphoreDelete(s_te_bod_obj->mutex);
    }
    TE_FREE_AND_NULL(s_te_bod_obj);
    return ESP_ERR_NO_MEM;
}

void touch_board_uninstall(void)
{
    xSemaphoreTake(s_te_bod_obj->mutex, portMAX_DELAY);
    if (s_te_bod_obj == NULL) {
        xSemaphoreGive(s_te_bod_obj->mutex);
        return;
    }
    te_object_method_unregister(TE_CLS_TYPE_BOARD);
    free(s_te_bod_obj->global_config);
    s_te_bod_obj->global_config = NULL;
    while (!SLIST_EMPTY(&s_te_bod_obj->handle_list)) {
        SLIST_FIRST(&s_te_bod_obj->handle_list);
        SLIST_REMOVE_HEAD(&s_te_bod_obj->handle_list, next);
    }
    xSemaphoreGive(s_te_bod_obj->mutex);
    vSemaphoreDelete(s_te_bod_obj->mutex);
    free(s_te_bod_obj);
    s_te_bod_obj = NULL;
}

esp_err_t touch_board_create(const touch_board_config_t *board_config, touch_board_handle_t *board_handle)
{
    TE_CHECK(s_te_bod_obj != NULL, ESP_ERR_INVALID_STATE);
    TE_CHECK(board_handle != NULL && board_config != NULL, ESP_ERR_INVALID_ARG);
    TE_CHECK(board_config->horizontal_config.channel_num > 2 &&
             board_config->horizontal_config.channel_num < TOUCH_PAD_MAX &&
             board_config->horizontal_config.channel_array != NULL &&
             board_config->horizontal_config.sensitivity_array != NULL &&
             board_config->horizontal_config.position_range > board_config->horizontal_config.channel_num,
             ESP_ERR_INVALID_ARG);
    TE_CHECK(te_object_check_channel(board_config->horizontal_config.channel_array, board_config->horizontal_config.channel_num) == false,
             ESP_ERR_INVALID_ARG);
    TE_CHECK(board_config->vertical_config.channel_num > 2 &&
             board_config->vertical_config.channel_num < TOUCH_PAD_MAX &&
             board_config->vertical_config.channel_array != NULL &&
             board_config->vertical_config.sensitivity_array != NULL &&
             board_config->vertical_config.position_range > board_config->vertical_config.channel_num,
             ESP_ERR_INVALID_ARG);
    TE_CHECK(te_object_check_channel(board_config->vertical_config.channel_array, board_config->vertical_config.channel_num) == false,
             ESP_ERR_INVALID_ARG);
    te_board_handle_t te_board = (te_board_handle_t)calloc(1, sizeof(struct te_board_s));
    TE_CHECK(te_board != NULL, ESP_ERR_NO_MEM);
    esp_err_t ret = ESP_ERR_NO_MEM;

    te_board->config = (te_board_handle_config_t *)calloc(1, sizeof(te_board_handle_config_t));
    TE_CHECK_GOTO(te_board->config != NULL, cleanup);
    te_board->horizontal_slider.pos_filter_window = calloc(TE_BOD_DEFAULT_POS_FILTER_SIZE(s_te_bod_obj), sizeof(uint8_t));
    te_board->horizontal_slider.device = (te_dev_t **)calloc(board_config->horizontal_config.channel_num, sizeof(te_dev_t *));
    te_board->horizontal_slider.channel_bcm = (uint32_t *)calloc(board_config->horizontal_config.channel_num, sizeof(uint32_t));
    te_board->horizontal_slider.quantify_signal_array = (float *)calloc(board_config->horizontal_config.channel_num, sizeof(float));
    TE_CHECK_GOTO(te_board->horizontal_slider.pos_filter_window != NULL &&
                  te_board->horizontal_slider.device != NULL &&
                  te_board->horizontal_slider.channel_bcm != NULL &&
                  te_board->horizontal_slider.quantify_signal_array != NULL,
                  cleanup);
    for (int idx = 0; idx < board_config->horizontal_config.channel_num; idx++) {
        te_board->horizontal_slider.device[idx] = (te_dev_t *)calloc(1, sizeof(te_dev_t));
        if (te_board->horizontal_slider.device[idx] == NULL) {
            ret = ESP_ERR_NO_MEM;
            goto cleanup;
        }
    }
    ret = te_dev_init(te_board->horizontal_slider.device, board_config->horizontal_config.channel_num, TOUCH_ELEM_TYPE_SLIDER,
                      board_config->horizontal_config.channel_array, board_config->horizontal_config.sensitivity_array,1);
    TE_CHECK_GOTO(ret == ESP_OK, cleanup);

    te_board->vertical_slider.pos_filter_window = calloc(TE_BOD_DEFAULT_POS_FILTER_SIZE(s_te_bod_obj), sizeof(uint8_t));
    te_board->vertical_slider.device = (te_dev_t **)calloc(board_config->vertical_config.channel_num, sizeof(te_dev_t *));
    te_board->vertical_slider.channel_bcm = (uint32_t *)calloc(board_config->vertical_config.channel_num, sizeof(uint32_t));
    te_board->vertical_slider.quantify_signal_array = (float *)calloc(board_config->vertical_config.channel_num, sizeof(float));
    TE_CHECK_GOTO(te_board->vertical_slider.pos_filter_window != NULL &&
                  te_board->vertical_slider.device != NULL &&
                  te_board->vertical_slider.channel_bcm != NULL &&
                  te_board->vertical_slider.quantify_signal_array != NULL,
                  cleanup);
    for (int idx = 0; idx < board_config->vertical_config.channel_num; idx++) {
        te_board->vertical_slider.device[idx] = (te_dev_t *)calloc(1, sizeof(te_dev_t));
        if (te_board->vertical_slider.device[idx] == NULL) {
            ret = ESP_ERR_NO_MEM;
            goto cleanup;
        }
    }
    ret = te_dev_init(te_board->vertical_slider.device, board_config->vertical_config.channel_num, TOUCH_ELEM_TYPE_SLIDER,
                      board_config->vertical_config.channel_array, board_config->vertical_config.sensitivity_array,1);
    TE_CHECK_GOTO(ret == ESP_OK, cleanup);

    te_board->config->event_mask = TOUCH_ELEM_EVENT_NONE;
    te_board->config->dispatch_method = TOUCH_ELEM_DISP_MAX;
    te_board->config->callback = NULL;
    te_board->config->arg = NULL;
    te_board->current_state = TE_STATE_IDLE;
    te_board->last_state = TE_STATE_IDLE;

    te_board->horizontal_slider.channel_bcm_update_cnt = TE_BOD_DEFAULT_BCM_UPDATE_TIME(s_te_bod_obj); //update at first time
    te_board->horizontal_slider.filter_reset_cnt = TE_BOD_DEFAULT_FILTER_RESET_TIME(s_te_bod_obj);     //reset at first time
    te_board->horizontal_slider.channel_sum = board_config->horizontal_config.channel_num;
    te_board->horizontal_slider.position_range = board_config->horizontal_config.position_range;
    te_board->horizontal_slider.position_scale = (float)(board_config->horizontal_config.position_range) / (board_config->horizontal_config.channel_num - 1);
    te_board->horizontal_slider.current_state = TE_STATE_IDLE;
    te_board->horizontal_slider.last_state = TE_STATE_IDLE;
    te_board->horizontal_slider.event = TOUCH_SLIDER_EVT_MAX;
    te_board->horizontal_slider.position = 0;
    te_board->horizontal_slider.last_position = 0;
    te_board->horizontal_slider.pos_window_idx = 0;
    te_board->horizontal_slider.is_first_sample = true;

    te_board->vertical_slider.channel_bcm_update_cnt = TE_BOD_DEFAULT_BCM_UPDATE_TIME(s_te_bod_obj); //update at first time
    te_board->vertical_slider.filter_reset_cnt = TE_BOD_DEFAULT_FILTER_RESET_TIME(s_te_bod_obj);     //reset at first time
    te_board->vertical_slider.channel_sum = board_config->vertical_config.channel_num;
    te_board->vertical_slider.position_range = board_config->vertical_config.position_range;
    te_board->vertical_slider.position_scale = (float)(board_config->vertical_config.position_range) / (board_config->vertical_config.channel_num - 1);
    te_board->vertical_slider.current_state = TE_STATE_IDLE;
    te_board->vertical_slider.last_state = TE_STATE_IDLE;
    te_board->vertical_slider.event = TOUCH_SLIDER_EVT_MAX;
    te_board->vertical_slider.position = 0;
    te_board->vertical_slider.last_position = 0;
    te_board->vertical_slider.pos_window_idx = 0;
    te_board->vertical_slider.is_first_sample = true;

    ret = board_object_add_instance(te_board);
    TE_CHECK_GOTO(ret == ESP_OK, cleanup);
    *board_handle = (touch_board_handle_t)te_board;
    return ESP_OK;

cleanup:
    TE_FREE_AND_NULL(te_board->config);

    TE_FREE_AND_NULL(te_board->horizontal_slider.pos_filter_window);
    TE_FREE_AND_NULL(te_board->horizontal_slider.channel_bcm);
    TE_FREE_AND_NULL(te_board->horizontal_slider.quantify_signal_array);
    if (te_board->horizontal_slider.device != NULL) {
        for (int idx = 0; idx < board_config->horizontal_config.channel_num; idx++) {
            TE_FREE_AND_NULL(te_board->horizontal_slider.device[idx]);
        }
        free(te_board->horizontal_slider.device);
        te_board->horizontal_slider.device = NULL;
    }

    TE_FREE_AND_NULL(te_board->vertical_slider.pos_filter_window);
    TE_FREE_AND_NULL(te_board->vertical_slider.channel_bcm);
    TE_FREE_AND_NULL(te_board->vertical_slider.quantify_signal_array);
    if (te_board->vertical_slider.device != NULL) {
        for (int idx = 0; idx < board_config->vertical_config.channel_num; idx++) {
            TE_FREE_AND_NULL(te_board->vertical_slider.device[idx]);
        }
        free(te_board->vertical_slider.device);
        te_board->vertical_slider.device = NULL;
    }

    TE_FREE_AND_NULL(te_board);
    return ret;
}

esp_err_t touch_board_delete(touch_board_handle_t board_handle)
{
    TE_CHECK(s_te_bod_obj != NULL, ESP_ERR_INVALID_STATE);
    TE_CHECK(board_handle != NULL, ESP_ERR_INVALID_ARG);
    /*< Release touch sensor application resource */
    esp_err_t ret = board_object_remove_instance(board_handle);
    TE_CHECK(ret == ESP_OK, ret);
    te_board_handle_t te_board = (te_board_handle_t)board_handle;
    /*< Release touch sensor device resource */
    te_dev_deinit(te_board->horizontal_slider.device, te_board->horizontal_slider.channel_sum);
    for (int idx = 0; idx < te_board->horizontal_slider.channel_sum; idx++) {
        free(te_board->horizontal_slider.device[idx]);
    }
    te_dev_deinit(te_board->vertical_slider.device, te_board->vertical_slider.channel_sum);
    for (int idx = 0; idx < te_board->vertical_slider.channel_sum; idx++) {
        free(te_board->vertical_slider.device[idx]);
    }

    free(te_board->horizontal_slider.quantify_signal_array);
    free(te_board->horizontal_slider.pos_filter_window);
    free(te_board->horizontal_slider.channel_bcm);
    free(te_board->horizontal_slider.device);

    free(te_board->vertical_slider.quantify_signal_array);
    free(te_board->vertical_slider.pos_filter_window);
    free(te_board->vertical_slider.channel_bcm);
    free(te_board->vertical_slider.device);

    free(te_board->config);
    free(te_board);
    return ESP_OK;
}

esp_err_t touch_board_set_dispatch_method(touch_board_handle_t board_handle, touch_elem_dispatch_t dispatch_method)
{
    TE_CHECK(s_te_bod_obj != NULL, ESP_ERR_INVALID_STATE);
    TE_CHECK(board_handle != NULL, ESP_ERR_INVALID_ARG);
    TE_CHECK(dispatch_method >= TOUCH_ELEM_DISP_EVENT && dispatch_method <= TOUCH_ELEM_DISP_MAX, ESP_ERR_INVALID_ARG);
    xSemaphoreTake(s_te_bod_obj->mutex, portMAX_DELAY);
    te_board_handle_t te_slider = (te_board_handle_t)board_handle;
    te_slider->config->dispatch_method = dispatch_method;
    xSemaphoreGive(s_te_bod_obj->mutex);
    return ESP_OK;
}

esp_err_t touch_board_subscribe_event(touch_board_handle_t board_handle, uint32_t event_mask, void *arg)
{
    TE_CHECK(s_te_bod_obj != NULL, ESP_ERR_INVALID_STATE);
    TE_CHECK(board_handle != NULL, ESP_ERR_INVALID_ARG);
    if (!(event_mask & TOUCH_ELEM_EVENT_ON_PRESS) && !(event_mask & TOUCH_ELEM_EVENT_ON_RELEASE) &&
        !(event_mask & TOUCH_ELEM_EVENT_NONE) && !(event_mask & TOUCH_ELEM_EVENT_ON_CALCULATION)) {
        ESP_LOGE(TE_TAG, "Touch board only support TOUCH_ELEM_EVENT_ON_PRESS, "
                         "TOUCH_ELEM_EVENT_ON_RELEASE, TOUCH_ELEM_EVENT_ON_CALCULATION event mask");
        return ESP_ERR_INVALID_ARG;
    }
    xSemaphoreTake(s_te_bod_obj->mutex, portMAX_DELAY);
    te_board_handle_t te_board = (te_board_handle_t)board_handle;
    te_board->config->event_mask = event_mask;
    te_board->config->arg = arg;
    xSemaphoreGive(s_te_bod_obj->mutex);
    return ESP_OK;
}

esp_err_t touch_board_set_callback(touch_board_handle_t board_handle, touch_board_callback_t board_callback)
{
    TE_CHECK(s_te_bod_obj != NULL, ESP_ERR_INVALID_STATE);
    TE_CHECK(board_handle != NULL, ESP_ERR_INVALID_ARG);
    TE_CHECK(board_callback != NULL, ESP_ERR_INVALID_ARG);
    te_board_handle_t te_board = (te_board_handle_t)board_handle;
    xSemaphoreTake(s_te_bod_obj->mutex, portMAX_DELAY);
    te_board->config->callback = board_callback;
    xSemaphoreGive(s_te_bod_obj->mutex);
    return ESP_OK;
}

const touch_board_message_t* touch_board_get_message(const touch_elem_message_t* element_message)
{
    return (touch_board_message_t*)&element_message->child_msg;
    _Static_assert(sizeof(element_message->child_msg) >= sizeof(touch_board_message_t), "Message size overflow");
}

static esp_err_t board_object_add_instance(te_board_handle_t board_handle)
{
    te_board_handle_list_t *item = (te_board_handle_list_t *)calloc(1, sizeof(te_board_handle_list_t));
    TE_CHECK(item != NULL, ESP_ERR_NO_MEM);
    item->board_handle = board_handle;
    xSemaphoreTake(s_te_bod_obj->mutex, portMAX_DELAY);
    SLIST_INSERT_HEAD(&s_te_bod_obj->handle_list, item, next);
    xSemaphoreGive(s_te_bod_obj->mutex);
    return ESP_OK;
}

static esp_err_t board_object_remove_instance(te_board_handle_t board_handle)
{
    esp_err_t ret = ESP_ERR_NOT_FOUND;
    te_board_handle_list_t *item;
    SLIST_FOREACH(item, &s_te_bod_obj->handle_list, next) {
        if (board_handle == item->board_handle) {
            xSemaphoreTake(s_te_bod_obj->mutex, portMAX_DELAY);
            SLIST_REMOVE(&s_te_bod_obj->handle_list, item, te_board_handle_list, next);
            xSemaphoreGive(s_te_bod_obj->mutex);
            free(item);
            ret = ESP_OK;
            break;
        }
    }
    return ret;
}

static bool board_object_check_channel(touch_pad_t channel_num)
{
    te_board_handle_list_t *item;
    SLIST_FOREACH(item, &s_te_bod_obj->handle_list, next) {
        if (board_channel_check(item->board_handle, channel_num)) {
            return true;
        }
    }
    return false;
}

static esp_err_t board_object_set_threshold(void)
{
    esp_err_t ret = ESP_OK;
    te_board_handle_list_t *item;
    SLIST_FOREACH(item, &s_te_bod_obj->handle_list, next) {
        ret = board_set_threshold(item->board_handle);
        if (ret != ESP_OK) {
            break;
        }
    }
    return ret;
}

static void board_object_process_state(void)
{
    te_board_handle_list_t *item;
    SLIST_FOREACH(item, &s_te_bod_obj->handle_list, next) {
        if (waterproof_check_mask_handle(item->board_handle)) {
            board_reset_state(item->board_handle);
            board_reset_position(item->board_handle);
            continue;
        }
        board_proc_state(item->board_handle);
    }
}

static void board_object_update_state(touch_pad_t channel_num, te_state_t channel_state)
{
    te_board_handle_list_t *item;
    SLIST_FOREACH(item, &s_te_bod_obj->handle_list, next) {
        if (waterproof_check_mask_handle(item->board_handle)) {
            continue;
        }
        board_update_state(item->board_handle, channel_num, channel_state);
    }
}

static bool slider_channel_check(te_slider_handle_t slider_handle, touch_pad_t channel_num)
{
    te_dev_t *device;
    for (int idx = 0; idx < slider_handle->channel_sum; idx++) {
        device = slider_handle->device[idx];
        if (device->channel == channel_num) {
            return true;
        }
    }
    return false;
}


static void slider_update_benchmark(te_slider_handle_t slider_handle)
{
    for (int idx = 0; idx < slider_handle->channel_sum; idx++) {
        uint32_t bcm_val;
        te_dev_t *device = slider_handle->device[idx];
        bcm_val = te_read_smooth_signal(device->channel);
        slider_handle->channel_bcm[idx] = bcm_val;
    }
}

static esp_err_t slider_set_threshold(te_slider_handle_t slider_handle)
{
    esp_err_t ret = ESP_OK;
    for (int idx = 0; idx < slider_handle->channel_sum; idx++) {
        ret |= te_dev_set_threshold(slider_handle->device[idx]);
    }
    slider_update_benchmark(slider_handle);  //Update benchmark at startup
    return ret;
}

static void slider_update_state(te_slider_handle_t slider_handle, touch_pad_t channel_num, te_state_t channel_state)
{
    te_dev_t *device;
    for (int idx = 0; idx < slider_handle->channel_sum; idx++) {
        device = slider_handle->device[idx];
        if (channel_num == device->channel) {
            device->state = channel_state;
        }
    }
}

static void slider_reset_state(te_slider_handle_t slider_handle)
{
    for (int idx = 0; idx < slider_handle->channel_sum; idx++) {
        slider_handle->device[idx]->state = TE_STATE_IDLE;
    }
    slider_handle->current_state = TE_STATE_IDLE;
}

static bool board_channel_check(te_board_handle_t board_handle, touch_pad_t channel_num)
{
    bool ret = false;
    ret |= slider_channel_check(&board_handle->horizontal_slider, channel_num);
    ret |= slider_channel_check(&board_handle->vertical_slider, channel_num);
    return ret;
}

static esp_err_t board_set_threshold(te_board_handle_t board_handle)
{
    slider_set_threshold(&board_handle->horizontal_slider);
    slider_set_threshold(&board_handle->vertical_slider);
    board_update_benchmark(board_handle);  //Update benchmark at startup
    return ESP_OK;
}

static void board_update_benchmark(te_board_handle_t board_handle)
{
    slider_update_benchmark(&board_handle->horizontal_slider);
    slider_update_benchmark(&board_handle->vertical_slider);
}

static void board_update_state(te_board_handle_t board_handle, touch_pad_t channel_num, te_state_t channel_state)
{
    slider_update_state(&board_handle->horizontal_slider, channel_num, channel_state);
    slider_update_state(&board_handle->vertical_slider, channel_num, channel_state);
}

static void board_reset_state(te_board_handle_t board_handle)
{
    slider_reset_state(&board_handle->horizontal_slider);
    slider_reset_state(&board_handle->vertical_slider);
    board_handle->current_state = TE_STATE_IDLE;
}

static void board_proc_state(te_board_handle_t board_handle)
{
    uint32_t event_mask = board_handle->config->event_mask;
    touch_elem_dispatch_t dispatch_method = board_handle->config->dispatch_method;
    BaseType_t mux_ret = xSemaphoreTake(s_te_bod_obj->mutex, 0);
    if (mux_ret != pdPASS) {
        return;
    }

    board_handle->current_state = board_get_state(board_handle);

    if (board_handle->current_state == TE_STATE_PRESS) {
        board_handle->horizontal_slider.channel_bcm_update_cnt = 0;   // Reset benchmark update counter
        board_handle->vertical_slider.channel_bcm_update_cnt = 0;
        board_update_position(board_handle);
        if (board_handle->last_state == TE_STATE_IDLE) {  //IDLE ---> Press = On_Press
            ESP_LOGD(TE_DEBUG_TAG, "board press");
            if (event_mask & TOUCH_ELEM_EVENT_ON_PRESS) {
                board_handle->event = TOUCH_BOARD_EVT_ON_PRESS;
                board_dispatch(board_handle, dispatch_method);
            }
        } else if (board_handle->last_state == TE_STATE_PRESS) {  //Press ---> Press = On_Calculation
            ESP_LOGD(TE_DEBUG_TAG, "board calculation");
            if (event_mask & TOUCH_ELEM_EVENT_ON_CALCULATION) {
                board_handle->event = TOUCH_BOARD_EVT_ON_CALCULATION;
                board_dispatch(board_handle, dispatch_method);
            }
        }
    } else if (board_handle->current_state == TE_STATE_RELEASE) {
        if (board_handle->last_state == TE_STATE_PRESS) { //Press ---> Release = On_Release
            ESP_LOGD(TE_DEBUG_TAG, "board release");
            if (event_mask & TOUCH_ELEM_EVENT_ON_RELEASE) {
                board_handle->event = TOUCH_BOARD_EVT_ON_RELEASE;
                board_dispatch(board_handle, dispatch_method);
            }
        } else if (board_handle->last_state == TE_STATE_RELEASE) { // Release ---> Release = On_IDLE (Not dispatch)
            board_reset_state(board_handle);
        }
    } else if (board_handle->current_state == TE_STATE_IDLE) {
        if (board_handle->last_state == TE_STATE_RELEASE) { //Release ---> IDLE = On_IDLE (Not dispatch)
            //Nothing
        } else if (board_handle->last_state == TE_STATE_IDLE) { //IDLE ---> IDLE = Running IDLE (Not dispatch)
            if (++board_handle->horizontal_slider.channel_bcm_update_cnt >= TE_BOD_DEFAULT_BCM_UPDATE_TIME(s_te_bod_obj)) {  //Update channel benchmark
                board_handle->horizontal_slider.channel_bcm_update_cnt = 0;
                slider_update_benchmark(&board_handle->horizontal_slider);
                ESP_LOGD(TE_DEBUG_TAG, "h slider bcm update");
            }
            if (++board_handle->horizontal_slider.filter_reset_cnt >= TE_BOD_DEFAULT_FILTER_RESET_TIME(s_te_bod_obj)) {
                slider_reset_position(&board_handle->horizontal_slider);  //Reset slider filter so as to speed up next time position calculation
            }

            if (++board_handle->vertical_slider.channel_bcm_update_cnt >= TE_BOD_DEFAULT_BCM_UPDATE_TIME(s_te_bod_obj)) {  //Update channel benchmark
                board_handle->vertical_slider.channel_bcm_update_cnt = 0;
                slider_update_benchmark(&board_handle->vertical_slider);
                ESP_LOGD(TE_DEBUG_TAG, "v slider bcm update");
            }
            if (++board_handle->vertical_slider.filter_reset_cnt >= TE_BOD_DEFAULT_FILTER_RESET_TIME(s_te_bod_obj)) {
                slider_reset_position(&board_handle->vertical_slider);  //Reset slider filter so as to speed up next time position calculation
            }
        }
    }
    board_handle->last_state = board_handle->current_state;
    xSemaphoreGive(s_te_bod_obj->mutex);
}

static te_state_t slider_get_state(te_dev_t **device, int device_num)
{
    /*< Scan the state of all the slider channel and calculate the number of them if the state is Press*/
    uint8_t press_cnt = 0;
    uint8_t idle_cnt = 0;
    for (int idx = 0; idx < device_num; idx++) {  //Calculate how many channel is pressed
        if (device[idx]->state == TE_STATE_PRESS) {
            press_cnt++;
        } else if (device[idx]->state == TE_STATE_IDLE) {
            idle_cnt++;
        }
    }
    if (press_cnt > 0) {
        return TE_STATE_PRESS;
    } else if (idle_cnt == device_num) {
        return TE_STATE_IDLE;
    } else {
        return TE_STATE_RELEASE;
    }
}

static inline te_state_t board_get_state(te_board_handle_t board_handle)
{
    te_state_t horizontal_state = slider_get_state(board_handle->horizontal_slider.device, board_handle->horizontal_slider.channel_sum);
    te_state_t vertical_state = slider_get_state(board_handle->vertical_slider.device, board_handle->vertical_slider.channel_sum);
    if (horizontal_state == TE_STATE_PRESS && vertical_state == TE_STATE_PRESS) {
        return TE_STATE_PRESS;
    } else if (horizontal_state == TE_STATE_RELEASE && vertical_state == TE_STATE_RELEASE) {
        return TE_STATE_RELEASE;
    } else if (horizontal_state == TE_STATE_IDLE && vertical_state == TE_STATE_IDLE) {
        return TE_STATE_IDLE;
    } else { //horizontal_state != vertical_state, middle state hold on last state
        return board_handle->last_state;
    }
}

static void board_event_give(te_board_handle_t board_handle)
{
    touch_elem_message_t element_message;
    touch_board_message_t board_message = {
        .event = board_handle->event,
        .position = board_handle->position
    };
    element_message.handle = (touch_elem_handle_t)board_handle;
    element_message.element_type = TOUCH_ELEM_TYPE_SLIDER;
    element_message.arg = board_handle->config->arg;
    memcpy(element_message.child_msg, &board_message, sizeof(board_message));
    te_event_give(element_message);
}

static inline void board_dispatch(te_board_handle_t board_handle, touch_elem_dispatch_t dispatch_method)
{
    if (dispatch_method == TOUCH_ELEM_DISP_EVENT) {
        board_event_give(board_handle);  //Event queue
    } else if (dispatch_method == TOUCH_ELEM_DISP_CALLBACK) {
        touch_board_message_t board_info;
        board_info.event = board_handle->event;
        board_info.position = board_handle->position;
        void *arg = board_handle->config->arg;
        board_handle->config->callback(board_handle, &board_info, arg);  //Event callback
    }
}

/**
 * @brief Slider channel difference-rate re-quantization
 *
 * This function will re-quantifies the touch sensor slider channel difference-rate
 * so as to make the different size of touch pad in PCB has the same difference value
 *
 */
static inline void slider_quantify_signal(te_slider_handle_t slider_handle)
{
    float weight_sum = 0;
    for (int idx = 0; idx < slider_handle->channel_sum; idx++) {
        te_dev_t *device = slider_handle->device[idx];
        weight_sum += device->sens;
        uint32_t current_signal = te_read_smooth_signal(device->channel);
        int ans = current_signal - slider_handle->channel_bcm[idx];
        float diff_rate = (float)ans / slider_handle->channel_bcm[idx];
        slider_handle->quantify_signal_array[idx] = diff_rate / device->sens;
        if (slider_handle->quantify_signal_array[idx] < TE_BOD_DEFAULT_QTF_THR(s_te_bod_obj)) {
            slider_handle->quantify_signal_array[idx] = 0;
        }
    }
    for (int idx = 0; idx < slider_handle->channel_sum; idx++) {
        te_dev_t *device = slider_handle->device[idx];
        slider_handle->quantify_signal_array[idx] = slider_handle->quantify_signal_array[idx] * weight_sum / device->sens;
    }
}

/**
 * @brief Calculate max sum subarray
 *
 * This function will figure out the max sum subarray from the
 * input array, return the max sum and max sum start index
 *
 */
static inline float slider_search_max_subarray(const float *array, int array_size, int *max_array_idx)
{
    *max_array_idx = 0;
    float max_array_sum = 0;
    float current_array_sum = 0;
    for (int idx = 0; idx <= (array_size - TE_BOD_DEFAULT_CALCULATE_CHANNEL(s_te_bod_obj)); idx++) {
        for (int x = idx; x < idx + TE_BOD_DEFAULT_CALCULATE_CHANNEL(s_te_bod_obj); x++) {
            current_array_sum += array[x];
        }
        if (max_array_sum < current_array_sum) {
            max_array_sum = current_array_sum;
            *max_array_idx = idx;
        }
        current_array_sum = 0;
    }
    return max_array_sum;
}

/**
 * @brief Calculate zero number
 *
 * This function will figure out the number of non-zero items from
 * the subarray
 */
static inline uint8_t slider_get_non_zero_num(const float *array, uint8_t array_idx)
{
    uint8_t zero_cnt = 0;
    for (int idx = array_idx; idx < array_idx + TE_BOD_DEFAULT_CALCULATE_CHANNEL(s_te_bod_obj); idx++) {
        zero_cnt += (array[idx] > 0) ? 1 : 0;
    }
    return zero_cnt;
}

static inline uint32_t slider_calculate_position(te_slider_handle_t slider_handle, int subarray_index, float subarray_sum, int non_zero_num)
{
    int range = slider_handle->position_range;
    int array_size = slider_handle->channel_sum;
    float scale = slider_handle->position_scale;
    const float *array = slider_handle->quantify_signal_array;
    uint32_t position = 0;
    if (non_zero_num == 0) {
        position = slider_handle->position;
    } else if (non_zero_num == 1) {
        for (int index = subarray_index; index < subarray_index + TE_BOD_DEFAULT_CALCULATE_CHANNEL(s_te_bod_obj); index++) {
            if (0 != array[index]) {
                if (index == array_size - 1) {
                    position = range;
                } else {
                    position = (uint32_t)((float)index * scale);
                }
                break;
            }
        }
    } else {
        for (int idx = subarray_index; idx < subarray_index + TE_BOD_DEFAULT_CALCULATE_CHANNEL(s_te_bod_obj); idx++) {
            position += ((float)idx * array[idx]);
        }
        position = position * scale / subarray_sum;
    }
    return position;
}

static uint32_t slider_filter_average(te_slider_handle_t slider_handle, uint32_t current_position)
{
    uint32_t position_average = 0;
    if (slider_handle->is_first_sample) {
        for (int win_idx = 0; win_idx < TE_BOD_DEFAULT_POS_FILTER_SIZE(s_te_bod_obj); win_idx++) {
            slider_handle->pos_filter_window[win_idx] = current_position;  //Preload filter buffer
        }
        slider_handle->is_first_sample = false;
    } else {
        slider_handle->pos_filter_window[slider_handle->pos_window_idx++] = current_position; //Moving average filter
        if (slider_handle->pos_window_idx >= TE_BOD_DEFAULT_POS_FILTER_SIZE(s_te_bod_obj)) {
            slider_handle->pos_window_idx = 0;
        }
    }

    for (int win_idx = 0; win_idx < TE_BOD_DEFAULT_POS_FILTER_SIZE(s_te_bod_obj); win_idx++) { //Moving average filter
        position_average += slider_handle->pos_filter_window[win_idx];
    }
    position_average = position_average / TE_BOD_DEFAULT_POS_FILTER_SIZE(s_te_bod_obj) + 0.5;
    return position_average;
}

static inline uint32_t slider_filter_iir(uint32_t in_now, uint32_t out_last, uint32_t k)
{
    if (k == 0) {
        return in_now;
    } else {
        uint32_t out_now = (in_now + (k - 1) * out_last) / k;
        return out_now;
    }
}

/**
 * @brief touch sensor slider position update
 *
 * This function is the core algorithm about touch sensor slider
 * position update, mainly has several steps:
 *      1. Re-quantization
 *      2. Figure out changed channel
 *      3. Calculate position
 *      4. Filter
 *
 */
static void slider_update_position(te_slider_handle_t slider_handle)
{
    int max_array_idx = 0;
    float max_array_sum;
    uint8_t non_zero_num;
    uint32_t current_position;

    slider_quantify_signal(slider_handle);
    max_array_sum = slider_search_max_subarray(slider_handle->quantify_signal_array, slider_handle->channel_sum, &max_array_idx);
    non_zero_num = slider_get_non_zero_num(slider_handle->quantify_signal_array, max_array_idx);
    current_position = slider_calculate_position(slider_handle, max_array_idx, max_array_sum, non_zero_num);
    uint32_t position_average = slider_filter_average(slider_handle, current_position);
    slider_handle->last_position = slider_handle->last_position == 0 ? (position_average << 4) : slider_handle->last_position;
    slider_handle->last_position = slider_filter_iir((position_average << 4), slider_handle->last_position, TE_BOD_DEFAULT_POS_FILTER_FACTOR(s_te_bod_obj));
    slider_handle->position = ((slider_handle->last_position + 8) >> 4);  //(x + 8) >> 4 ---->  (x + 8) / 16 ----> x/16 + 0.5
}

static void board_update_position(te_board_handle_t board_handle)
{
    slider_update_position(&board_handle->horizontal_slider);
    slider_update_position(&board_handle->vertical_slider);
    board_handle->position.x_axis = board_handle->horizontal_slider.position;
    board_handle->position.y_axis = board_handle->vertical_slider.position;
}

static void slider_reset_position(te_slider_handle_t slider_handle)
{
    slider_handle->is_first_sample = true;
    slider_handle->last_position   = 0;
    slider_handle->pos_window_idx  = 0;
}

static void board_reset_position(te_board_handle_t board_handle)
{
    slider_reset_position(&board_handle->horizontal_slider);
    slider_reset_position(&board_handle->vertical_slider);
}