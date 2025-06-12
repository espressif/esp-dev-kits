/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include "coco_detect.hpp"

std::list<dl::detect::result_t> app_coco_detect(uint16_t *frame, int width, int height);

// Get class name from category index
const char* get_coco_class_name(int category_index);

#ifdef __cplusplus
extern "C" {
#endif

COCODetect *get_coco_detect();
void delete_coco_detect();

#ifdef __cplusplus
}
#endif 