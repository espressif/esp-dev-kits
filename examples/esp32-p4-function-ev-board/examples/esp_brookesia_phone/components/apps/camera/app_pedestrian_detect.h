/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include "pedestrian_detect.hpp"

#define EXAMPLE_DETECT_RES                   (224)
#define EXAMPLE_DETECT_PX_FORMAT             (24)

std::list<dl::detect::result_t> app_pedestrian_detect(uint16_t *frame, int width, int height);

#ifdef __cplusplus
extern "C" {
#endif

PedestrianDetect *get_pedestrian_detect();
void delete_pedestrian_detect();

void draw_rectangle_rgb(uint16_t *buffer, int width, int height, int x1, int y1, int x2, int y2, int x_offset, int y_offset, uint8_t r, uint8_t g, uint8_t b, int thickness);

void draw_green_points(uint16_t *buffer, const std::vector<int> &landmarks);

#ifdef __cplusplus
}
#endif
