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

#ifdef __cplusplus
}
#endif
