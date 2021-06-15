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

#include "device.h"
#include "ui_main.h"

static float temp = 0, humid = 0;
static volatile bool b_sensor_data_update = true;

static void sensor_task(void *data);

esp_err_t device_init(void)
{
    /* Init I2C bus for sensor communication */
    ESP_ERROR_CHECK(bsp_i2c_init(I2C_NUM_0, 400000));

    /* Turn on sensor power supply */
    ESP_ERROR_CHECK(tca9554_init());
    ext_io_t io_level = BSP_EXT_IO_DEFAULT_LEVEL();
    ext_io_t io_config = BSP_EXT_IO_DEFAULT_CONFIG();
    ESP_ERROR_CHECK(tca9554_write_output_pins(io_level.val));
    ESP_ERROR_CHECK(tca9554_set_configuration(io_config.val));

    /* Wait for power stable */
    vTaskDelay(pdMS_TO_TICKS(30));

    /* Init ADC */
    ESP_ERROR_CHECK(adc081_init());
    ESP_ERROR_CHECK(adc081_config_default());

    /* Init LCD touch panel */
    ESP_ERROR_CHECK(ft5x06_init());

    /* Init temp and humid sensor */
    ESP_ERROR_CHECK(hdc1080_init());

    /* Volume digital potentiometer */
    ESP_ERROR_CHECK(tpl0401_init());

    ws2812_init();

    /* Noting went wrong */
    return ESP_OK;
}

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

float hdc1080_get_temp(void)
{
    uint8_t data[2];
    hdc1080_start_measure(hdc1080_measure_temp);

    /* *******************************************
     * You must wait for hdc1080 convertion done *
     * Temperature convertion time in spec:      *
     *  11 Bit  3.65 ms                          *
     *  14 Bit  6.35 ms                          *
     * *******************************************/
    vTaskDelay(pdMS_TO_TICKS(10));
    
    hdc1080_get_measure_data(data);

    return (float)((data[0] << 8) + data [1]) / 65535 * 165 - 40;
}

float hdc1080_get_humid(void)
{
    uint8_t data[2];
    hdc1080_start_measure(hdc1080_measure_humid);

    /* *******************************************
     * You must wait for hdc1080 convertion done *
     * Humidity convertion time in spec:         *
     *  8 Bit   2.5 ms                           *
     *  11 Bit  3.85 ms                          *
     *  14 Bit  6.5 ms                           *
     * *******************************************/
    vTaskDelay(pdMS_TO_TICKS(10));
    
    hdc1080_get_measure_data(data);

    return (float)((data[0] << 8) + data [1]) / 65535 * 100;
}

static void sensor_task(void *data)
{
    static char fmt_text[16];

    while (1) {

        if (b_sensor_data_update) {
            /* Get temp sensor data */
            temp = hdc1080_get_temp();
            humid = hdc1080_get_humid();

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

    adc081_get_converted_value(val);
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
