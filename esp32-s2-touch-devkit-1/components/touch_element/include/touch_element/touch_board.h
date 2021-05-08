// Copyright 2016-2020 Espressif Systems (Shanghai) PTE LTD
//
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

#pragma once

#include "touch_element/touch_element.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    touch_slider_global_config_t horizontal_global_config;  //!< Touch Board horizontal slider global configuration
    touch_slider_global_config_t vertical_global_config;    //!< Touch Board horizontal slider vertical global configuration
} touch_board_global_config_t;

typedef struct {
    touch_slider_config_t horizontal_config;    //!< Horizontal slider configuration (for new instance) passed to touch_board_create()
    touch_slider_config_t vertical_config;      //!< Vertical slider configuration (for new instance) passed to touch_board_create()
} touch_board_config_t;

typedef enum {
    TOUCH_BOARD_EVT_ON_PRESS,                  //!< Touch Board on Press event
    TOUCH_BOARD_EVT_ON_RELEASE,                //!< Touch Board on Release event
    TOUCH_BOARD_EVT_ON_CALCULATION,            //!< Touch Board on Calculation event
    TOUCH_BOARD_EVT_MAX
} touch_board_event_t;

typedef struct {
    uint8_t x_axis;     //!< Touch Board x axis position
    uint8_t y_axis;     //!< Touch Board y axis position
} touch_board_position_t;

typedef struct {
    touch_board_event_t event;             //!< Touch Board event
    touch_board_position_t position;       //!< Touch Board position
} touch_board_message_t;

typedef touch_elem_handle_t touch_board_handle_t;  //!< Touch Board instance handle
typedef void(*touch_board_callback_t)(touch_board_handle_t, touch_board_message_t *, void *); //!< Touch Board callback type

/**
 * @brief   Touch board initialize
 *
 * This function initializes touch board object and acts on all
 * touch board instances.
 *
 * @param[in] global_config   Touch board global initialization configuration
 *
 * @return
 *      - ESP_OK: Successfully initialized touch board
 *      - ESP_ERR_INVALID_STATE: Touch element library was not initialized
 *      - ESP_ERR_INVALID_ARG: board_init is NULL
 *      - ESP_ERR_NO_MEM: Insufficient memory
 */
esp_err_t touch_board_install(const touch_board_global_config_t *global_config);

/**
 * @brief   Release resources allocated using touch_board_install()
 *
 * @return
 *      - ESP_OK: Successfully released resources
 */
void touch_board_uninstall(void);

/**
* @brief   Create a new touch board instance
*
* @param[in]  board_config     Touch board configuration
* @param[out] board_handle     Touch board handle
*
* @note    The index of Channel array and sensitivity array must be one-one correspondence
*
* @return
*      - ESP_OK: Successfully create touch board
*      - ESP_ERR_INVALID_STATE: Touch board driver was not initialized
*      - ESP_ERR_INVALID_ARG: Invalid configuration struct or arguments is NULL
*      - ESP_ERR_NO_MEM: Insufficient memory
*/
esp_err_t touch_board_create(const touch_board_config_t *board_config, touch_board_handle_t *board_handle);

/**
 * @brief   Release resources allocated using touch_board_create()
 *
 * @param[in] board_handle   Touch board handle
 * @return
 *      - ESP_OK: Successfully released resources
 *      - ESP_ERR_INVALID_STATE: Touch board driver was not initialized
 *      - ESP_ERR_INVALID_ARG: board_handle is null
 *      - ESP_ERR_NOT_FOUND: Input handle is not a board handle
 */
esp_err_t touch_board_delete(touch_board_handle_t board_handle);

/**
 * @brief   Touch board subscribes event
 *
 * This function uses event mask to subscribe to touch board events, once one of
 * the subscribed events occurs, the event message could be retrieved by calling
 * touch_element_message_receive() or input callback routine.
 *
 * @param[in] board_handle      Touch board handle
 * @param[in] event_mask        Touch board subscription event mask
 * @param[in] arg               User input argument
 *
 * @note    Touch board only support three kind of event masks, they are
 *          TOUCH_ELEM_EVENT_ON_PRESS, TOUCH_ELEM_EVENT_ON_RELEASE. You can use those event masks in any
 *          combination to achieve the desired effect.
 *
 * @return
 *      - ESP_OK: Successfully subscribed touch board event
 *      - ESP_ERR_INVALID_STATE: Touch board driver was not initialized
 *      - ESP_ERR_INVALID_ARG: board_handle is null or event is not supported
 */
esp_err_t touch_board_subscribe_event(touch_board_handle_t board_handle, uint32_t event_mask, void *arg);

/**
 * @brief   Touch board set dispatch method
 *
 * This function sets a dispatch method that the driver core will use
 * this method as the event notification method.
 *
 * @param[in] board_handle      Touch board handle
 * @param[in] dispatch_method   Dispatch method (By callback/event)
 *
 * @return
 *      - ESP_OK: Successfully set dispatch method
 *      - ESP_ERR_INVALID_STATE: Touch board driver was not initialized
 *      - ESP_ERR_INVALID_ARG: board_handle is null or dispatch_method is invalid
 */
esp_err_t touch_board_set_dispatch_method(touch_board_handle_t board_handle, touch_elem_dispatch_t dispatch_method);

/**
 * @brief   Touch board set callback
 *
 * This function sets a callback routine into touch element driver core,
 * when the subscribed events occur, the callback routine will be called.
 *
 * @param[in] board_handle     Touch board handle
 * @param[in] board_callback   User input callback
 *
 * @note        Touch board message will be passed from the callback function and it will be destroyed when the callback function return.
 *
 * @warning     Since this input callback routine runs on driver core (esp-timer callback routine),
 *              it should not do something that attempts to Block, such as calling vTaskDelay().
 *
 * @return
 *      - ESP_OK: Successfully set callback
 *      - ESP_ERR_INVALID_STATE: Touch board driver was not initialized
 *      - ESP_ERR_INVALID_ARG: board_handle or board_callback is null
 */
esp_err_t touch_board_set_callback(touch_board_handle_t board_handle, touch_board_callback_t board_callback);

/**
 * @brief   Touch board get message
 *
 * This function decodes the element message from touch_element_message_receive() and return
 * a touch board message pointer.
 *
 * @param[in] element_message   element message
 *
 * @return  Touch board message pointer
 */
const touch_board_message_t* touch_board_get_message(const touch_elem_message_t* element_message);



#ifdef __cplusplus
}
#endif
