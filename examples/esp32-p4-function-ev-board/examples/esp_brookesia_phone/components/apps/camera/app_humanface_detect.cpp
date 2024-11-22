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
    auto detect_results = detect->run(frame, {width, height, 3});

    return detect_results;
}

HumanFaceDetect **get_humanface_detect()
{
    return &detect;
}
