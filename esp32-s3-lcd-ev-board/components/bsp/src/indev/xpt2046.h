// Copyright 2020 Espressif Systems (Shanghai) Co. Ltd.
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

#ifndef _XPT2046_H
#define _XPT2046_H

#include <stdbool.h>
#include "esp_lcd_types.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct esp_lcd_panel_t *esp_lcd_panel_handle_t;

/**
 * max point number on touch panel
 */
#define TOUCH_MAX_POINT_NUMBER (5)

/**
 * @brief Define all screen direction
 *
 */
typedef enum {
    /* @---> X
       |
       Y
    */
    TOUCH_DIR_LRTB,   /**< From left to right then from top to bottom, this consider as the original direction of the touch panel*/

    /*  Y
        |
        @---> X
    */
    TOUCH_DIR_LRBT,   /**< From left to right then from bottom to top */

    /* X <---@
             |
             Y
    */
    TOUCH_DIR_RLTB,   /**< From right to left then from top to bottom */

    /*       Y
             |
       X <---@
    */
    TOUCH_DIR_RLBT,   /**< From right to left then from bottom to top */

    /* @---> Y
       |
       X
    */
    TOUCH_DIR_TBLR,   /**< From top to bottom then from left to right */

    /*  X
        |
        @---> Y
    */
    TOUCH_DIR_BTLR,   /**< From bottom to top then from left to right */

    /* Y <---@
             |
             X
    */
    TOUCH_DIR_TBRL,   /**< From top to bottom then from right to left */

    /*       X
             |
       Y <---@
    */
    TOUCH_DIR_BTRL,   /**< From bottom to top then from right to left */

    TOUCH_DIR_MAX,

    /* Another way to represent rotation with 3 bit*/
    TOUCH_MIRROR_X = 0x40, /**< Mirror X-axis */
    TOUCH_MIRROR_Y = 0x20, /**< Mirror Y-axis */
    TOUCH_SWAP_XY  = 0x80, /**< Swap XY axis */
} touch_panel_dir_t;

/**
 * @brief Touch events
 *
 */
typedef enum {
    TOUCH_EVT_RELEASE = 0x0,  /*!< Release event */
    TOUCH_EVT_PRESS   = 0x1,  /*!< Press event */
} touch_panel_event_t;

/**
 * @brief Information of touch panel
 *
 */
typedef struct {
    touch_panel_event_t event;   /*!< Event of touch */
    uint8_t point_num;           /*!< Touch point number */
    uint16_t curx[TOUCH_MAX_POINT_NUMBER];            /*!< Current x coordinate */
    uint16_t cury[TOUCH_MAX_POINT_NUMBER];            /*!< Current y coordinate */
} touch_panel_points_t;

/**
 * @brief Configuration of touch panel
 *
 */
typedef struct {
    /** Interface configuration */
    struct {
        uint8_t host_id;    /*!< id of spi bus */
        int8_t pin_num_cs;           /*!< SPI Chip Select Pin */
        int clk_freq;                /*!< spi clock frequency */
    } interface;
    int8_t pin_num_int;                            /*!< Interrupt pin of touch panel. NOTE: Now this line is not used, you can set to -1 and no connection with hardware */
    touch_panel_dir_t direction;                   /*!< Rotate direction */
    uint16_t width;                                /*!< touch panel width */
    uint16_t height;                               /*!< touch panel height */
} touch_panel_config_t;

/**
 * @brief Initial touch panel
 *
 * @param config Pointer to a structure with touch config arguments.
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Fail
 */
esp_err_t xpt2046_init(void);

/**
 * @brief Deinitial touch panel
 *
 * @param free_bus Is free bus
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Fail
 */
esp_err_t xpt2046_deinit(void);

/**
 * @brief Check if there is a press
 *
 * @return
 *      - 0 Not press
 *      - 1 pressed
 */
int xpt2046_is_pressed(void);

/**
 * @brief Set touch rotate direction
 *
 * @param dir rotate direction
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Fail
 */
esp_err_t xpt2046_set_direction(touch_panel_dir_t dir);

/**
 * @brief Get raw data
 *
 * @param x Value of X axis direction
 * @param y Value of Y axis direction
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Fail
 */
esp_err_t xpt2046_get_rawdata(uint16_t *x, uint16_t *y);

/**
 * @brief Start run touch panel calibration
 *
 * @param screen LCD driver for display prompts
 * @param recalibrate Is calibration mandatory
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Fail
 */
esp_err_t xpt2046_calibration_run(bool recalibrate);

/**
 * @brief Start a sample for screen
 *
 * @param info a pointer of touch_panel_points_t contained touch information.
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Fail
 */
esp_err_t xpt2046_sample(touch_panel_points_t* info);


#ifdef __cplusplus
}
#endif

#endif
