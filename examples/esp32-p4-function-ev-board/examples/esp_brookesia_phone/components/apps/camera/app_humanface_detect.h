/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include "human_face_detect.hpp"

std::list<dl::detect::result_t> app_humanface_detect(uint16_t *frame, int width, int height);

#ifdef __cplusplus
extern "C" {
#endif

HumanFaceDetect *get_humanface_detect();
void delete_humanface_detect();

#ifdef __cplusplus
}
#endif
