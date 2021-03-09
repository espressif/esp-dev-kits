/**
 * @file lvgl_port.h
 * @brief 
 * @version 0.1
 * @date 2021-02-24
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#pragma once

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "lvgl/lvgl.h"

#include "bsp_lcd.h"
#include "ft5x06.h"
#include "rm68120.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    size_t buffer_size;
    uint32_t buffer_alloc_caps;
} lvgl_port_init_config_t;

/**
 * @brief Initialize LVGL with config
 * 
 * @param cfg 
 * @return esp_err_t 
 */
esp_err_t lvgl_init(size_t buffer_pix_size, uint32_t buffer_caps);

/**
 * @brief Take LVGL widget update and flush semaphore
 * 
 * @return esp_err_t 
 */
esp_err_t lv_port_sem_take(void);

/**
 * @brief Give LVGL widget update and flush semaphore
 * 
 * @return esp_err_t 
 */
esp_err_t lv_port_sem_give(void);

#ifdef __cplusplus
}
#endif
