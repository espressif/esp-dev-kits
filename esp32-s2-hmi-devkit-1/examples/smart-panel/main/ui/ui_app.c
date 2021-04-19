
/**
 * @file ui_app.c
 * @brief APP provision page src.
 * @version 0.1
 * @date 2021-01-11
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

#include "ui_main.h"
#include "esp_qcloud_iothub.h"
#include "qrcodegen.h"

static const char *TAG = "ui_app";
#define LOG_TRACE(...)  ESP_LOGI(TAG, ##__VA_ARGS__)
#define CHECK(a, str, ret_val) do { \
        if (!(a)) { \
            ESP_LOGE(TAG,"%s(%d): %s", __FUNCTION__, __LINE__, str); \
            return (ret_val); \
        } \
    } while(0)

/* UI function declaration */
ui_func_desc_t ui_app_func = {
    .name = "APP Connect",
    .init = ui_app_init,
    .show = ui_app_show,
    .hide = ui_app_hide,
};

/* LVGL objects defination */
static lv_obj_t *obj_page_app = NULL;
static lv_obj_t *app_guide = NULL;
static lv_obj_t *obj_qr = NULL;
static lv_obj_t *img_qr = NULL;
static lv_img_dsc_t img_qr_desc;
static lv_obj_t *obj_num[3] = { NULL, NULL, NULL};

static ui_state_t ui_app_state = ui_state_dis;

/* Static function forward declaration */
static void ui_app_gen_qr(void);

void ui_app_init(void *data)
{
    ui_page_show("APP Connect");
    obj_page_app = ui_page_get_obj();

    app_guide = lv_table_create(obj_page_app, NULL);
    lv_table_set_col_cnt(app_guide, 1);
    lv_table_set_row_cnt(app_guide, 4);
    lv_table_set_col_width(app_guide, 0, 300);
    lv_obj_set_style_local_border_width(app_guide, LV_TABLE_PART_BG, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_text_font(app_guide, LV_TABLE_PART_CELL1, LV_STATE_DEFAULT, &font_en_24);
    lv_obj_set_style_local_text_font(app_guide, LV_TABLE_PART_CELL2, LV_STATE_DEFAULT, &font_en_24);
    lv_obj_set_style_local_text_font(app_guide, LV_TABLE_PART_CELL3, LV_STATE_DEFAULT, &font_en_24);

    lv_obj_set_style_local_border_color(app_guide, LV_TABLE_PART_CELL1, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_border_color(app_guide, LV_TABLE_PART_CELL3, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    
    lv_obj_set_style_local_pad_top(app_guide, LV_TABLE_PART_CELL1, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_pad_bottom(app_guide, LV_TABLE_PART_CELL1, LV_STATE_DEFAULT, 15);
    lv_obj_set_style_local_pad_top(app_guide, LV_TABLE_PART_CELL2, LV_STATE_DEFAULT, 15);
    lv_obj_set_style_local_pad_bottom(app_guide, LV_TABLE_PART_CELL2, LV_STATE_DEFAULT, 15);
    lv_obj_set_style_local_pad_top(app_guide, LV_TABLE_PART_CELL3, LV_STATE_DEFAULT, 15);
    lv_obj_set_style_local_pad_bottom(app_guide, LV_TABLE_PART_CELL3, LV_STATE_DEFAULT, 0);

    lv_table_set_cell_type(app_guide, 0, 0, 1);
    lv_table_set_cell_type(app_guide, 1, 0, 2);
    lv_table_set_cell_type(app_guide, 2, 0, 2);
    lv_table_set_cell_type(app_guide, 3, 0, 3);

    lv_table_set_cell_value(app_guide, 0, 0, "Open the WeChat");
    lv_table_set_cell_value(app_guide, 1, 0, "Scan QR Code");
    lv_table_set_cell_value_fmt(app_guide, 2, 0, "Select \"%s\"", CONFIG_LIGHT_PROVISIONING_SOFTAPCONFIG_SSID);
    lv_obj_align(app_guide, NULL, LV_ALIGN_IN_TOP_LEFT, 80, 100);
    
    obj_qr = lv_obj_create(obj_page_app, NULL);
    lv_obj_set_size(obj_qr, 250, 250);
    lv_obj_set_click(obj_qr, false);
    lv_obj_set_style_local_border_width(obj_qr, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_radius(obj_qr, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 20);
    lv_obj_set_style_local_bg_color(obj_qr, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, COLOR_BG);
    lv_obj_align(obj_qr, NULL, LV_ALIGN_IN_TOP_RIGHT, -40, 90);

    for (int i = 0; i < 3; i++) {
        obj_num[i] = lv_obj_create(obj_page_app, NULL);
        lv_obj_set_size(obj_num[i], 36, 36);
        lv_obj_set_style_local_bg_color(obj_num[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, COLOR_THEME);
        lv_obj_set_style_local_border_color(obj_num[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, COLOR_THEME);
        lv_obj_set_style_local_radius(obj_num[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 18);
        lv_obj_set_style_local_value_font(obj_num[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &font_en_24);
        lv_obj_set_style_local_value_color(obj_num[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
        
        lv_obj_set_click(obj_num[i], false);
    }

    /* DO NOT USE SPRINTF !!!!! */
    lv_obj_set_style_local_value_str(obj_num[0], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, "1");
    lv_obj_set_style_local_value_str(obj_num[1], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, "2");
    lv_obj_set_style_local_value_str(obj_num[2], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, "3");

    lv_obj_align(obj_num[0], app_guide, LV_ALIGN_OUT_LEFT_MID, 10, -25 -65);
    lv_obj_align(obj_num[1], app_guide, LV_ALIGN_OUT_LEFT_MID, 10, -25 -5);
    lv_obj_align(obj_num[2], app_guide, LV_ALIGN_OUT_LEFT_MID, 10, -25 + 55);

    lv_obj_t *white_bg = lv_obj_create(obj_qr, NULL);
    lv_obj_set_size(white_bg, 210, 210);
    lv_obj_set_click(white_bg, false);
    lv_obj_set_style_local_border_width(white_bg, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_radius(white_bg, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_bg_color(white_bg, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_align(white_bg, NULL, LV_ALIGN_CENTER, 0, 0);

    img_qr = lv_img_create(white_bg, NULL);
    lv_img_set_src(img_qr, &img_qr_desc);
    lv_img_set_antialias(img_qr, false);
    lv_img_set_zoom(img_qr, 256 * 5);
    lv_obj_align(img_qr, NULL, LV_ALIGN_CENTER, -2, -2);

    /* Check QCloud config */
    if (strlen(CONFIG_QCLOUD_DEVICE_SECRET) != DEVICE_SECRET_SIZE
        || strlen(CONFIG_QCLOUD_PRODUCT_ID) != PRODUCT_ID_SIZE) {
            static const char *btns[] = { "OK", "" };
            lv_obj_t *msgbox = lv_msgbox_create(lv_scr_act(), NULL);
            lv_obj_set_style_local_text_font(msgbox, LV_MSGBOX_PART_BG, LV_STATE_DEFAULT, &font_en_24);
            lv_msgbox_set_text(msgbox, "Check QCloud config!");
            lv_msgbox_add_btns(msgbox, btns);
            lv_obj_align(msgbox, NULL, LV_ALIGN_CENTER, 0, 0);
    }

    ui_app_state = ui_state_show;
}

void ui_app_show(void *data)
{
    if (ui_state_dis == ui_app_state) {
        ui_app_gen_qr();
        ui_app_init(data);
    } else if (ui_state_hide == ui_app_state) {
        lv_obj_set_hidden(app_guide, false);
        lv_obj_set_hidden(obj_qr, false);
        lv_obj_set_hidden(obj_num[0], false);
        lv_obj_set_hidden(obj_num[1], false);
        lv_obj_set_hidden(obj_num[2], false);
        
        ui_page_show("APP Connect");
        
        ui_app_state = ui_state_show;
    }
}

void ui_app_hide(void *data)
{
    if (ui_state_show == ui_app_state) {
        lv_obj_set_hidden(app_guide, true);
        lv_obj_set_hidden(obj_qr, true);
        lv_obj_set_hidden(obj_num[0], true);
        lv_obj_set_hidden(obj_num[1], true);
        lv_obj_set_hidden(obj_num[2], true);
        lv_obj_set_hidden(obj_page_app, true);
        
        ui_app_state = ui_state_hide;
    }
}

static esp_err_t qr_encode(const char *text, uint8_t **qr_code) {
    *qr_code = heap_caps_malloc(qrcodegen_BUFFER_LEN_MAX, MALLOC_CAP_SPIRAM);
    CHECK(qr_code, "Failed allocate mem for QR code", ESP_ERR_NO_MEM);

    uint8_t *qr_buffer = heap_caps_malloc(qrcodegen_BUFFER_LEN_MAX, MALLOC_CAP_SPIRAM);
    CHECK(qr_buffer, "Failed allocate mem for QR code", ESP_ERR_NO_MEM);

    LOG_TRACE("Buffer size : %d * %d", qrcodegen_BUFFER_LEN_MAX, qrcodegen_BUFFER_LEN_MAX);

    bool gen_succ = qrcodegen_encodeText(text, qr_buffer, *qr_code, qrcodegen_Ecc_LOW,
        qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO, true);

    free(qr_buffer);

    CHECK(gen_succ, "Failed generate QR code", ESP_FAIL);

    return ESP_OK;
}

static esp_err_t qr_data_to_img(uint8_t *qr_data)
{
    int size = qrcodegen_getSize(qr_data);

    static lv_color_t *img_data = NULL;
    
    if (NULL == img_data) {
        img_data = heap_caps_malloc(size * size *  LV_COLOR_SIZE / 8, MALLOC_CAP_SPIRAM);
    }

    CHECK(img_data, "Failed allocate mem for QR image", ESP_ERR_NO_MEM);

    img_qr_desc.header.always_zero = 0;
    img_qr_desc.header.w = size;
    img_qr_desc.header.h = size;
    img_qr_desc.data_size = size * size * size * LV_COLOR_SIZE / 8,
    img_qr_desc.header.cf = LV_IMG_CF_TRUE_COLOR;
    img_qr_desc.data = (uint8_t *) img_data;

    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            img_data[y * size + x].full = qrcodegen_getModule(qr_data, x, y) ? LV_COLOR_BLACK.full : LV_COLOR_WHITE.full;
        }
    }

    return ESP_OK;
}

static void ui_app_gen_qr(void)
{
    uint8_t *qr_code = NULL;
    char *qr_text = NULL;

    asprintf(&qr_text,
        "https://iot.cloud.tencent.com/iotexplorer/device?page=softap&productId=%s&ver=%s&name=%s",
        CONFIG_QCLOUD_PRODUCT_ID, "v1", CONFIG_LIGHT_PROVISIONING_SOFTAPCONFIG_SSID);

    if (ESP_OK == qr_encode(qr_text, &qr_code)) {
        qr_data_to_img(qr_code);
        free(qr_code);
    }

    free(qr_text);
}
