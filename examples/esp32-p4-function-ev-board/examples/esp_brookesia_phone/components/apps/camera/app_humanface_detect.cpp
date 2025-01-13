/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "esp_log.h"
#include "iostream"
#include "human_face_detect.hpp"
#include "dl_tool.hpp"
#include "app_humanface_detect.h"

static HumanFaceDetect *detect = NULL;

std::list<dl::detect::result_t> app_humanface_detect(uint16_t *frame, int width, int height)
{
    dl::image::img_t img;
    img.data = frame;
    img.width = width;
    img.height = height;
    img.pix_type = dl::image::DL_IMAGE_PIX_TYPE_RGB565;
    
    auto &detect_results = detect->run(img);

    return detect_results;
}

HumanFaceDetect *get_humanface_detect()
{
    if (detect == NULL) {
        detect = new HumanFaceDetect();
    }

    return detect;
}

void delete_humanface_detect()
{
    if (detect) {
        delete detect;
        detect = NULL;
    }
}