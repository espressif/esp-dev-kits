/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "../ui.h"

#include <string.h>

LV_IMAGE_DECLARE(environment_bg);

#define SCR_W 280
#define SCR_H 241

#define COLOR_BLACK lv_color_hex(0x000000)
#define COLOR_LVL1 lv_color_hex(0x349D2C)
#define COLOR_TRANSPARENT lv_color_hex(0xFFFFFF)

typedef struct {
    lv_obj_t *card;
    lv_obj_t *value;
    lv_obj_t *unit;
    lv_obj_t *status;
} monitor_card_handles_t;

static monitor_card_handles_t monitor_cards[4];
static lv_obj_t *ui_Background = NULL;

lv_obj_t *ui_TemperatureScreen = NULL;
lv_obj_t *ui_TimeLabel = NULL;
lv_obj_t *ui_BatteryLabel = NULL;
lv_obj_t *ui_Temperature = NULL;
lv_obj_t *ui_TemperatureLabel = NULL;
lv_obj_t *ui_TemperatureNumber = NULL;
lv_obj_t *ui_TemperatureSignal = NULL;
lv_obj_t *ui_TemperatureStatus = NULL;
lv_obj_t *ui_Thermometer = NULL;
lv_obj_t *ui_Fire = NULL;
lv_obj_t *ui_Humidity = NULL;
lv_obj_t *ui_HumidityLabel = NULL;
lv_obj_t *ui_Water = NULL;
lv_obj_t *ui_HumidityNumber = NULL;
lv_obj_t *ui_HumidityUnit = NULL;
lv_obj_t *ui_HumidityEvaluation = NULL;
lv_obj_t *ui_Pressure = NULL;
lv_obj_t *ui_PressureLabel = NULL;
lv_obj_t *ui_PressureImage = NULL;
lv_obj_t *ui_PressureText = NULL;
lv_obj_t *ui_PressureUnit = NULL;
lv_obj_t *ui_PressureTrend = NULL;
lv_obj_t *ui_AirQuality = NULL;
lv_obj_t *ui_AirQualityLabel = NULL;
lv_obj_t *ui_Leaf = NULL;
lv_obj_t *ui_AirQualityCO2 = NULL;
lv_obj_t *ui_AirQualityNumber = NULL;
lv_obj_t *ui_AirQualityLevel = NULL;
lv_obj_t *ui_AirQualitySignal = NULL;

static lv_obj_t *create_hidden_label(lv_obj_t *parent)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, "");
    lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
    return label;
}

static lv_obj_t *create_card_proxy(lv_obj_t *parent, int32_t x, int32_t y, int32_t w, int32_t h)
{
    lv_obj_t *obj = lv_obj_create(parent);
    lv_obj_remove_style_all(obj);
    lv_obj_set_pos(obj, x, y);
    lv_obj_set_size(obj, w, h);
    lv_obj_set_style_bg_color(obj, COLOR_TRANSPARENT, 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    return obj;
}

static lv_obj_t *create_value_label(lv_obj_t *parent, int32_t x, int32_t y, int32_t width)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, "--");
    lv_obj_set_pos(label, x, y);
    lv_obj_set_width(label, width);
    lv_label_set_long_mode(label, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_26, 0);
    lv_obj_set_style_text_color(label, COLOR_BLACK, 0);
    return label;
}

static lv_obj_t *create_unit_label(lv_obj_t *parent, lv_obj_t *value, const char *unit)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, unit);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(label, COLOR_BLACK, 0);
    lv_obj_align_to(label, value, LV_ALIGN_OUT_RIGHT_BOTTOM, 2, -2);
    return label;
}

static lv_obj_t *create_status_label(lv_obj_t *parent, int32_t x, int32_t y, int32_t width)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, "");
    lv_obj_set_pos(label, x, y);
    lv_obj_set_width(label, width);
    lv_label_set_long_mode(label, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(label, COLOR_LVL1, 0);
    return label;
}

static void create_overlay_card(lv_obj_t *parent, monitor_card_handles_t *handles,
                                int32_t card_x, int32_t card_y, int32_t value_x, int32_t value_y,
                                int32_t value_width, int32_t status_y, int32_t status_width,
                                const char *unit)
{
    handles->card = create_card_proxy(parent, card_x, card_y, 124, 95);
    handles->value = create_value_label(parent, value_x, value_y, value_width);
    handles->unit = create_unit_label(parent, handles->value, unit);
    handles->status = create_status_label(parent, value_x, status_y, status_width);
}

void ui_TemperatureScreen_screen_init(void)
{
    memset(monitor_cards, 0, sizeof(monitor_cards));

    ui_TemperatureScreen = lv_obj_create(NULL);
    lv_obj_set_size(ui_TemperatureScreen, SCR_W, SCR_H);
    lv_obj_set_style_bg_color(ui_TemperatureScreen, lv_color_hex(0xF6F6F6), 0);
    lv_obj_set_style_bg_opa(ui_TemperatureScreen, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(ui_TemperatureScreen, 48, 0);
    lv_obj_set_style_clip_corner(ui_TemperatureScreen, true, 0);
    lv_obj_set_style_border_width(ui_TemperatureScreen, 0, 0);
    lv_obj_set_style_pad_all(ui_TemperatureScreen, 0, 0);
    lv_obj_clear_flag(ui_TemperatureScreen, LV_OBJ_FLAG_SCROLLABLE);

    ui_Background = lv_image_create(ui_TemperatureScreen);
    lv_image_set_src(ui_Background, &environment_bg);
    lv_obj_align(ui_Background, LV_ALIGN_CENTER, 0, -20);

    ui_TimeLabel = create_hidden_label(ui_TemperatureScreen);
    ui_BatteryLabel = create_hidden_label(ui_TemperatureScreen);

    create_overlay_card(ui_TemperatureScreen, &monitor_cards[0], 12, 10, 24, 34, 72, 74, 82, "\xC2\xB0" "C");
    ui_Temperature = monitor_cards[0].card;
    ui_TemperatureLabel = create_hidden_label(ui_TemperatureScreen);
    ui_TemperatureNumber = monitor_cards[0].value;
    ui_TemperatureSignal = monitor_cards[0].unit;
    ui_TemperatureStatus = monitor_cards[0].status;
    ui_Fire = ui_Background;
    ui_Thermometer = ui_Background;

    create_overlay_card(ui_TemperatureScreen, &monitor_cards[1], 145, 10, 156, 34, 72, 74, 82, "%");
    ui_Humidity = monitor_cards[1].card;
    ui_HumidityLabel = create_hidden_label(ui_TemperatureScreen);
    ui_HumidityNumber = monitor_cards[1].value;
    ui_HumidityUnit = monitor_cards[1].unit;
    ui_HumidityEvaluation = monitor_cards[1].status;
    ui_Water = ui_Background;

    create_overlay_card(ui_TemperatureScreen, &monitor_cards[2], 12, 113, 24, 137, 76, 177, 82, "hPa");
    ui_Pressure = monitor_cards[2].card;
    ui_PressureLabel = create_hidden_label(ui_TemperatureScreen);
    ui_PressureText = monitor_cards[2].value;
    ui_PressureUnit = monitor_cards[2].unit;
    ui_PressureTrend = monitor_cards[2].status;
    ui_PressureImage = ui_Background;

    create_overlay_card(ui_TemperatureScreen, &monitor_cards[3], 145, 113, 156, 137, 60, 177, 82, "ppm");
    ui_AirQuality = monitor_cards[3].card;
    ui_AirQualityLabel = create_hidden_label(ui_TemperatureScreen);
    ui_AirQualityCO2 = ui_AirQualityLabel;
    ui_AirQualityNumber = monitor_cards[3].value;
    ui_AirQualitySignal = monitor_cards[3].unit;
    ui_AirQualityLevel = monitor_cards[3].status;
    ui_Leaf = ui_Background;
}

void ui_TemperatureScreen_screen_destroy(void)
{
    if (ui_TemperatureScreen) {
        lv_obj_del(ui_TemperatureScreen);
    }

    memset(monitor_cards, 0, sizeof(monitor_cards));
    ui_Background = NULL;
    ui_TemperatureScreen = NULL;
    ui_TimeLabel = NULL;
    ui_BatteryLabel = NULL;
    ui_Temperature = NULL;
    ui_TemperatureLabel = NULL;
    ui_TemperatureNumber = NULL;
    ui_TemperatureSignal = NULL;
    ui_TemperatureStatus = NULL;
    ui_Thermometer = NULL;
    ui_Fire = NULL;
    ui_Humidity = NULL;
    ui_HumidityLabel = NULL;
    ui_Water = NULL;
    ui_HumidityNumber = NULL;
    ui_HumidityUnit = NULL;
    ui_HumidityEvaluation = NULL;
    ui_Pressure = NULL;
    ui_PressureLabel = NULL;
    ui_PressureImage = NULL;
    ui_PressureText = NULL;
    ui_PressureUnit = NULL;
    ui_PressureTrend = NULL;
    ui_AirQuality = NULL;
    ui_AirQualityLabel = NULL;
    ui_Leaf = NULL;
    ui_AirQualityCO2 = NULL;
    ui_AirQualityNumber = NULL;
    ui_AirQualityLevel = NULL;
    ui_AirQualitySignal = NULL;
}
