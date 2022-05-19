/**
 * @file ws2812.c
 * @brief WS2812 RGB LED APIs.
 * @version 0.1
 * @date 2021-03-07
 *
 * @copyright Copyright 2021 Espressif Systems (Shanghai) Co. Ltd.
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *               http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/rmt.h"
#include "driver/ledc.h"
#include "esp_check.h"
#include "esp_log.h"
#include "led_strip.h"
#include "bsp_board.h"

static const char *TAG = "bsp led";

#define LEDPWM_CNT_TOP 256
static uint8_t g_gamma_table[LEDPWM_CNT_TOP + 1];

#define INIT_RMT_LED BIT(0)
#define INIT_PWM_LED BIT(1)
static uint8_t g_initialized = 0;
static led_strip_t *strip = NULL;

static esp_err_t bsp_pwm_led_init(gpio_num_t gpio_r, gpio_num_t gpio_g, gpio_num_t gpio_b)
{
    esp_err_t ret_val = ESP_OK;

    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .timer_num        = LEDC_TIMER_0,
        .duty_resolution  = LEDC_TIMER_8_BIT,
        .freq_hz          = 8192,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ret_val |= ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel_red = {
        .speed_mode     = LEDC_LOW_SPEED_MODE,
        .channel        = LEDC_CHANNEL_0,
        .timer_sel      = LEDC_TIMER_0,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = gpio_r,
        .duty           = 0,
        .hpoint         = 0
    };
    ret_val |= ledc_channel_config(&ledc_channel_red);

    ledc_channel_config_t ledc_channel_green = {
        .speed_mode     = LEDC_LOW_SPEED_MODE,
        .channel        = LEDC_CHANNEL_1,
        .timer_sel      = LEDC_TIMER_0,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = gpio_g,
        .duty           = 0,
        .hpoint         = 0
    };
    ret_val |= ledc_channel_config(&ledc_channel_green);

    ledc_channel_config_t ledc_channel_blue = {
        .speed_mode     = LEDC_LOW_SPEED_MODE,
        .channel        = LEDC_CHANNEL_2,
        .timer_sel      = LEDC_TIMER_0,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = gpio_b,
        .duty           = 0,
        .hpoint         = 0
    };
    ret_val |= ledc_channel_config(&ledc_channel_blue);

    return ret_val;
}

static esp_err_t bsp_pwm_led_deinit(void)
{
    ledc_stop(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
    ledc_stop(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 0);
    ledc_stop(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, 0);
    ledc_timer_rst(LEDC_LOW_SPEED_MODE, LEDC_TIMER_0);
    return ESP_OK;
}

esp_err_t bsp_led_init(void)
{
    ESP_RETURN_ON_FALSE(0 == g_initialized, ESP_ERR_INVALID_STATE, TAG,  "led already initialized");

    esp_err_t ret = ESP_OK;
    const board_res_desc_t *brd = bsp_board_get_description();
    if (brd->FUNC_RMT_EN && GPIO_NUM_NC != brd->GPIO_RMT_LED) { //for s3-lcd-evb
        strip = led_strip_init(RMT_CHANNEL_0, brd->GPIO_RMT_LED, brd->RMT_LED_NUM);
        g_initialized |= INIT_RMT_LED;
        ret = NULL == strip ? ESP_FAIL : ESP_OK;
    }
    if (brd->PMOD2) { // for s3-box and s3-box-lite
        ret = bsp_pwm_led_init(brd->PMOD2->row1[1], brd->PMOD2->row1[2], brd->PMOD2->row1[3]);
        g_initialized |= INIT_PWM_LED;
    }

    // Generate gamma correction table
    for (int i = 0; i < (LEDPWM_CNT_TOP + 1); i++) {
        g_gamma_table[i] = (int)roundf(powf((float)i / (float)LEDPWM_CNT_TOP, 1.0 / 0.45) * (float)LEDPWM_CNT_TOP);
    }
    return ret;
}

esp_err_t bsp_led_deinit(void)
{
    if (g_initialized & INIT_RMT_LED) {
        led_strip_deinit(strip);
    }
    if (g_initialized & INIT_PWM_LED) {
        bsp_pwm_led_deinit();
    }
    return ESP_OK;
}

esp_err_t bsp_pwm_led_change_io(gpio_num_t gpio_r, gpio_num_t gpio_g, gpio_num_t gpio_b)
{
    bsp_pwm_led_deinit();
    return bsp_pwm_led_init(gpio_r, gpio_g, gpio_b);
}

esp_err_t bsp_led_set_rgb(uint8_t index, uint8_t r, uint8_t g, uint8_t b)
{
    r = g_gamma_table[r];
    g = g_gamma_table[g];
    b = g_gamma_table[b];

    if (g_initialized & INIT_RMT_LED) {
        ESP_RETURN_ON_FALSE(NULL != strip, ESP_ERR_INVALID_STATE, TAG,  "led not initialized");
        strip->set_pixel(strip, index, r, g, b);
        return strip->refresh(strip, pdMS_TO_TICKS(100));
    }
    if (g_initialized & INIT_PWM_LED) {
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, r);
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, g);
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, b);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2);
    }
    return ESP_OK;
}
