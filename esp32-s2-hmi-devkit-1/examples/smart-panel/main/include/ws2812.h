#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_log.h"

#include "bsp_board.h"

#include "driver/rmt.h"
#include "led_strip.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 
 * 
 * @return esp_err_t 
 */
esp_err_t ws2812_init(void);

/**
 * @brief 
 * 
 * @param index 
 * @param r 
 * @param g 
 * @param b 
 * @return esp_err_t 
 */
esp_err_t ws2812_set_rgb(uint8_t index, uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief 
 * 
 * @return esp_err_t 
 */
esp_err_t ws2812_refresh(void);

#ifdef __cplusplus
}
#endif
