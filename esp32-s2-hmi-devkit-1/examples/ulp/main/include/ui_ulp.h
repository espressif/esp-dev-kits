#pragma once

#include <stdio.h>

#include "driver/gpio.h"
#include "driver/rtc_io.h"

#include "esp_sleep.h"
#include "esp32/ulp.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "nvs.h"
#include "nvs_flash.h"

#include "soc/rtc_cntl_reg.h"
#include "soc/sens_reg.h"
#include "soc/rtc_periph.h"

#include "lvgl/lvgl.h"
#include "lvgl_port.h"

#include "ws2812.h"

#include "tca9554.h"

#include "bsp_ext_io.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize ulp evaluation UI
 * 
 */
void ui_ulp_init(void);

#ifdef __cplusplus
}
#endif
