#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_log.h"

#include "bsp_board.h"

#include "driver/rmt.h"
#include "led_strip.h"

esp_err_t ws2812_init(void);
esp_err_t ws2812_set_rgb(uint8_t index, uint8_t r, uint8_t g, uint8_t b);
esp_err_t ws2812_refresh(void);

void ws2812_set_r(uint8_t r);
void ws2812_set_g(uint8_t g);
void ws2812_set_b(uint8_t b);

uint8_t ws2812_get_r(void);
uint8_t ws2812_get_g(void);
uint8_t ws2812_get_b(void);
