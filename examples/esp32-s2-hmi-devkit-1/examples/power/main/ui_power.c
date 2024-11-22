/**
 * @file ui_power.c
 * @brief UI src.
 * @version 0.1
 * @date 2021-03-07
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

#include "ui_power.h"

/* Log defination */
#define TAG                 ((const char *) "ui_power")
#define UI_LOG_TRACE(...)	ESP_LOGI(TAG, ##__VA_ARGS__)

static lv_obj_t *btn_sleep = NULL;
static lv_obj_t *bar_bat_voltage = NULL;
static lv_obj_t *slider_led[3];
static lv_obj_t *label_bat_voltage = NULL;
static lv_obj_t *label_led = NULL;
static lv_obj_t *led[8] = { [0 ... 7] = NULL };
static lv_obj_t *sw_ext_io[8] = { [0 ... 7] = NULL };
static const char *sw_ext_io_name[] = {
    "ICM-20600",
    "LCD RST",
    "Touch INT",
    "Audio PA",
    "3.3V Off",
    "5V Booster",
    "WS2812 CSH",
    "LCD Light", };

static void btn_sleep_cb(lv_obj_t *obj, lv_event_t event)
{
    gpio_num_t gpio_num = GPIO_NUM_0;
    static ext_io_t io_level_sleep = BSP_EXT_IO_SLEEP_LEVEL();

    if (LV_EVENT_CLICKED == event) {
        /* Config wakeup pin mode and default level */
        rtc_gpio_init(gpio_num);
        rtc_gpio_set_direction(gpio_num, RTC_GPIO_MODE_INPUT_ONLY);
        rtc_gpio_pulldown_dis(gpio_num);
        rtc_gpio_pullup_en(gpio_num);
        rtc_gpio_hold_en(gpio_num);

        /* Config wakeup pin(s) */
        esp_sleep_enable_ext1_wakeup(1ULL << gpio_num, ESP_EXT1_WAKEUP_ALL_LOW);

        mpu6050_sleep();
        
        bh1750_power_down();
        
        /* Shut down all programmable power */
        tca9554_write_output_pins(io_level_sleep.val);

        /* Wait IO status written to IO expander */
        vTaskDelay(pdMS_TO_TICKS(10));

        /* Enter deep sleep */
        esp_deep_sleep_start();
    }
}

static void sw_ext_io_cb(lv_obj_t *obj, lv_event_t event)
{
    static ext_io_t io_level = BSP_EXT_IO_DEFAULT_LEVEL();

    if (LV_EVENT_VALUE_CHANGED == event) {
        int sw_index = (size_t) lv_obj_get_user_data(obj);

        if (lv_switch_get_state(obj)) {
            io_level.val |= 1 << sw_index;
        } else {
            io_level.val &= ~(1 << sw_index);
        }

        tca9554_write_output_pins(io_level.val);
    }
}

static void slider_led_cb(lv_obj_t *obj, lv_event_t event)
{
    static char fmt_text[3][8];

    if (LV_EVENT_VALUE_CHANGED == event) {
        ws2812_set_rgb(0,
            lv_slider_get_value(slider_led[0]),
            lv_slider_get_value(slider_led[1]),
            lv_slider_get_value(slider_led[2]));
        ws2812_refresh();

        int slider_id = (size_t) lv_obj_get_user_data(obj);
        sprintf(fmt_text[slider_id], "%d", lv_slider_get_value(obj));
        lv_obj_set_style_local_value_str(slider_led[slider_id], LV_SLIDER_PART_BG, LV_STATE_DEFAULT, fmt_text[slider_id]);
    }
}

static void ext_io_update_task(lv_task_t *task)
{
    static ext_io_t input_level;

    tca9554_read_input_pins(&input_level.val);
    for (size_t i = 0; i < 8; i++) {
        if (input_level.val & (1 << i)) {
            lv_led_on(led[i]);
        } else {
            lv_led_off(led[i]);
        }
    }
}

static void bat_voltage_update_task(lv_task_t *task)
{
    static int count = 0;
    static float voltage = 0;
    static uint8_t adc_val = 0;
    static char fmt_text[8] = "0.00V";

    count++;
    if (count <= 10) {
        adc081_get_converted_value(&adc_val);
        voltage += adc_val * 3.3f * 4 / 255.0f;
    } else {
        voltage /= 10.0f;
        sprintf(fmt_text, "%.2fV", voltage);
        lv_bar_set_value(bar_bat_voltage, 100 * voltage, LV_ANIM_ON);
        lv_obj_set_style_local_value_str(bar_bat_voltage, LV_BAR_PART_BG, LV_STATE_DEFAULT, fmt_text);
        count = 0;
        voltage = 0;
    }

}

void ui_power_init(void)
{
    lv_port_sem_take();

    /* Get de*/
    static ext_io_t ext_io_config;
    tca9554_read_output_pins(&ext_io_config.val);
    UI_LOG_TRACE("IO expander level : [0x%02X]", ext_io_config.val);

    /* Create a button to enter sleep mode */
    btn_sleep = lv_btn_create(lv_scr_act(), NULL);
    lv_obj_align(btn_sleep, NULL, LV_ALIGN_IN_RIGHT_MID, -180, 160);
    lv_obj_set_style_local_value_str(btn_sleep, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "Sleep");
    lv_obj_set_style_local_value_font(btn_sleep, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, &font_en_24);
    lv_obj_set_event_cb(btn_sleep, btn_sleep_cb);

    /* Create switches to control power and LEDs to show input level */
    for (size_t i = 0; i < sizeof(sw_ext_io) / sizeof(sw_ext_io[0]); i++) {
        sw_ext_io[i] = lv_switch_create(lv_scr_act(), NULL);
        lv_obj_set_width(sw_ext_io[i], 60);
        lv_obj_set_ext_click_area(sw_ext_io[i], 50, 100, 20, 20);
        lv_obj_set_pos(sw_ext_io[i], 100, 50 * (i + 1));
        lv_obj_set_event_cb(sw_ext_io[i], sw_ext_io_cb);
        lv_obj_set_user_data(sw_ext_io[i], (lv_obj_user_data_t) i);
        lv_obj_set_style_local_value_str(sw_ext_io[i], LV_SWITCH_PART_BG, LV_STATE_DEFAULT, sw_ext_io_name[i]);
        lv_obj_set_style_local_value_font(sw_ext_io[i], LV_SWITCH_PART_BG, LV_STATE_DEFAULT, &font_en_24);
        lv_obj_set_style_local_value_align(sw_ext_io[i], LV_SWITCH_PART_BG, LV_STATE_DEFAULT, LV_ALIGN_OUT_RIGHT_MID);
        lv_obj_set_style_local_value_ofs_x(sw_ext_io[i], LV_SWITCH_PART_BG, LV_STATE_DEFAULT, 5);
        if (ext_io_config.val & (1 << i)) {
            lv_switch_on(sw_ext_io[i], LV_ANIM_ON);
        } else {
            lv_switch_off(sw_ext_io[i], LV_ANIM_ON);
        }

        led[i] = lv_led_create(lv_scr_act(), NULL);
        lv_obj_set_size(led[i], 30, 30);
        lv_obj_align(led[i], sw_ext_io[i], LV_ALIGN_OUT_LEFT_MID, -30, 0);
    }

    /* Some of power domain control is not recommanded by UI, disable them */
    lv_obj_set_state(sw_ext_io[1], LV_STATE_DISABLED);
    lv_obj_set_state(sw_ext_io[2], LV_STATE_DISABLED);
    lv_obj_set_state(sw_ext_io[4], LV_STATE_DISABLED);
    lv_obj_set_state(sw_ext_io[7], LV_STATE_DISABLED);
    

    /* Create a task to show input level */
    lv_task_create(ext_io_update_task, 100, 1, NULL);

    /* Create sliders to set LCD backlight */
    bar_bat_voltage = lv_bar_create(lv_scr_act(), NULL);
    lv_bar_set_range(bar_bat_voltage, 300, 450);
    lv_bar_set_value(bar_bat_voltage, 0, LV_ANIM_ON);
    lv_obj_align(bar_bat_voltage, NULL, LV_ALIGN_CENTER, 150, -120);
    lv_obj_set_style_local_value_str(bar_bat_voltage, LV_BAR_PART_BG, LV_STATE_DEFAULT, "0.00V");
    lv_obj_set_style_local_value_font(bar_bat_voltage, LV_BAR_PART_BG, LV_STATE_DEFAULT, &font_en_24);
    lv_obj_set_style_local_value_align(bar_bat_voltage, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_ALIGN_OUT_RIGHT_MID);
    lv_obj_set_style_local_value_ofs_x(bar_bat_voltage, LV_BAR_PART_BG, LV_STATE_DEFAULT, 10);
    label_bat_voltage = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text_static(label_bat_voltage, "Battery Voltage");
    lv_obj_set_style_local_text_font(label_bat_voltage, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_bold_28);
    lv_obj_align(label_bat_voltage, bar_bat_voltage, LV_ALIGN_OUT_TOP_MID, 0, -20);

    /* Create sliders to control WS2812 RGB LED */
    for (int i = 0; i < 3; i++) {
        slider_led[i] = lv_slider_create(lv_scr_act(), NULL);
        lv_slider_set_range(slider_led[i], 0, 255);
        lv_slider_set_value(slider_led[i], 0, LV_ANIM_ON);
        lv_obj_set_event_cb(slider_led[i], slider_led_cb);
        lv_obj_set_user_data(slider_led[i], (lv_obj_user_data_t) i);
        lv_obj_align(slider_led[i], bar_bat_voltage, LV_ALIGN_OUT_BOTTOM_MID, 0, 100 + 50 * i);
        lv_obj_set_style_local_value_str(slider_led[i], LV_SLIDER_PART_BG, LV_STATE_DEFAULT, "0");
        lv_obj_set_style_local_value_font(slider_led[i], LV_SLIDER_PART_BG, LV_STATE_DEFAULT, &font_en_24);
        lv_obj_set_style_local_value_align(slider_led[i], LV_SLIDER_PART_BG, LV_STATE_DEFAULT, LV_ALIGN_OUT_RIGHT_MID);
        lv_obj_set_style_local_value_ofs_x(slider_led[i], LV_SLIDER_PART_BG, LV_STATE_DEFAULT, 10);
    }

    lv_obj_set_style_local_bg_color(slider_led[0], LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_COLOR_RED);
    lv_obj_set_style_local_bg_color(slider_led[1], LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_COLOR_GREEN);
    lv_obj_set_style_local_bg_color(slider_led[2], LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_COLOR_BLUE);
    
    label_led = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text_static(label_led, "LED Color");
    lv_obj_set_style_local_text_font(label_led, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_bold_28);
    lv_obj_align(label_led, slider_led[0], LV_ALIGN_OUT_TOP_MID, 0, -20);

    lv_task_create(bat_voltage_update_task, 100, 1, NULL);

    lv_port_sem_give();
}
