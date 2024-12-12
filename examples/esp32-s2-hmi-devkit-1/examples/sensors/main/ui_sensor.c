/**
 * @file ui_sensor.c
 * @brief Sensor UI src.
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

#include "ui_sensor.h"

/* Log defination */
#define TAG ((const char *)"ui_sensor")
#define UI_LOG_TRACE(...)	ESP_LOGD(TAG, ##__VA_ARGS__)

/* Color value defination */
#define COLOR_BG    lv_color_make(238, 241, 245)
#define COLOR_DEEP  lv_color_make(242, 173, 71)
#define COLOR_LIGHT lv_color_make(253, 200, 0)
#define COLOR_THEME lv_color_make(252, 199, 0)

typedef enum {
    ui_item_mems = 0,
    ui_item_temp_humid,
    ui_item_als,
} ui_item_t;

typedef enum {
    ui_state_not_init = 0,
    ui_state_showing,
    ui_state_hidden,
} ui_state_t;

/* Current selected item, MEMS for default */
static ui_item_t ui_item = ui_item_mems;
static ui_state_t ui_als_state = ui_state_not_init;
static ui_state_t ui_mems_state = ui_state_not_init;
static ui_state_t ui_temp_humid_state = ui_state_not_init;

/* Objects for sensor selection */
static lv_obj_t *obj_temp_humid = NULL;
static lv_obj_t *img_temp_humid = NULL;
static lv_obj_t *obj_mems = NULL;
static lv_obj_t *img_mems = NULL;
static lv_obj_t *obj_als = NULL;
static lv_obj_t *img_als = NULL;
static lv_obj_t *obj_detail = NULL;

/* Objects for MEMS detail page */
static lv_obj_t *label_acc = NULL;
static lv_obj_t *label_acc_x = NULL;
static lv_obj_t *label_acc_y = NULL;
static lv_obj_t *label_acc_z = NULL;
static lv_obj_t *label_gyro = NULL;
static lv_obj_t *label_gyro_x = NULL;
static lv_obj_t *label_gyro_y = NULL;
static lv_obj_t *label_gyro_z = NULL;
static lv_obj_t *bar_acc = NULL;
static lv_obj_t *bar_acc_x = NULL;
static lv_obj_t *bar_acc_y = NULL;
static lv_obj_t *bar_acc_z = NULL;
static lv_obj_t *bar_gyro = NULL;
static lv_obj_t *bar_gyro_x = NULL;
static lv_obj_t *bar_gyro_y = NULL;
static lv_obj_t *bar_gyro_z = NULL;

/* Objects for als sensor detail page */
static lv_obj_t *arc_als_val = NULL;
static lv_obj_t *label_voltage = NULL;
static lv_obj_t *label_voltage_val = NULL;
// static lv_obj_t *gauge_als_val = NULL;
// static lv_obj_t *slider_backlight = NULL;

/* Objects for temp & humid sensor detail page */
static lv_obj_t *label_temp = NULL;
static lv_obj_t *label_temp_val = NULL;
static lv_obj_t *label_humid = NULL;
static lv_obj_t *label_humid_val = NULL;
static lv_obj_t *obj_temp_val_bg = NULL;
static lv_obj_t *obj_temp_val_indic = NULL;
static lv_obj_t *obj_humid_val_bg = NULL;
static lv_obj_t *obj_humid_val_indic = NULL;

/* Image declaration */
LV_IMG_DECLARE(light);
LV_IMG_DECLARE(mems);
LV_IMG_DECLARE(temp_humid);

/* Static function declaration */
static void ui_mems_detail_init(void);
static void ui_mems_detail_show(void);
static void ui_mems_detail_hide(void);

static void ui_als_detail_init(void);
static void ui_als_detail_show(void);
static void ui_als_detail_hide(void);

static void ui_temp_humid_detail_init(void);
static void ui_temp_humid_detail_show(void);
static void ui_temp_humid_detail_hide(void);

void sensor_data_update_task(lv_task_t *task)
{
    static char fmt_text[8];
    static float voltage = 0.0f;
    static uint8_t adc_cvt_count = 0;
    static uint8_t adc_val = 0;
    static float light;
    static float temp, humid;
    static mpu6050_gyro_value_t gyro_val;
    static mpu6050_acce_value_t acc_val;

    switch (ui_item) {
    case ui_item_mems:
        if (ui_state_showing == ui_mems_state) {
            sensor_get_mems_acc(&acc_val);
            sensor_get_mems_gyro(&gyro_val);
            
            lv_bar_set_value(bar_acc_x, acc_val.acce_x * 100, LV_ANIM_ON);
            lv_bar_set_value(bar_acc_y, acc_val.acce_y * 100, LV_ANIM_ON);
            lv_bar_set_value(bar_acc_z, acc_val.acce_z * 100, LV_ANIM_ON);
            lv_bar_set_value(bar_gyro_x, gyro_val.gyro_x, LV_ANIM_ON);
            lv_bar_set_value(bar_gyro_y, gyro_val.gyro_y, LV_ANIM_ON);
            lv_bar_set_value(bar_gyro_z, gyro_val.gyro_z, LV_ANIM_ON);

            UI_LOG_TRACE("Acc : %4.2f, %4.2f, %4.2f | Ang : %8.2f, %8.2f, %8.2f",
                acc_val.acce_x, acc_val.acce_y, acc_val.acce_z,
                gyro_val.gyro_x, gyro_val.gyro_y, gyro_val.gyro_z);
        }
        break;
    
    case ui_item_als:
        als_get_value(&light);
        adc081_get_converted_value(&adc_val);
        adc_cvt_count++;
        if (adc_cvt_count <= 5) {
            voltage += adc_val * 3.3f * 4 / 255.0f;
        } else {
            sprintf(fmt_text, "%.2f V", voltage / 5);
            lv_label_set_text(label_voltage_val, fmt_text);
            voltage = 0;
            adc_cvt_count = 0;
        }


        sprintf(fmt_text, "%d", (int) light);
        lv_arc_set_value(arc_als_val, (int) light);
        lv_obj_set_style_local_value_str(arc_als_val, LV_ARC_PART_BG, LV_STATE_DEFAULT, fmt_text);
        break;

    case ui_item_temp_humid:
        sensor_get_temp(&temp);
        sensor_get_humid(&humid);

        sprintf(fmt_text, "%.1f°C", temp);
        lv_label_set_text(label_temp_val, fmt_text);
        sprintf(fmt_text, "%.1f%%", humid);
        lv_label_set_text(label_humid_val, fmt_text);
        
        lv_coord_t height = 50 + 3 * (lv_coord_t) temp;
        if (height > 200)   height = 200;
        if (height < 0)     height = 10;
        lv_obj_set_height(obj_temp_val_indic, height);
        lv_obj_set_height(obj_humid_val_indic, 2 * (int) humid);
        
        lv_obj_align(obj_temp_val_indic, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -10);
        lv_obj_align(obj_humid_val_indic, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -10);
        break;
    }
}

static void ui_mems_detail_init(void)
{
    label_acc = lv_label_create(obj_detail, NULL);
    lv_obj_set_style_local_text_font(label_acc, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_20);
    label_gyro = lv_label_create(obj_detail, label_acc);
    label_acc_x = lv_label_create(obj_detail, label_acc);
    label_acc_y = lv_label_create(obj_detail, label_acc);
    label_acc_z = lv_label_create(obj_detail, label_acc);
    label_gyro_x = lv_label_create(obj_detail, label_acc);
    label_gyro_y = lv_label_create(obj_detail, label_acc);
    label_gyro_z = lv_label_create(obj_detail, label_acc);

    lv_label_set_text(label_acc, "Acceleration");
    lv_label_set_text(label_gyro, "Angular Velocity");
    lv_label_set_text(label_acc_x, "X");
    lv_label_set_text(label_acc_y, "Y");
    lv_label_set_text(label_acc_z, "Z");
    lv_label_set_text(label_gyro_x, "X");
    lv_label_set_text(label_gyro_y, "Y");
    lv_label_set_text(label_gyro_z, "Z");

    bar_acc = lv_bar_create(obj_detail, NULL);
    bar_gyro = lv_bar_create(obj_detail, NULL);

    bar_acc_x = lv_bar_create(obj_detail, NULL);
    lv_bar_set_sym(bar_acc_x, true);
    lv_obj_set_size(bar_acc_x, 140, 8);
    lv_obj_set_style_local_bg_color(bar_acc_x, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, COLOR_DEEP);
    
    bar_acc_y = lv_bar_create(obj_detail, bar_acc_x);
    bar_acc_z = lv_bar_create(obj_detail, bar_acc_x);
    bar_gyro_x = lv_bar_create(obj_detail, bar_acc_x);
    bar_gyro_y = lv_bar_create(obj_detail, bar_acc_x);
    bar_gyro_z = lv_bar_create(obj_detail, bar_acc_x);

    lv_obj_set_style_local_bg_color(bar_acc, LV_BAR_PART_BG, LV_STATE_DEFAULT, COLOR_DEEP);
    lv_obj_set_style_local_bg_color(bar_gyro, LV_BAR_PART_BG, LV_STATE_DEFAULT, COLOR_LIGHT);

    /* Set bars' size */
    lv_obj_set_size(bar_acc, 50, 8);
    lv_obj_set_size(bar_gyro, 50, 8);

    lv_bar_set_range(bar_acc_x, -160, 160);
    lv_bar_set_range(bar_acc_y, -160, 160);
    lv_bar_set_range(bar_acc_z, -160, 160);
    lv_bar_set_range(bar_gyro_x, -360, 360);
    lv_bar_set_range(bar_gyro_y, -360, 360);
    lv_bar_set_range(bar_gyro_z, -360, 360);

    lv_obj_align(bar_acc_x, NULL, LV_ALIGN_IN_TOP_LEFT, 20, 150);
    lv_obj_align(bar_acc_y, bar_acc_x, LV_ALIGN_OUT_BOTTOM_MID, 0, 60);
    lv_obj_align(bar_acc_z, bar_acc_y, LV_ALIGN_OUT_BOTTOM_MID, 0, 60);
    lv_obj_align(bar_gyro_x, NULL, LV_ALIGN_IN_TOP_RIGHT, -20, 150);
    lv_obj_align(bar_gyro_y, bar_gyro_x, LV_ALIGN_OUT_BOTTOM_MID, 0, 60);
    lv_obj_align(bar_gyro_z, bar_gyro_y, LV_ALIGN_OUT_BOTTOM_MID, 0, 60);

    lv_obj_align(bar_acc, bar_acc_x, LV_ALIGN_OUT_TOP_LEFT, 0, -80);
    lv_obj_align(bar_gyro, bar_gyro_x, LV_ALIGN_OUT_TOP_LEFT, 0, -80);

    lv_obj_align(label_acc, bar_acc, LV_ALIGN_OUT_TOP_LEFT, 0, -10);
    lv_obj_align(label_gyro, bar_gyro, LV_ALIGN_OUT_TOP_LEFT, 0, -10);
    
    lv_obj_align(label_acc_x, bar_acc_x, LV_ALIGN_OUT_TOP_LEFT, 0, -10);
    lv_obj_align(label_acc_y, bar_acc_y, LV_ALIGN_OUT_TOP_LEFT, 0, -10);
    lv_obj_align(label_acc_z, bar_acc_z, LV_ALIGN_OUT_TOP_LEFT, 0, -10);
    lv_obj_align(label_gyro_x, bar_gyro_x, LV_ALIGN_OUT_TOP_LEFT, 0, -10);
    lv_obj_align(label_gyro_y, bar_gyro_y, LV_ALIGN_OUT_TOP_LEFT, 0, -10);
    lv_obj_align(label_gyro_z, bar_gyro_z, LV_ALIGN_OUT_TOP_LEFT, 0, -10);
}

static void ui_mems_detail_show(void)
{
    lv_obj_set_hidden(label_acc, false);
    lv_obj_set_hidden(label_gyro, false);
    lv_obj_set_hidden(label_acc_x, false);
    lv_obj_set_hidden(label_acc_y, false);
    lv_obj_set_hidden(label_acc_z, false);
    lv_obj_set_hidden(label_gyro_x, false);
    lv_obj_set_hidden(label_gyro_y, false);
    lv_obj_set_hidden(label_gyro_z, false);
    lv_obj_set_hidden(bar_acc, false);
    lv_obj_set_hidden(bar_gyro, false);
    lv_obj_set_hidden(bar_acc_x, false);
    lv_obj_set_hidden(bar_acc_y, false);
    lv_obj_set_hidden(bar_acc_z, false);
    lv_obj_set_hidden(bar_gyro_x, false);
    lv_obj_set_hidden(bar_gyro_y, false);
    lv_obj_set_hidden(bar_gyro_z, false);
}

static void ui_mems_detail_hide(void)
{
    lv_obj_set_hidden(label_acc, true);
    lv_obj_set_hidden(label_gyro, true);
    lv_obj_set_hidden(label_acc_x, true);
    lv_obj_set_hidden(label_acc_y, true);
    lv_obj_set_hidden(label_acc_z, true);
    lv_obj_set_hidden(label_gyro_x, true);
    lv_obj_set_hidden(label_gyro_y, true);
    lv_obj_set_hidden(label_gyro_z, true);
    lv_obj_set_hidden(bar_acc, true);
    lv_obj_set_hidden(bar_gyro, true);
    lv_obj_set_hidden(bar_acc_x, true);
    lv_obj_set_hidden(bar_acc_y, true);
    lv_obj_set_hidden(bar_acc_z, true);
    lv_obj_set_hidden(bar_gyro_x, true);
    lv_obj_set_hidden(bar_gyro_y, true);
    lv_obj_set_hidden(bar_gyro_z, true);
}

static void ui_als_detail_init(void)
{
    arc_als_val = lv_arc_create(obj_detail, NULL);

    lv_obj_set_click(arc_als_val, false);
    lv_arc_set_range(arc_als_val, 0, 1000);
    lv_obj_set_size(arc_als_val, 300, 300);
    lv_obj_set_style_local_border_width(arc_als_val, LV_ARC_PART_BG, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_value_font(arc_als_val, LV_ARC_PART_BG, LV_STATE_DEFAULT, &font_en_bold_48);
    lv_obj_set_style_local_value_align(arc_als_val, LV_ARC_PART_BG, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
    lv_obj_set_style_local_line_color(arc_als_val, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, COLOR_THEME);
    lv_obj_align(arc_als_val, NULL, LV_ALIGN_CENTER, 0, -20);

    label_voltage = lv_label_create(obj_detail, NULL);
    lv_label_set_text_static(label_voltage, "Battery :");
    lv_obj_set_style_local_text_font(label_voltage, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_bold_28);
    lv_obj_align(label_voltage, arc_als_val, LV_ALIGN_OUT_BOTTOM_LEFT, 50, 0);

    label_voltage_val = lv_label_create(obj_detail, NULL);
    lv_label_set_text_static(label_voltage_val, "0.00 V");
    lv_obj_set_style_local_text_font(label_voltage_val, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_28);
    lv_obj_align(label_voltage_val, arc_als_val, LV_ALIGN_OUT_BOTTOM_RIGHT, -50, 0);
}

static void ui_als_detail_show(void)
{
    lv_obj_set_hidden(arc_als_val, false);
    lv_obj_set_hidden(label_voltage, false);
    lv_obj_set_hidden(label_voltage_val, false);
}

static void ui_als_detail_hide(void)
{
    UI_LOG_TRACE("Hide ALS page");
    lv_obj_set_hidden(arc_als_val, true);
    lv_obj_set_hidden(label_voltage, true);
    lv_obj_set_hidden(label_voltage_val, true);
}

static void ui_temp_humid_detail_init(void)
{
    obj_temp_val_bg = lv_obj_create(obj_detail, NULL);
    lv_obj_set_click(obj_temp_val_bg, false);
    lv_obj_set_style_local_border_width(obj_temp_val_bg, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    obj_humid_val_bg = lv_obj_create(obj_detail, obj_temp_val_bg);
    obj_temp_val_indic = lv_obj_create(obj_temp_val_bg, obj_temp_val_bg);
    obj_humid_val_indic = lv_obj_create(obj_humid_val_bg, obj_temp_val_bg);

    lv_obj_set_style_local_bg_color(obj_temp_val_bg, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, COLOR_BG);
    lv_obj_set_style_local_bg_color(obj_humid_val_bg, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, COLOR_BG);
    lv_obj_set_style_local_bg_color(obj_temp_val_indic, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, COLOR_LIGHT);
    lv_obj_set_style_local_bg_color(obj_humid_val_indic, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, COLOR_DEEP);

    lv_obj_set_size(obj_temp_val_bg, 100, 220);
    lv_obj_set_size(obj_humid_val_bg, 100, 220);
    lv_obj_set_size(obj_temp_val_indic, 80, 100);
    lv_obj_set_size(obj_humid_val_indic, 80, 100);

    lv_obj_align(obj_temp_val_bg, NULL, LV_ALIGN_CENTER, -60, 0);
    lv_obj_align(obj_humid_val_bg, NULL, LV_ALIGN_CENTER, 60, 0);

    lv_obj_align(obj_temp_val_indic, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -10);
    lv_obj_align(obj_humid_val_indic, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -10);

    label_temp = lv_label_create(obj_detail, NULL);
    lv_obj_set_style_local_text_font(label_temp, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_en_24);
    label_temp_val = lv_label_create(obj_detail, label_temp);
    label_humid = lv_label_create(obj_detail, label_temp);
    label_humid_val = lv_label_create(obj_detail, label_temp);

    lv_label_set_text(label_temp_val, "50°C");
    lv_label_set_text(label_humid_val, "50%");
    lv_label_set_text(label_temp, "Temp");
    lv_label_set_text(label_humid, "Humidity");

    lv_obj_align(label_temp, obj_temp_val_bg, LV_ALIGN_OUT_TOP_MID, 0, -10);
    lv_obj_align(label_temp_val, obj_temp_val_bg, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_align(label_humid, obj_humid_val_bg, LV_ALIGN_OUT_TOP_MID, 0, -10);
    lv_obj_align(label_humid_val, obj_humid_val_bg, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
}

static void ui_temp_humid_detail_show(void)
{
    lv_obj_set_hidden(label_temp_val, false);
    lv_obj_set_hidden(label_humid_val, false);
    lv_obj_set_hidden(label_temp, false);
    lv_obj_set_hidden(label_humid, false);
    lv_obj_set_hidden(obj_temp_val_bg, false);
    lv_obj_set_hidden(obj_temp_val_indic, false);
    lv_obj_set_hidden(obj_humid_val_bg, false);
    lv_obj_set_hidden(obj_humid_val_indic, false);
}

static void ui_temp_humid_detail_hide(void)
{
    lv_obj_set_hidden(label_temp_val, true);
    lv_obj_set_hidden(label_humid_val, true);
    lv_obj_set_hidden(label_temp, true);
    lv_obj_set_hidden(label_humid, true);
    lv_obj_set_hidden(obj_temp_val_bg, true);
    lv_obj_set_hidden(obj_temp_val_indic, true);
    lv_obj_set_hidden(obj_humid_val_bg, true);
    lv_obj_set_hidden(obj_humid_val_indic, true);
}

static void ui_detail_show(ui_item_t ui_item)
{
    switch (ui_item) {
    case ui_item_mems:
        if (ui_state_not_init == ui_mems_state) {
            ui_mems_detail_init();
        } else if (ui_state_hidden == ui_mems_state) {
            ui_mems_detail_show();
        }
        ui_mems_state = ui_state_showing;
        break;

    case ui_item_als:
        if (ui_state_not_init == ui_als_state) {
            ui_als_detail_init();
        } else if (ui_state_hidden == ui_als_state) {
            ui_als_detail_show();
        }
        ui_als_state = ui_state_showing;
        break;

    case ui_item_temp_humid:
        if (ui_state_not_init == ui_temp_humid_state) {
            ui_temp_humid_detail_init();
        } else if (ui_state_hidden == ui_temp_humid_state) {
            ui_temp_humid_detail_show();
        }
        ui_temp_humid_state = ui_state_showing;
        break;
    }
}

static void ui_detail_hide(ui_item_t ui_item)
{
    switch (ui_item) {
    case ui_item_mems:
        UI_LOG_TRACE("Hide MEMS Page");
        if (ui_state_showing == ui_mems_state) {
            ui_mems_detail_hide();
            ui_mems_state = ui_state_hidden;
        }
        break;

    case ui_item_als:
        UI_LOG_TRACE("Hide ALS page");
        if (ui_state_showing == ui_als_state) {
            ui_als_detail_hide();
            ui_als_state = ui_state_hidden;
        }
        break;

    case ui_item_temp_humid:
        UI_LOG_TRACE("Hide temp & humid page");
        if (ui_state_showing == ui_temp_humid_state) {
            ui_temp_humid_detail_hide();
            ui_temp_humid_state = ui_state_hidden;
        }
        break;
    }
}

static lv_obj_t * ui_create_obj(lv_obj_t *parent, lv_coord_t width, lv_coord_t height, const char *text)
{
    lv_obj_t *_obj = lv_obj_create(parent, NULL);
    lv_obj_set_size(_obj, width, height);
    lv_obj_set_click(_obj, true);

    lv_obj_set_style_local_border_width(_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_border_width(_obj, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, 0);
    lv_obj_set_style_local_radius(_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 10);

    lv_obj_set_style_local_bg_color(_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_bg_color(_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, COLOR_DEEP);

    if (NULL != text) {
        lv_obj_set_style_local_value_str(_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text);
        lv_obj_set_style_local_value_font(_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &font_en_28);
        lv_obj_set_style_local_value_color(_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
        lv_obj_set_style_local_value_color(_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_WHITE);
        lv_obj_set_style_local_value_align(_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_LEFT_MID);
        lv_obj_set_style_local_value_ofs_x(_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, height);
    }

    return _obj;
}

static void obj_event_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_CLICKED == event) {
        if (obj_als == obj) {
            ui_item = ui_item_als;
            lv_obj_set_style_local_bg_color(obj_als, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, COLOR_DEEP);
            lv_obj_set_style_local_value_color(obj_als, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
            lv_obj_set_style_local_bg_color(obj_mems, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
            lv_obj_set_style_local_value_color(obj_mems, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
            lv_obj_set_style_local_bg_color(obj_temp_humid, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
            lv_obj_set_style_local_value_color(obj_temp_humid, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
            ui_detail_hide(ui_item_mems);
            ui_detail_hide(ui_item_temp_humid);
            ui_detail_show(ui_item_als);
        } else if (obj_mems == obj) {
            ui_item = ui_item_mems;
            lv_obj_set_style_local_bg_color(obj_als, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
            lv_obj_set_style_local_value_color(obj_als, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
            lv_obj_set_style_local_bg_color(obj_mems, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, COLOR_DEEP);
            lv_obj_set_style_local_value_color(obj_mems, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
            lv_obj_set_style_local_bg_color(obj_temp_humid, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
            lv_obj_set_style_local_value_color(obj_temp_humid, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
            ui_detail_hide(ui_item_als);
            ui_detail_hide(ui_item_temp_humid);
            ui_detail_show(ui_item_mems);
        } else if (obj_temp_humid == obj) {
            ui_item = ui_item_temp_humid;
            lv_obj_set_style_local_bg_color(obj_als, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
            lv_obj_set_style_local_value_color(obj_als, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
            lv_obj_set_style_local_bg_color(obj_mems, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
            lv_obj_set_style_local_value_color(obj_mems, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
            lv_obj_set_style_local_bg_color(obj_temp_humid, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, COLOR_DEEP);
            lv_obj_set_style_local_value_color(obj_temp_humid, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
            ui_detail_hide(ui_item_als);
            ui_detail_hide(ui_item_mems);
            ui_detail_show(ui_item_temp_humid);
        }
    }
}

void ui_sensor_init(void)
{
    lv_port_sem_take();
    
    obj_temp_humid = ui_create_obj(lv_scr_act(), 300, 100, "Temp & Humid");
    lv_obj_align(obj_temp_humid, NULL, LV_ALIGN_IN_TOP_LEFT, 50, 50);
    img_temp_humid = lv_img_create(obj_temp_humid, NULL);
    lv_img_set_src(img_temp_humid, &temp_humid);
    lv_obj_align(img_temp_humid, NULL, LV_ALIGN_IN_LEFT_MID, (lv_obj_get_height(obj_temp_humid) - lv_obj_get_height(img_temp_humid)) / 2, 0);
    lv_obj_set_event_cb(obj_temp_humid, obj_event_cb);

    obj_mems = ui_create_obj(lv_scr_act(), 300, 100, "MEMS Sensor");
    lv_obj_align(obj_mems, NULL, LV_ALIGN_IN_LEFT_MID, 50, 0);
    img_mems = lv_img_create(obj_mems, NULL);
    lv_img_set_src(img_mems, &mems);
    lv_obj_align(img_mems, NULL, LV_ALIGN_IN_LEFT_MID, (lv_obj_get_height(obj_mems) - lv_obj_get_height(img_mems)) / 2, 0);
    lv_obj_set_event_cb(obj_mems, obj_event_cb);

    obj_als = ui_create_obj(lv_scr_act(), 300, 100, "ALS Sensor");
    lv_obj_align(obj_als, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 50, -50);
    img_als = lv_img_create(obj_als, NULL);
    lv_img_set_src(img_als, &light);
    lv_obj_align(img_als, NULL, LV_ALIGN_IN_LEFT_MID, (lv_obj_get_height(obj_als) - lv_obj_get_height(img_als)) / 2, 0);
    lv_obj_set_event_cb(obj_als, obj_event_cb);

    /* Create sensor detail page */
    obj_detail = ui_create_obj(lv_scr_act(), 350, 380, NULL);

    /* Disable click on detail page */
    lv_obj_set_click(obj_detail, false);
    lv_obj_align(obj_detail, NULL, LV_ALIGN_IN_RIGHT_MID, -50, 0);

    /* Show default page */
    ui_detail_show(ui_item);
    lv_obj_set_style_local_bg_color(obj_mems, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, COLOR_DEEP);

    /* Start sensor data acquire task and update periodically */
    lv_task_create(sensor_data_update_task, 100, 1, NULL);

    lv_port_sem_give();
}
