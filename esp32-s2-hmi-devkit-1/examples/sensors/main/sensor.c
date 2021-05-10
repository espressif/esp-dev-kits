/**
 * @file sensor.c
 * @brief 
 * @version 0.1
 * @date 2021-01-14
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

#include "sensor.h"

static float als_val = 0;
static float temp = 0, humid = 0;
static mpu6050_acce_value_t acc_val;
static mpu6050_gyro_value_t gyro_val;
static ext_io_t io_config = BSP_EXT_IO_DEFAULT_CONFIG();
static ext_io_t io_level = BSP_EXT_IO_DEFAULT_LEVEL();

static void sensor_task(void *data);
static esp_err_t sensor_task_start(void);

esp_err_t sensor_init(void)
{
    /* Init I2C bus for sensor communication */
    ESP_ERROR_CHECK(bsp_i2c_init(I2C_NUM_0, 400000));

    /* Turn on sensor power supply */
    ESP_ERROR_CHECK(tca9554_init());
    ESP_ERROR_CHECK(tca9554_set_configuration(io_config.val));
    ESP_ERROR_CHECK(tca9554_write_output_pins(io_level.val));
    vTaskDelay(pdMS_TO_TICKS(10));

    /* Turn on ALS */
    ESP_ERROR_CHECK(bh1750_init());
    ESP_ERROR_CHECK(bh1750_power_on());

    /* Init LCD touch panel */
    ESP_ERROR_CHECK(ft5x06_init());

    /* Init temp and humid sensor */
    ESP_ERROR_CHECK(hdc1080_init());

    /* Init MEMS sensor */
    ESP_ERROR_CHECK(mpu6050_init());

    /* Init ADC */
    ESP_ERROR_CHECK(adc081_init());
    ESP_ERROR_CHECK(adc081_config_default());

    ESP_ERROR_CHECK(sensor_task_start());

    /* Noting went wrong */
    return ESP_OK;
}

static esp_err_t sensor_task_start(void)
{
    if (pdPASS != xTaskCreate(
		(TaskFunction_t)        sensor_task,
		(const char * const)    "Sensor Task",
		(const uint32_t)        4 * 1024,
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
    bh1750_power_on();

    while (1) {
        /*!< Get MEMS data */
        mpu6050_get_acce(&acc_val);
        mpu6050_get_gyro(&gyro_val);

        /*!< Get als data */
        bh1750_get_light_intensity(BH1750_CONTINUE_4LX_RES, &als_val);

        /*!< Get temp sensor data */
        temp = hdc1080_get_temp();
        humid = hdc1080_get_humid();

        vTaskDelay(pdMS_TO_TICKS(10));
    }

    vTaskDelete(NULL);
}

void sensor_get_mems_acc(mpu6050_acce_value_t *data)
{
    *data = acc_val;
}

void sensor_get_mems_gyro(mpu6050_gyro_value_t *data)
{
    *data = gyro_val;
}

void als_get_value(float *data)
{
    *data = als_val;
}

void sensor_get_temp(float *data)
{
    *data = temp;
}

void sensor_get_humid(float *data)
{
    *data = humid;
}
