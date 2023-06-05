/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "esp_err.h"
#include "esp_log.h"

#include "device.h"
#include "ui_main.h"

static float temp = 0, humid = 0;
static volatile bool b_sensor_data_update = true;

static void sensor_task(void *data);


esp_err_t sensor_task_start(void)
{
    if (pdPASS != xTaskCreate(
                (TaskFunction_t)        sensor_task,
                (const char *const)    "Sensor Task",
                (const uint32_t)        3 * 1024,
                (void *const)          NULL,
                (UBaseType_t)           1,
                (TaskHandle_t *const)  NULL)) {
        return ESP_FAIL;
    }

    return ESP_OK;
}


static void sensor_task(void *data)
{
    static char fmt_text[16];

    while (1) {

        if (b_sensor_data_update) {
            /* Get temp sensor data */
            temp = 11;
            humid = 22;

            /* Update clock page */
            sprintf(fmt_text, "%.1f°C", temp);

            /* Update data in clock page if network not connected */
            bsp_display_lock(0);
            ui_clock_set_item_val(ui_clock_item_temp, fmt_text);
            bsp_display_unlock();

            sprintf(fmt_text, "%.1f%%", humid);

            bsp_display_lock(0);
            ui_clock_set_item_val(ui_clock_item_humid, fmt_text);
            bsp_display_unlock();
        } else {
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }

    vTaskDelete(NULL);
}

void sensor_get_temp(float *data)
{
    *data = temp;
}

void sensor_get_humid(float *data)
{
    *data = humid;
}

void sensor_data_update_stop(void)
{
    b_sensor_data_update = false;
}

void adc_bat_get_raw(uint8_t *val)
{
    if (NULL == val) {
        return;
    }

    *val = 12;
}

void adc_bat_get_voltage(float *voltage)
{
    if (NULL == voltage) {
        return;
    }

    uint8_t adc_val = 0;
    adc_bat_get_raw(&adc_val);
    *voltage = 3.3f * 4 * adc_val / 255;
}
