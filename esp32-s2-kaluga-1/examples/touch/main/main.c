/*!< Touch  Example
 *
 *  This example code is in the Public Domain (or CC0 licensed, at your option.)
 *
 *  Unless required by applicable law or agreed to in writing, this
 *  software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 *  CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/touch_pad.h"
#include "soc/rtc_periph.h"
#include "soc/sens_periph.h"
#include "driver/rmt.h"
#include "led_strip.h"

#define TOUCH_BUTTON_WATERPROOF_ENABLE 1
#define TOUCH_BUTTON_DENOISE_ENABLE    1
#define TOUCH_CHANGE_CONFIG            0
#define TOUCH_BUTTON_NUM    7

#ifdef CONFIG_TOUCH_PAD_ESP32_S2_KALUGA_V1_2
#define TOUCH_BUTTON_PLAY     TOUCH_PAD_NUM2
#define TOUCH_BUTTON_PHOTO    TOUCH_PAD_NUM6
#define TOUCH_BUTTON_NETWORK  TOUCH_PAD_NUM11
#define TOUCH_BUTTON_RECORD   TOUCH_PAD_NUM5
#define TOUCH_BUTTON_VOLUP    TOUCH_PAD_NUM1
#define TOUCH_BUTTON_VOLDOWN  TOUCH_PAD_NUM3
#define TOUCH_BUTTON_GUARD    TOUCH_PAD_NUM4

#elif defined (CONFIG_TOUCH_PAD_ESP32_S2_KALUGA_V1_1)
#define TOUCH_BUTTON_PLAY     TOUCH_PAD_NUM3
#define TOUCH_BUTTON_PHOTO    TOUCH_PAD_NUM8
#define TOUCH_BUTTON_NETWORK  TOUCH_PAD_NUM11
#define TOUCH_BUTTON_RECORD   TOUCH_PAD_NUM13
#define TOUCH_BUTTON_VOLUP    TOUCH_PAD_NUM2
#define TOUCH_BUTTON_VOLDOWN  TOUCH_PAD_NUM9
#define TOUCH_BUTTON_GUARD    TOUCH_PAD_NUM4
#endif

#define RMT_TX_CHANNEL RMT_CHANNEL_0
#define LEDC_COLOR 200        /*!< Initializes the led light value （0～255）*/
#define LEDC_RANGE 10         /*!< Light adjustment amplitude */

static const char *TAG = "Touch pad";

static QueueHandle_t que_touch = NULL;
typedef struct touch_msg {
    touch_pad_intr_mask_t intr_mask;
    uint32_t pad_num;
    uint32_t pad_status;
    uint32_t pad_val;
} touch_event_t;

typedef enum {
    LED_NOOE = 0,
    LED_RED,
    LED_GREEN,
    LED_BLUE,
    LED_MAX,
} type_lcd_t;

led_strip_t *strip;


static const touch_pad_t button[TOUCH_BUTTON_NUM] = {
    TOUCH_BUTTON_PHOTO,      /*!< 'PHOTO' button */
    TOUCH_BUTTON_PLAY,       /*!< 'PLAY/PAUSE' button */
    TOUCH_BUTTON_NETWORK,    /*!< 'NETWORK' button */
    TOUCH_BUTTON_RECORD,     /*!< 'RECORD' button */
    TOUCH_BUTTON_VOLUP,      /*!< 'VOL_UP' button */
    TOUCH_BUTTON_VOLDOWN,    /*!< 'VOL_DOWN' button */
    TOUCH_BUTTON_GUARD,      /*!< Guard ring for waterproof design. */
    /*!< If this pad be touched, other pads no response. */
};

/*!<
 * Touch threshold. The threshold determines the sensitivity of the touch.
 * This threshold is derived by testing changes in readings from different touch channels.
 * If (raw_data - baseline) > baseline * threshold, the pad be activated.
 * If (raw_data - baseline) < baseline * threshold, the pad be inactivated.
 */
static const float button_threshold[TOUCH_BUTTON_NUM] = {
    0.01,     /*!< threshold = 1% */
    0.01,
    0.01,
    0.01,
    0.01,
    0.01,
    0.01,
};

/*!<
  Handle an interrupt triggered when a pad is touched.
  Recognize what pad has been touched and save it in a table.
 */
static void touchsensor_interrupt_cb(void *arg)
{
    int task_awoken = pdFALSE;
    touch_event_t evt;

    evt.intr_mask = touch_pad_read_intr_status_mask();
    evt.pad_status = touch_pad_get_status();
    evt.pad_num = touch_pad_get_current_meas_channel();

    if (evt.intr_mask & TOUCH_PAD_INTR_MASK_DONE) {
        touch_pad_filter_read_baseline(evt.pad_num, &evt.pad_val);
    }

    xQueueSendFromISR(que_touch, &evt, &task_awoken);

    if (task_awoken == pdTRUE) {
        portYIELD_FROM_ISR();
    }
}

static void tp_example_set_thresholds(void)
{
    uint32_t touch_value;

    for (int i = 0; i < TOUCH_BUTTON_NUM; i++) {
        /*!< read baseline value */
        touch_pad_filter_read_baseline(button[i], &touch_value);
        /*!< set interrupt threshold. */
        touch_pad_set_thresh(button[i], touch_value * button_threshold[i]);
        ESP_LOGI(TAG, "touch pad [%d] base %d, thresh %d", \
                 button[i], touch_value, (uint32_t)(touch_value * button_threshold[i]));
    }
}

static void touchsensor_filter_set(touch_filter_mode_t mode)
{
    /*!< Filter function */
    touch_filter_config_t filter_info = {
        .mode = mode,           /*!< Test jitter and filter 1/4. */
        .debounce_cnt = 1,      /*!< 1 time count. */
        .hysteresis_thr = 3,    /*!< 3% */
        .noise_thr = 0,         /*!< 50% */
        .noise_neg_thr = 0,     /*!< 50% */
        .neg_noise_limit = 10,  /*!< 10 time count. */
        .jitter_step = 4,       /*!< use for jitter mode. */
    };
    touch_pad_filter_set_config(&filter_info);
    touch_pad_filter_enable();
    ESP_LOGI(TAG, "touch pad filter init");
}

static void touch_pad_read_task(void *pvParameter)
{
    touch_event_t evt = {0};
    uint8_t red   = 0;
    uint8_t green = 0;
    uint8_t blue  = 0;
    type_lcd_t flag  = LED_NOOE;

    /*!< Wait touch sensor init done */
    vTaskDelay(100 / portTICK_RATE_MS);
    tp_example_set_thresholds();

    while (1) {
        int ret = xQueueReceive(que_touch, &evt, (portTickType)portMAX_DELAY);

        if (ret != pdTRUE || (evt.intr_mask & TOUCH_PAD_INTR_MASK_ACTIVE) == false) {
            continue;
        }

        /*!< if guard pad be touched, other pads no response. */
        switch (evt.pad_num) {
            case TOUCH_BUTTON_PHOTO:
                ESP_LOGI(TAG, "photo    -> set the red light");
                ESP_ERROR_CHECK(strip->set_pixel(strip, 0, LEDC_COLOR, 0, 0));
                flag  = LED_RED;
                red   = LEDC_COLOR;
                green = 0;
                blue  = 0;
                ESP_ERROR_CHECK(strip->refresh(strip, 0));
                break;

            case TOUCH_BUTTON_PLAY:
                ESP_LOGI(TAG, "play     -> set the green light");
                ESP_ERROR_CHECK(strip->set_pixel(strip, 0, 0, LEDC_COLOR, 0));
                flag  = LED_GREEN;
                red   = 0;
                green = LEDC_COLOR;
                blue  = 0;
                ESP_ERROR_CHECK(strip->refresh(strip, 0));
                break;

            case TOUCH_BUTTON_NETWORK:
                ESP_LOGI(TAG, "network  -> set the blue light");
                ESP_ERROR_CHECK(strip->set_pixel(strip, 0, 0, 0, LEDC_COLOR));
                flag  = LED_BLUE;
                red   = 0;
                green = 0;
                blue  = LEDC_COLOR;
                ESP_ERROR_CHECK(strip->refresh(strip, 0));
                break;

            case TOUCH_BUTTON_RECORD:
                ESP_LOGI(TAG, "record   -> shut down the light");
                strip->clear(strip, 0);
                flag = LED_NOOE;
                break;

            case TOUCH_BUTTON_VOLUP:
                if (flag) {
                    int value = 0;

                    if (flag == LED_RED) {
                        red = (red + LEDC_RANGE > 255) ? 255 : red + LEDC_RANGE;
                        ESP_ERROR_CHECK(strip->set_pixel(strip, 0, red, 0, 0));
                        value = red;
                    } else if (flag == LED_GREEN) {
                        green = (green + LEDC_RANGE > 255) ? 255 : green + LEDC_RANGE;
                        ESP_ERROR_CHECK(strip->set_pixel(strip, 0, 0, green, 0));
                        value = green;
                    } else if (flag == LED_BLUE) {
                        blue = (blue + LEDC_RANGE > 255) ? 255 : blue + LEDC_RANGE;
                        ESP_ERROR_CHECK(strip->set_pixel(strip, 0, 0, 0, blue));
                        value = blue;
                    }

                    ESP_LOGI(TAG, "vol_up   -> make the light brighter:%d", value);
                    ESP_ERROR_CHECK(strip->refresh(strip, 0));
                }

                break;

            case TOUCH_BUTTON_VOLDOWN:
                if (flag) {
                    int value = 0;

                    if (flag == LED_RED) {
                        red = (red - LEDC_RANGE < 0) ? 0 : red - LEDC_RANGE;
                        ESP_ERROR_CHECK(strip->set_pixel(strip, 0, red, 0, 0));
                        value = red;
                    } else if (flag == LED_GREEN) {
                        green = (green - LEDC_RANGE < 0) ? 0 : green - LEDC_RANGE;
                        ESP_ERROR_CHECK(strip->set_pixel(strip, 0, 0, green, 0));
                        value = green;
                    } else if (flag == LED_BLUE) {
                        blue = (blue - LEDC_RANGE < 0) ? 0 : blue - LEDC_RANGE;
                        ESP_ERROR_CHECK(strip->set_pixel(strip, 0, 0, 0, blue));
                        value = blue;
                    }

                    ESP_LOGI(TAG, "vol_down -> make the light darker:%d", value);
                    ESP_ERROR_CHECK(strip->refresh(strip, 0));
                }

                break;

            default:
                ESP_LOGI(TAG, "ERROR\n");
                break;
        }

    }

    vTaskDelete(NULL);
}

/**
  * @brief Initialize the WB2812
  *
  * @param gpio_num: GPIO of WB2812
  * @param led_number: The number of leds
  * @param rmt_channel: RMT channel number
  *
  * @return
  *      - ESP_OK: Initialize successfully
  *      - ESP_FAIL: Initialize fail
  */
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

/**
  * @brief Initialize the touch pad
  *
  * @return
  *      - ESP_OK: Initialize successfully
  *      - ESP_FAIL: Initialize fail
  */
esp_err_t example_touch_init(void)
{

    if (que_touch == NULL) {
        que_touch = xQueueCreate(TOUCH_BUTTON_NUM, sizeof(touch_event_t));
    }

    /*!< Initialize touch pad peripheral, it will start a timer to run a filter */
    ESP_LOGI(TAG, "Initializing touch pad");
    /*!< Initialize touch pad peripheral. */
    touch_pad_init();

    for (int i = 0; i < TOUCH_BUTTON_NUM; i++) {
        touch_pad_config(button[i]);
    }

#if TOUCH_CHANGE_CONFIG
    /*!< If you want change the touch sensor default setting, please write here(after initialize). There are examples: */
    touch_pad_set_meas_time(TOUCH_PAD_SLEEP_CYCLE_DEFAULT, TOUCH_PAD_SLEEP_CYCLE_DEFAULT);
    touch_pad_set_voltage(TOUCH_PAD_HIGH_VOLTAGE_THRESHOLD, TOUCH_PAD_LOW_VOLTAGE_THRESHOLD, TOUCH_PAD_ATTEN_VOLTAGE_THRESHOLD);
    touch_pad_set_inactive_connect(TOUCH_PAD_INACTIVE_CONNECT_DEFAULT);

    for (int i = 0; i < TOUCH_BUTTON_NUM; i++) {
        touch_pad_set_cnt_mode(i, TOUCH_PAD_SLOPE_DEFAULT, TOUCH_PAD_TIE_OPT_DEFAULT);
    }

#endif

#if TOUCH_BUTTON_DENOISE_ENABLE
    /*!< Denoise setting at TouchSensor 0. */
    touch_pad_denoise_t denoise = {
        /*!< The bits to be cancelled are determined according to the noise level. */
        .grade     = TOUCH_PAD_DENOISE_BIT4,
        .cap_level = TOUCH_PAD_DENOISE_CAP_L4,
    };
    touch_pad_denoise_set_config(&denoise);
    touch_pad_denoise_enable();
    ESP_LOGI(TAG, "Denoise function init");
#endif

#if TOUCH_BUTTON_WATERPROOF_ENABLE
    /*!< Waterproof function */
    touch_pad_waterproof_t waterproof = {
        .guard_ring_pad = TOUCH_BUTTON_GUARD,       /*!< If no ring pad, set 0; */
        /*!< It depends on the number of the parasitic capacitance of the shield pad. */
        .shield_driver  = TOUCH_PAD_SHIELD_DRV_L0,  /*!< 40pf */
    };
    touch_pad_waterproof_set_config(&waterproof);
    touch_pad_waterproof_enable();
    ESP_LOGI(TAG, "touch pad waterproof init");
#endif

    /*!< Filter setting */
    touchsensor_filter_set(TOUCH_PAD_FILTER_IIR_8);
    /*!< Register touch interrupt ISR, enable intr type. */
    touch_pad_isr_register(touchsensor_interrupt_cb, NULL, TOUCH_PAD_INTR_MASK_ALL);
    touch_pad_intr_enable(TOUCH_PAD_INTR_MASK_ACTIVE | TOUCH_PAD_INTR_MASK_INACTIVE);

    /*!< Enable touch sensor clock. Work mode is "timer trigger". */
    touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);
    touch_pad_fsm_start();

    return ESP_OK;
}

void app_main(void)
{
    /*!< Initialize the WS2812 */
    ESP_ERROR_CHECK(example_rmt_init(CONFIG_EXAMPLE_RMT_TX_GPIO, CONFIG_EXAMPLE_STRIP_LED_NUMBER, RMT_CHANNEL_0));
    /*!< Initialize the touch pad */
    ESP_ERROR_CHECK(example_touch_init());

    /*!< Start a task to show what pads have been touched */
    xTaskCreate(&touch_pad_read_task, "touch_pad_read_task", 2048, NULL, 5, NULL);
}
