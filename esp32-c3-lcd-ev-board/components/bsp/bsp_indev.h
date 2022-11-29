#pragma once

#include <stdint.h>
#include "driver/gpio.h"
#include "esp_err.h"

#define BSP_BTN_PIN_NUM         (GPIO_NUM_9)
#define BSP_ENCODER_A_PIN_NUM   (GPIO_NUM_7)
#define BSP_ENCODER_B_PIN_NUM   (GPIO_NUM_6)

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    bsp_encoder_EVENT_INC,
    bsp_encoder_EVENT_DEC,
    bsp_encoder_EVENT_MAX,
} bsp_encoder_event_t;

typedef void (* bsp_encoder_cb_t)(void *);

esp_err_t bsp_encoder_init(int gpio_a, int gpio_b);
int32_t bsp_encoder_get_value(void);
esp_err_t bsp_encoder_register_callback(bsp_encoder_event_t event, bsp_encoder_cb_t cb, void *user_data);

esp_err_t bsp_btn_init(int gpio_num);
uint8_t bsp_btn_get_state(int gpio_num);

#ifdef __cplusplus
}
#endif
