/* Camera Example

    This example code is in the Public Domain (or CC0 licensed, at your option.)
    Unless required by applicable law or agreed to in writing, this
    software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
    CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "driver/rmt.h"
#include "led_strip.h"


#define DEFAULT_VREF    1100                            /*!< Use adc2_vref_to_gpio() to obtain a better estimate */
#define NO_OF_SAMPLES   64
#define SAMPLE_TIME     200                             /*!< Sampling time(ms) */
static const adc_channel_t channel = ADC_CHANNEL_5;     /*!< PIO7 if ADC1, GPIO17 if ADC2 */
static const adc_bits_width_t width = ADC_WIDTH_BIT_13;

static const adc_atten_t atten = ADC_ATTEN_DB_12;
static const adc_unit_t unit = ADC_UNIT_1;
static xQueueHandle adc_queue = NULL;
led_strip_t *strip;
#define DEVIATION 0.1

const char *TAG = "main";

double adc_voltage_conversion(uint32_t adc_reading)
{
    double voltage = 0;

    voltage = (2.60 * adc_reading) / 8191;

    return voltage;
}
void button_task(void *arg)
{
    /*!<Continuously sample ADC1*/
    while (1) {
        uint32_t adc_reading = 0;
        double voltage = 0;

        /*!< Multisampling */
        for (int i = 0; i < NO_OF_SAMPLES; i++) {
            if (unit == ADC_UNIT_1) {
                adc_reading += adc1_get_raw((adc1_channel_t)channel);
            } else {
                int raw;
                adc2_get_raw((adc2_channel_t)channel, width, &raw);
                adc_reading += raw;
            }
        }

        adc_reading /= NO_OF_SAMPLES;

        voltage = adc_voltage_conversion(adc_reading);
        ESP_LOGD(TAG, "ADC%d CH%d Raw: %d   ; Voltage: %0.2lfV", unit, channel, adc_reading, voltage);

        xQueueSend(adc_queue, (double *)&voltage, 0);
        vTaskDelay(pdMS_TO_TICKS(SAMPLE_TIME));
    }

    vTaskDelete(NULL);
}

void led_task(void *arg)
{
    double voltage = 0;

    while (1) {
        xQueueReceive(adc_queue, &voltage, portMAX_DELAY);

        if (voltage > 2.6) {
            continue;
        } else if (voltage > 2.41 - DEVIATION  && voltage <= 2.41 + DEVIATION) {
            ESP_LOGI(TAG, "rec(K1) -> red");
            ESP_ERROR_CHECK(strip->set_pixel(strip, 0, 255, 0, 0));
            ESP_ERROR_CHECK(strip->refresh(strip, 0));

        } else if (voltage > 1.98 - DEVIATION && voltage <= 1.98 + DEVIATION) {
            ESP_LOGI(TAG, "mode(K2) -> green");
            ESP_ERROR_CHECK(strip->set_pixel(strip, 0, 0, 255, 0));
            ESP_ERROR_CHECK(strip->refresh(strip, 0));
        } else if (voltage > 1.65 - DEVIATION && voltage <= 1.65 + DEVIATION) {
            ESP_LOGI(TAG, "play(K3) -> blue");
            ESP_ERROR_CHECK(strip->set_pixel(strip, 0, 0, 0, 255));
            ESP_ERROR_CHECK(strip->refresh(strip, 0));

        } else if (voltage > 1.11 - DEVIATION && voltage <= 1.11 + DEVIATION) {
            ESP_LOGI(TAG, "set(K4) -> yellow");
            ESP_ERROR_CHECK(strip->set_pixel(strip, 0, 255, 255, 0));
            ESP_ERROR_CHECK(strip->refresh(strip, 0));

        } else if (voltage > 0.82 - DEVIATION && voltage <= 0.82 + DEVIATION) {

            ESP_LOGI(TAG, "vol(K5) -> purple");
            ESP_ERROR_CHECK(strip->set_pixel(strip, 0, 255, 0, 255));
            ESP_ERROR_CHECK(strip->refresh(strip, 0));

        } else if (voltage > 0.38 - DEVIATION && voltage <= 0.38 + DEVIATION) {

            ESP_LOGI(TAG, "vol+(K6) -> write");
            ESP_ERROR_CHECK(strip->set_pixel(strip, 0, 255, 255, 255));
            ESP_ERROR_CHECK(strip->refresh(strip, 0));
        }

    }

    vTaskDelete(NULL);

}

void adc_init(void)
{
    if (unit == ADC_UNIT_1) {
        adc1_config_width(width);
        adc1_config_channel_atten(channel, atten);
    } else {
        adc2_config_channel_atten((adc2_channel_t)channel, atten);
    }

}

esp_err_t example_rmt_init(uint8_t gpio_num, int led_number, uint8_t rmt_channel)
{
    ESP_LOGI(TAG, "Initializing WS2812");
    rmt_config_t config = RMT_DEFAULT_CONFIG_TX(gpio_num, rmt_channel);

    /*!< set counter clock to 40MHz */
    config.clk_div = 2;

    ESP_ERROR_CHECK(rmt_config(&config));
    ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));

    led_strip_config_t strip_config = LED_STRIP_DEFAULT_CONFIG(led_number, (led_strip_dev_t)config.channel);
    strip = led_strip_new_rmt_ws2812(&strip_config);

    if (!strip) {
        ESP_LOGE(TAG, "install WS2812 driver failed");
        return ESP_FAIL;
    }

    /*!< Clear LED strip (turn off all LEDs) */
    ESP_ERROR_CHECK(strip->clear(strip, 100));
    /*!< Show simple rainbow chasing pattern */

    return ESP_OK;
}

void app_main()
{
    /*!< WS2812 init*/
    ESP_ERROR_CHECK(example_rmt_init(CONFIG_EXAMPLE_RMT_TX_GPIO, CONFIG_EXAMPLE_STRIP_LED_NUMBER, RMT_CHANNEL_0));

    adc_queue = xQueueCreate(1, sizeof(double));
    adc_init();
    xTaskCreatePinnedToCore(&button_task, "button_task", 3 * 1024, NULL, 5, NULL, 0);
    xTaskCreatePinnedToCore(&led_task, "led_task", 3 * 1024, NULL, 5, NULL, 0);
}

