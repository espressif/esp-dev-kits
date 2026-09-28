/*
 * SPDX-FileCopyrightText: 2023-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifdef ESP_UTILS_LOG_TAG
#   undef ESP_UTILS_LOG_TAG
#endif
#define ESP_UTILS_LOG_TAG "BS:App:FactoryGuide"
#include "esp_lib_utils.h"
#include "esp_brookesia_app_factory_guide.hpp"

#define APP_NAME        "Factory Guide"
#define APP_VERSION     "1.1.0"
#define SCREEN_SIZE     (240)

#define IMAGE_SIZE      (150)
#define IMAGE_SCALE     ((float)IMAGE_SIZE / SCREEN_SIZE * LV_SCALE_NONE)
#define IMAGE_Y_OFFSET  (-20)

#define LABEL_WIDTH     (250)
#define LABEL_Y_OFFSET  (1)

#define VERSION_LABEL_WIDTH     (220)
#define VERSION_LABEL_HEIGHT    (20)
#define VERSION_LABEL_Y_OFFSET  (5)

LV_IMG_DECLARE(img_app_factory_guide);
LV_IMG_DECLARE(img_qr_code);

namespace esp_brookesia::apps {

constexpr systems::base::App::Config CORE_DATA = {
    .name = APP_NAME,
    .launcher_icon = gui::StyleImage::IMAGE(&img_app_factory_guide),
    .screen_size = gui::StyleSize::RECT_PERCENT(100, 100),
    .flags = {
        .enable_default_screen = 1,
        .enable_recycle_resource = 1,
        .enable_resize_visual_area = 1,
    },
};
constexpr systems::phone::App::Config APP_DATA = {
    .app_launcher_page_index = 0,
    .flags = {
        .enable_navigation_gesture = 1,
    },
};

FactoryGuide::FactoryGuide()
    : App(CORE_DATA, APP_DATA)
{
}

FactoryGuide::~FactoryGuide()
{
    ESP_UTILS_LOG_TRACE_GUARD_WITH_THIS();
}

bool FactoryGuide::run()
{
    ESP_UTILS_LOG_TRACE_GUARD_WITH_THIS();

    // Create version label at the top
    char version_text[64];
    snprintf(version_text, sizeof(version_text), "v%s %s", APP_VERSION, __DATE__);

    auto version_label = lv_label_create(lv_scr_act());
    lv_label_set_text(version_label, version_text);
    lv_obj_set_size(version_label, VERSION_LABEL_WIDTH, VERSION_LABEL_HEIGHT);
    lv_obj_align(version_label, LV_ALIGN_TOP_MID, 0, VERSION_LABEL_Y_OFFSET);
    lv_obj_set_style_text_font(version_label, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_align(version_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(version_label, lv_color_hex(0x808080), 0); // Gray color for subtle appearance

    auto qr_code = lv_image_create(lv_scr_act());
    lv_image_set_src(qr_code, &img_qr_code);
    lv_obj_set_size(qr_code, IMAGE_SIZE, IMAGE_SIZE);
    lv_obj_refr_size(qr_code);
    lv_obj_align(qr_code, LV_ALIGN_CENTER, 0, IMAGE_Y_OFFSET);
    lv_image_set_scale(qr_code, IMAGE_SCALE);
    lv_image_set_inner_align(qr_code, LV_IMAGE_ALIGN_CENTER);

    auto label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "Please scan the QR code to access the factory guide");
    lv_obj_set_size(label, LABEL_WIDTH, LV_SIZE_CONTENT);
    lv_obj_align_to(label, qr_code, LV_ALIGN_OUT_BOTTOM_MID, 0, LABEL_Y_OFFSET);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);

    return true;
}

/**
 * @brief The function will be called when the left button of navigate bar is clicked.
 */
bool FactoryGuide::back()
{
    ESP_UTILS_LOG_TRACE_GUARD_WITH_THIS();

    ESP_UTILS_CHECK_FALSE_RETURN(notifyCoreClosed(), false, "Notify core closed failed");

    return true;
}

/**
 * @brief The function will be called when app should be closed.
 */
bool FactoryGuide::close()
{
    ESP_UTILS_LOG_TRACE_GUARD_WITH_THIS();

    return true;
}

ESP_UTILS_REGISTER_PLUGIN(systems::base::App, FactoryGuide, APP_NAME)

} // namespace esp_brookesia::apps
