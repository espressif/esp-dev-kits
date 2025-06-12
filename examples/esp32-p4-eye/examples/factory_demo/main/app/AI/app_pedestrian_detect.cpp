/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "esp_log.h"
#include "iostream"
#include "pedestrian_detect.hpp"
#include "dl_tool.hpp"
#include "dl_image_define.hpp"
#include "app_pedestrian_detect.h"

static PedestrianDetect *detect = NULL;

std::list<dl::detect::result_t> app_pedestrian_detect(uint16_t *frame, int width, int height)
{
    dl::image::img_t img;
    img.data = frame;
    img.width = width;
    img.height = height;
    img.pix_type = dl::image::DL_IMAGE_PIX_TYPE_RGB565;

    auto &detect_results = detect->run(img);

    return detect_results;
}

PedestrianDetect *get_pedestrian_detect()
{
    if (detect == NULL) {
        detect = new PedestrianDetect();
    }

    return detect;
}

void delete_pedestrian_detect()
{
    if (detect) {
        delete detect;
        detect = NULL;
    }
}
