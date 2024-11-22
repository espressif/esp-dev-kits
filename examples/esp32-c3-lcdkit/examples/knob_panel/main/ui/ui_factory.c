/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "lvgl.h"
#include <stdio.h>
#include "ui_language.h"

#include "settings.h"
#include "app_audio.h"
#include "ir_nec_test.h"

#include "lv_example_pub.h"
#include "lv_example_image.h"
#include "bsp/esp-bsp.h"

static const char *TAG = "factory";

typedef void (* lv_obj_func_create)(lv_obj_t *parent, uint8_t event);

typedef enum {
    FACTORY_STEP_ENCODE,
    FACTORY_STEP_LCD,
    FACTORY_STEP_PDM,
    FACTORY_STEP_LED,
    FACTORY_STEP_IR,
    FACTORY_STEP_RESULT,
    FACTORY_STEP_MAX,
} factory_step_t;

typedef enum {
    FACTORY_ENCODE_STEP_LEFT,
    FACTORY_ENCODE_STEP_RIGHT,
    FACTORY_ENCODE_STEP_PRESS,
    FACTORY_ENCODE_STEP_MAX
} factory_encode_substep_t;

typedef enum {
    FACTORY_LCD_STEP_R,
    FACTORY_LCD_STEP_G,
    FACTORY_LCD_STEP_B,
    FACTORY_LCD_STEP_W,
    FACTORY_LCD_STEP_MAX
} factory_LCD_substep_t;

typedef enum {
    FACTORY_SOUND_STEP_0,
    FACTORY_SOUND_STEP_MAX
} factory_Sound_substep_t;

typedef enum {
    FACTORY_LED_STEP_LIGHT,
    FACTORY_LED_STEP_MAX
} factory_LED_substep_t;

typedef struct {
    const char *test_name;
    lv_obj_func_create sprite_create_func;
    lv_obj_func_create sprite_event_detect;
    uint8_t max_steps;
    lv_obj_t *sprite_parent;
} sprite_create_func_t;

static lv_obj_t *page;
static lv_obj_t *label_EN, *label_CN;
static lv_obj_t *imgbtn_TEST_FAILED, *imgbtn_TEST_OK;

static time_out_count time_500ms;
static factory_step_t factory_test_step = FACTORY_STEP_MAX;
static uint8_t factory_sub_step;

static uint8_t factory_test_result;

static bool factory_Layer_enter_cb(void *layer);
static bool factory_Layer_exit_cb(void *layer);
static void factory_Layer_timer_cb(lv_timer_t *tmr);

static void lv_create_test_encode(lv_obj_t *factory_bg, uint8_t event);
static void lv_create_test_LCD(lv_obj_t *factory_bg, uint8_t event);
static void lv_create_test_Sound(lv_obj_t *factory_bg, uint8_t event);
static void lv_create_test_LED(lv_obj_t *factory_bg, uint8_t event);
static void lv_create_test_IR(lv_obj_t *factory_bg, uint8_t event);
static void lv_create_test_Result(lv_obj_t *factory_bg, uint8_t event);

lv_layer_t factory_Layer = {
    .lv_obj_name    = "factory_Layer",
    .lv_obj_parent  = NULL,
    .lv_obj_layer   = NULL,
    .lv_show_layer  = NULL,
    .enter_cb       = factory_Layer_enter_cb,
    .exit_cb        = factory_Layer_exit_cb,
    .timer_cb       = factory_Layer_timer_cb,
};

static sprite_create_func_t sprite_test_list[] = {
    {"编码器",  lv_create_test_encode,  lv_create_test_encode,  0, NULL},
    {"LCD",     lv_create_test_LCD,     lv_create_test_LCD,     0, NULL},
    {"喇叭",    lv_create_test_Sound,   lv_create_test_Sound,   0, NULL},
    {"LED",     lv_create_test_LED,     lv_create_test_LED,     0, NULL},
    {"红外",    lv_create_test_IR,      lv_create_test_IR,      0, NULL},
    {"Result",  lv_create_test_Result,  lv_create_test_Result,  0, NULL},
};

void factory_result_save(factory_step_t step)
{
    if (imgbtn_TEST_OK) {
        if (lv_obj_has_state(imgbtn_TEST_OK, LV_STATE_CHECKED) == true) {
            factory_test_result |= (0x01 << step);
        }
    } else {
        factory_test_result |= (0x01 << step);
    }
}

bool factory_result_get(factory_step_t step)
{
    return (factory_test_result & (0x01 << step)) ? (true) : (false);
}

static uint8_t factory_test_step_goto(uint8_t sprite)
{
    lv_obj_t *parent;
    if (sprite ^ factory_test_step) {
        factory_test_step = sprite;
        factory_sub_step = 0;

        for (int i = 0; i < FACTORY_STEP_MAX; i++) {
            if (sprite_test_list[i].sprite_parent) {
                imgbtn_TEST_OK = NULL;
                imgbtn_TEST_FAILED = NULL;
                lv_obj_del(sprite_test_list[i].sprite_parent);
                sprite_test_list[i].sprite_parent = NULL;
            }
        }
        parent = lv_obj_create(factory_Layer.lv_obj_layer);
        lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_border_width(parent, 0, 0);
        lv_obj_set_style_border_side(parent, LV_BORDER_SIDE_BOTTOM, 0);

        sprite_test_list[sprite].sprite_parent = parent;
        sprite_test_list[sprite].sprite_create_func(parent, 0xFF);
    }

    return factory_test_step;
}

void lv_create_factory_title(lv_obj_t *parent, const char *title_name, factory_step_t page)
{
    lv_obj_set_size(parent, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_radius(parent, 0, 0);
    lv_obj_set_style_bg_color(parent, lv_color_hex(COLOUR_GREY_2F), LV_PART_MAIN);

    lv_obj_t *label_title = lv_label_create(parent);
    lv_obj_set_style_text_color(label_title, lv_color_white(), 0);
    lv_obj_set_style_text_font(label_title, &font_SourceHanSansCN_20, 0);
    lv_label_set_text(label_title, title_name);
    lv_obj_center(label_title);
    lv_obj_align(label_title, LV_ALIGN_TOP_MID, 0, 10);
}

void lv_create_rst_select(lv_obj_t *parent)
{
    page = parent;

    if ((NULL == imgbtn_TEST_OK) && (NULL == imgbtn_TEST_FAILED)) {
        imgbtn_TEST_OK = lv_img_create(page);
        lv_obj_align(imgbtn_TEST_OK, LV_ALIGN_CENTER, -40, 50);
        lv_img_set_src(imgbtn_TEST_OK, &language_select);

        label_EN = lv_label_create(imgbtn_TEST_OK);
        lv_obj_set_style_text_font(label_EN, &font_SourceHanSansCN_20, 0);
        lv_label_set_text(label_EN, "成功");
        lv_obj_set_style_text_color(label_EN, lv_color_hex(COLOUR_BLACK), 0);
        lv_obj_align(label_EN, LV_ALIGN_CENTER, 0, 0);

        imgbtn_TEST_FAILED = lv_img_create(page);
        lv_obj_align(imgbtn_TEST_FAILED, LV_ALIGN_CENTER, 40, 50);
        lv_img_set_src(imgbtn_TEST_FAILED, &language_unselect);

        label_CN = lv_label_create(imgbtn_TEST_FAILED);
        lv_obj_set_style_text_font(label_CN, &font_SourceHanSansCN_20, 0);
        lv_label_set_text(label_CN, "失败");
        lv_obj_set_style_text_opa(label_CN, LV_OPA_40, 0);
        lv_obj_set_style_text_color(label_CN, lv_color_hex(COLOUR_BLACK), 0);
        lv_obj_align(label_CN, LV_ALIGN_CENTER, 0, 0);

        lv_obj_clear_state(imgbtn_TEST_OK, LV_STATE_CHECKED);
    }

    if (lv_obj_has_state(imgbtn_TEST_OK, LV_STATE_CHECKED) == false) {
        lv_img_set_src(imgbtn_TEST_OK, &language_select);
        lv_obj_add_state(imgbtn_TEST_OK, LV_STATE_CHECKED);
        lv_obj_set_style_text_opa(label_EN, LV_OPA_COVER, 0);

        lv_img_set_src(imgbtn_TEST_FAILED, &language_unselect);
        lv_obj_clear_state(imgbtn_TEST_FAILED, LV_STATE_CHECKED);
        lv_obj_set_style_text_opa(label_CN, LV_OPA_40, 0);
    } else {
        lv_img_set_src(imgbtn_TEST_FAILED, &language_select);
        lv_obj_add_state(imgbtn_TEST_FAILED, LV_STATE_CHECKED);
        lv_obj_set_style_text_opa(label_CN, LV_OPA_COVER, 0);

        lv_img_set_src(imgbtn_TEST_OK, &language_unselect);
        lv_obj_clear_state(imgbtn_TEST_OK, LV_STATE_CHECKED);
        lv_obj_set_style_text_opa(label_EN, LV_OPA_40, 0);
    }
}

static void lv_create_test_encode(lv_obj_t *factory_bg, uint8_t event)
{
    uint8_t focused = FACTORY_STEP_ENCODE;
    static lv_obj_t *label_guide = NULL;

    ESP_LOGI(TAG, "[Encode], event:%d, sub_step:%d", event, factory_sub_step);

    if (NULL == label_guide) {

        label_guide = lv_label_create(factory_bg);
        lv_obj_set_style_text_align(label_guide, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_color(label_guide, lv_color_white(), 0);
        lv_obj_set_style_text_font(label_guide, &font_SourceHanSansCN_20, 0);
        lv_obj_align(label_guide, LV_ALIGN_CENTER, 0, 0);
        lv_label_set_text(label_guide, "左旋");

        lv_obj_t *label_version = lv_label_create(factory_bg);
        lv_obj_set_style_text_align(label_version, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_color(label_version, lv_color_white(), 0);
        lv_obj_set_style_text_font(label_version, &font_SourceHanSansCN_20, 0);
        lv_obj_align(label_version, LV_ALIGN_CENTER, 0, 80);
        lv_label_set_text_fmt(label_version, "Ver: V%u.%u.%u", \
            DEMO_VERSION_MAJOR, DEMO_VERSION_MINOR, DEMO_VERSION_PATCH);

        lv_create_factory_title(factory_bg, sprite_test_list[focused].test_name, focused);

        factory_sub_step = FACTORY_ENCODE_STEP_LEFT;
    } else {
        audio_handle_info(SOUND_TYPE_KNOB);
    }

    switch (factory_sub_step) {
    case FACTORY_ENCODE_STEP_LEFT:
        if (LV_KEY_LEFT == event) {
            factory_sub_step++;
            lv_label_set_text(label_guide, "右旋");
        }
        break;
    case FACTORY_ENCODE_STEP_RIGHT:
        if (LV_KEY_RIGHT == event) {
            factory_sub_step++;
            lv_label_set_text(label_guide, "按下");
        }
        break;
    case FACTORY_ENCODE_STEP_PRESS:
        if (LV_KEY_DOWN == event) {
            factory_sub_step++;
            lv_label_set_text(label_guide, "成功\n按下下一步");
        }
        break;
    case FACTORY_ENCODE_STEP_MAX:
        if (LV_KEY_DOWN == event) {
            factory_result_save(focused);
            focused++;
            ESP_LOGI(TAG, "FACTORY_STEP_ENCODE++:%d", focused);
            factory_test_step_goto(focused);
        }
        break;
    }
}

static void lv_create_test_LCD(lv_obj_t *factory_bg, uint8_t event)
{
    uint8_t focused = FACTORY_STEP_LCD;
    static lv_obj_t *obj_BG = NULL;
    static lv_obj_t *label_guide;

    ESP_LOGI(TAG, "[LCD] event:%d, sub_step:%d", event, factory_sub_step);

    if (NULL == obj_BG) {
        obj_BG = lv_obj_create(factory_bg);
        lv_obj_center(obj_BG);
        lv_obj_set_style_border_width(obj_BG, 0, 0);
        lv_obj_set_size(obj_BG, LV_HOR_RES, LV_VER_RES);
        lv_obj_set_style_radius(obj_BG, 0, 0);
        lv_obj_set_style_bg_color(obj_BG, lv_color_hex(0xFF0000), LV_PART_MAIN);

        lv_create_factory_title(factory_bg, sprite_test_list[focused].test_name, focused);

        label_guide = lv_label_create(obj_BG);
        lv_obj_set_style_text_color(label_guide, lv_color_white(), 0);
        lv_obj_set_style_text_font(label_guide, &font_SourceHanSansCN_20, 0);
        lv_obj_center(label_guide);
        lv_obj_align(label_guide, LV_ALIGN_CENTER, 0, 0);
        lv_label_set_text(label_guide, "按下下一步");
    }

    switch (factory_sub_step) {
    case FACTORY_LCD_STEP_R:
        if (LV_KEY_DOWN == event) {
            factory_sub_step++;
            lv_obj_set_style_bg_color(obj_BG, lv_color_hex(0xFF00), LV_PART_MAIN);
        }
        break;
    case FACTORY_LCD_STEP_G:
        if (LV_KEY_DOWN == event) {
            factory_sub_step++;
            lv_obj_set_style_bg_color(obj_BG, lv_color_hex(0xFF), LV_PART_MAIN);
        }
        break;
    case FACTORY_LCD_STEP_B:
        if (LV_KEY_DOWN == event) {
            factory_sub_step++;
            lv_obj_set_style_bg_color(obj_BG, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        }
        break;
    case FACTORY_LCD_STEP_W:
        if (LV_KEY_DOWN == event) {
            factory_sub_step++;
            lv_obj_set_style_bg_color(obj_BG, lv_color_hex(COLOUR_GREY_4F), LV_PART_MAIN);
            lv_create_rst_select(obj_BG);

            lv_obj_align(label_guide, LV_ALIGN_CENTER, 0, -20);
            lv_label_set_text(label_guide, "屏幕正常?");
        }
        break;
    case FACTORY_LCD_STEP_MAX:
        if (LV_KEY_DOWN == event) {
            factory_result_save(focused);
            focused++;
            ESP_LOGI(TAG, "FACTORY_STEP_LCD++:%d", focused);
            factory_test_step_goto(focused);
        } else {
            lv_create_rst_select(obj_BG);
        }
        break;
    }
}

static void lv_create_test_Sound(lv_obj_t *factory_bg, uint8_t event)
{
    ESP_LOGI(TAG, "[Sound] event:%d, sub_step:%d", event, factory_sub_step);

    uint8_t focused = FACTORY_STEP_PDM;
    static lv_obj_t *obj_BG = NULL;

    if (NULL == obj_BG) {
        obj_BG = lv_obj_create(factory_bg);
        lv_obj_center(obj_BG);
        lv_obj_set_size(obj_BG, LV_HOR_RES, LV_VER_RES);
        lv_obj_set_style_radius(obj_BG, 0, 0);

        lv_create_factory_title(factory_bg, sprite_test_list[focused].test_name, focused);

        lv_obj_t *label_guide = lv_label_create(obj_BG);
        lv_obj_set_style_text_color(label_guide, lv_color_white(), 0);
        lv_obj_set_style_text_font(label_guide, &font_SourceHanSansCN_20, 0);
        lv_obj_center(label_guide);
        lv_obj_align(label_guide, LV_ALIGN_CENTER, 0, -20);
        lv_label_set_text(label_guide, "喇叭正常?");

        audio_handle_info(SOUND_TYPE_FACTORY);
    } else {
        audio_handle_info(SOUND_TYPE_FACTORY);
    }

    switch (factory_sub_step) {
    case FACTORY_SOUND_STEP_0:
    case FACTORY_SOUND_STEP_MAX:
        if (LV_KEY_DOWN == event) {
            factory_result_save(focused);
            focused++;
            ESP_LOGI(TAG, "FACTORY_STEP_SOUND++:%d", focused);
            factory_test_step_goto(focused);
        } else {
            lv_create_rst_select(obj_BG);
        }
        break;
    }
}

static void lv_create_test_LED(lv_obj_t *factory_bg, uint8_t event)
{
    ESP_LOGI(TAG, "[LED] event:%d, sub_step:%d", event, factory_sub_step);

    uint8_t focused = FACTORY_STEP_LED;
    static lv_obj_t *obj_BG = NULL;
    static lv_obj_t *label_guide = NULL;
    static uint8_t light = 51;

    if (NULL == obj_BG) {
        obj_BG = lv_obj_create(factory_bg);
        lv_obj_center(obj_BG);
        lv_obj_set_size(obj_BG, LV_HOR_RES, LV_VER_RES);
        lv_obj_set_style_radius(obj_BG, 0, 0);

        lv_create_factory_title(factory_bg, sprite_test_list[focused].test_name, focused);

        label_guide = lv_label_create(obj_BG);
        lv_obj_set_style_text_color(label_guide, lv_color_white(), 0);
        lv_obj_set_style_text_font(label_guide, &font_SourceHanSansCN_20, 0);
        lv_obj_center(label_guide);
        lv_obj_align(label_guide, LV_ALIGN_CENTER, 0, -30);
        lv_label_set_text(label_guide, "左旋:暗\n右旋:亮\n按下:下一步");

        bsp_led_rgb_set(light, light, light);
    } else {
        audio_handle_info(SOUND_TYPE_KNOB);
    }

    switch (factory_sub_step) {
    case FACTORY_LED_STEP_LIGHT:
        if (LV_KEY_LEFT == event) {
            if (light > 10) {
                light -= 10;
            }
            ESP_LOGI(TAG, "pwm:%d", light);
            bsp_led_rgb_set(light, light, light);
        } else if (LV_KEY_RIGHT == event) {
            if (light < 200) {
                light += 10;
            }
            ESP_LOGI(TAG, "pwm:%d", light);
            bsp_led_rgb_set(light, light, light);
        } else if (LV_KEY_DOWN == event) {
            factory_sub_step++;
            bsp_led_rgb_set(0x00, 0x00, 0x00);
            lv_obj_align(label_guide, LV_ALIGN_CENTER, 0, -20);
            lv_label_set_text(label_guide, "LED正常?");
            lv_create_rst_select(obj_BG);
        }
        break;
    case FACTORY_LED_STEP_MAX:
        if (LV_KEY_DOWN == event) {
            factory_result_save(focused);
            focused ++;
            ESP_LOGI(TAG, "FACTORY_STEP_LED++:%d", focused);
            factory_test_step_goto(focused);
        } else {
            lv_create_rst_select(obj_BG);
        }
        break;
    }
}

static void lv_create_test_IR(lv_obj_t *factory_bg, uint8_t event)
{
    static lv_obj_t *label_guide = NULL;

    uint8_t focused = FACTORY_STEP_IR;
    if (0xFF == event) {
        nec_test_start();
        lv_create_factory_title(factory_bg, sprite_test_list[focused].test_name, focused);

        label_guide = lv_label_create(factory_bg);
        lv_obj_set_style_text_align(label_guide, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_color(label_guide, lv_color_white(), 0);
        lv_obj_set_style_text_font(label_guide, &font_SourceHanSansCN_20, 0);
        lv_obj_center(label_guide);
        lv_obj_align(label_guide, LV_ALIGN_CENTER, 0, 0);
        lv_label_set_text(label_guide, "1:跳帽先接 IR_TX\n 2:2S后再接 IR_RX\n 3:等待结果");
    } else {
        if (LV_KEY_DOWN == event) {
            factory_result_save(focused);
            focused++;
            ESP_LOGI(TAG, "FACTORY_STEP_IR++:%d", focused);
            factory_test_step_goto(focused);
        } else if (0xFE == event) {
            if (nec_test_result()) {
                lv_label_set_text(label_guide, "成功\n按下结束");
            }
        }
    }
}

static void lv_create_test_Result(lv_obj_t *factory_bg, uint8_t event)
{
    char test_Result[100];
    ESP_LOGI(TAG, "[Result] event:%d, sub_step:%d", event, factory_sub_step);

    uint8_t focused = FACTORY_STEP_RESULT;
    if (0xFF == event) {
        lv_create_factory_title(factory_bg, sprite_test_list[focused].test_name, focused);

        sprintf(test_Result, "%s: #%x %s# \n%s: #%x %s# \n%s: #%x %s# \n%s: #%x %s# \n%s: #%x %s#",
                sprite_test_list[FACTORY_STEP_ENCODE].test_name,
                factory_result_get(FACTORY_STEP_ENCODE) ? 0xFF00 : 0xFF0000,
                factory_result_get(FACTORY_STEP_ENCODE) ? "OK" : "Fail",
                sprite_test_list[FACTORY_STEP_LCD].test_name,
                factory_result_get(FACTORY_STEP_LCD) ? 0xFF00 : 0xFF0000,
                factory_result_get(FACTORY_STEP_LCD) ? "OK" : "Fail",
                sprite_test_list[FACTORY_STEP_PDM].test_name,
                factory_result_get(FACTORY_STEP_PDM) ? 0xFF00 : 0xFF0000,
                factory_result_get(FACTORY_STEP_PDM) ? "OK" : "Fail",
                sprite_test_list[FACTORY_STEP_LED].test_name,
                factory_result_get(FACTORY_STEP_LED) ? 0xFF00 : 0xFF0000,
                factory_result_get(FACTORY_STEP_LED) ? "OK" : "Fail",
                sprite_test_list[FACTORY_STEP_IR].test_name,
                factory_result_get(FACTORY_STEP_IR) ? 0xFF00 : 0xFF0000,
                factory_result_get(FACTORY_STEP_IR) ? "OK" : "Fail");

        lv_obj_t *label_guide = lv_label_create(factory_bg);
        lv_label_set_recolor(label_guide, true);
        lv_obj_set_style_base_dir(label_guide, LV_BASE_DIR_LTR, 0);
        lv_obj_set_style_text_color(label_guide, lv_color_white(), 0);
        lv_obj_set_style_text_font(label_guide, &font_SourceHanSansCN_20, 0);
        lv_obj_center(label_guide);
        lv_obj_align(label_guide, LV_ALIGN_TOP_MID, 0, 50);
        lv_label_set_text(label_guide, test_Result);

        ESP_LOGI(TAG, "\r\n%s", test_Result);
    }
}

static void factory_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (LV_EVENT_FOCUSED == code) {
        lv_group_set_editing(lv_group_get_default(), true);
    } else if ((LV_EVENT_KEY == code) || (LV_EVENT_CLICKED == code)) {

        uint32_t key;
        factory_step_t focus = factory_test_step;
        lv_obj_t *parent = sprite_test_list[focus].sprite_parent;

        if (LV_EVENT_KEY == code) {
            key = lv_event_get_key(e);
        } else {
            key = LV_KEY_DOWN;
        }

        if (is_time_out(&time_500ms)) {
            if (sprite_test_list[focus].sprite_event_detect) {
                sprite_test_list[focus].sprite_event_detect(parent, key);
            }
        }
    } else if (LV_EVENT_LONG_PRESSED == code) {
    }
}

static bool factory_Layer_enter_cb(void *layer)
{
    bool ret = false;

    LV_LOG_USER("");

    lv_layer_t *create_layer = layer;
    if (NULL == create_layer->lv_obj_layer) {
        ret = true;
        create_layer->lv_obj_layer = lv_obj_create(lv_scr_act());
        lv_obj_remove_style_all(create_layer->lv_obj_layer);
        lv_obj_set_size(create_layer->lv_obj_layer, LV_HOR_RES, LV_VER_RES);

        factory_test_step_goto(FACTORY_STEP_ENCODE);
        set_time_out(&time_500ms, 100);
    }

    ui_remove_all_objs_from_encoder_group();//roll will add event default.
    lv_obj_add_event_cb(create_layer->lv_obj_layer, factory_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(create_layer->lv_obj_layer, factory_event_cb, LV_EVENT_FOCUSED, NULL);
    lv_obj_add_event_cb(create_layer->lv_obj_layer, factory_event_cb, LV_EVENT_KEY, NULL);
    lv_obj_add_event_cb(create_layer->lv_obj_layer, factory_event_cb, LV_EVENT_LONG_PRESSED, NULL);
    ui_add_obj_to_encoder_group(create_layer->lv_obj_layer);

    return ret;
}

static bool factory_Layer_exit_cb(void *layer)
{
    LV_LOG_USER("");
    return true;
}

static void factory_Layer_timer_cb(lv_timer_t *tmr)
{
    feed_clock_time();

    if (FACTORY_STEP_IR == factory_test_step) {
        lv_obj_t *parent = sprite_test_list[FACTORY_STEP_IR].sprite_parent;
        if (sprite_test_list[FACTORY_STEP_IR].sprite_event_detect) {
            sprite_test_list[FACTORY_STEP_IR].sprite_event_detect(parent, 0xFE);
        }
    }
}
