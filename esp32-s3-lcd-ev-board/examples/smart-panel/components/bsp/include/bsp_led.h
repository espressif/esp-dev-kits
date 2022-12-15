#pragma once


#include "esp_err.h"
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Initialize leds
 * 
 * @return 
 *    - ESP_OK: Success
 *    - ESP_ERR_INVALID_STATE: Already initialized
 */
esp_err_t bsp_led_init(void);

/**
 * @brief Deinitialize leds
 * 
 * @return 
 *    - ESP_OK: Success
 */
esp_err_t bsp_led_deinit(void);

/**
 * @brief Modify gpio for pwm leds
 * 
 * @param gpio_r gpio connected to red LED
 * @param gpio_g gpio connected to green LED
 * @param gpio_b gpio connected to blue LED
 * @return 
 *    - ESP_OK: Success
 *    - Others: Fail
 */
esp_err_t bsp_pwm_led_change_io(gpio_num_t gpio_r, gpio_num_t gpio_g, gpio_num_t gpio_b);

/**
 * @brief Set RGB value to LEDs
 * 
 * @note This function will set LED after gamma correction of the input value
 * 
 * @param index LED index you want to set
 * @param r value of red color
 * @param g value of green color
 * @param b value of blue color
 * @return 
 *    - ESP_OK: Success
 *    - Others: Fail
 */
esp_err_t bsp_led_set_rgb(uint8_t index, uint8_t r, uint8_t g, uint8_t b);

#ifdef __cplusplus
}
#endif
