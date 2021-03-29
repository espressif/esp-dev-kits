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

#include "bsp_ext_io.h"
#include "bh1750.h"
#include "cat5171.h"
#include "hdc1080.h"
#include "mpu6050.h"
#include "tca9554.h"
#include "ws2812.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize power evaluation UI
 * 
 */
void ui_power_init(void);

#ifdef __cplusplus
}
#endif
