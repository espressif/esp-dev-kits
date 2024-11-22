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

#include "sensor.h"

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
