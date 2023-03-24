/**
 * @file device.c
 * @brief Initialize and get data of devices
 * @version 0.1
 * @date 2021-03-30
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
        (const char * const)    "Sensor Task",
        (const uint32_t)        3 * 1024,
        (void * const)          NULL,
        (UBaseType_t)           1,
        (TaskHandle_t * const)  NULL)) {
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
            lv_port_sem_take();
            ui_clock_set_item_val(ui_clock_item_temp, fmt_text);
            lv_port_sem_give();

            sprintf(fmt_text, "%.1f%%", humid);
            
            lv_port_sem_take();
            ui_clock_set_item_val(ui_clock_item_humid, fmt_text);
            lv_port_sem_give();
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

    *val=12;
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
