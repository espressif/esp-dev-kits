/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "esp_log.h"
#include "iostream"
#include "coco_detect.hpp"
#include "dl_tool.hpp"
#include "dl_image_define.hpp"
#include "app_coco_detect.h"

// Remove unused TAG variable since this module doesn't use ESP logging
static COCODetect *detect = NULL;

// COCO dataset class names
static const char* COCO_CLASSES[] = {
    "person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat", "traffic light",
    "fire hydrant", "stop sign", "parking meter", "bench", "bird", "cat", "dog", "horse", "sheep", "cow",
    "elephant", "bear", "zebra", "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee",
    "skis", "snowboard", "sports ball", "kite", "baseball bat", "baseball glove", "skateboard", "surfboard", 
    "tennis racket", "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl", "banana", "apple", 
    "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair", "couch",
    "potted plant", "bed", "dining table", "toilet", "tv", "laptop", "mouse", "remote", "keyboard", "cell phone",
    "microwave", "oven", "toaster", "sink", "refrigerator", "book", "clock", "vase", "scissors", "teddy bear",
    "hair drier", "toothbrush"
};

#define COCO_CLASS_COUNT (sizeof(COCO_CLASSES) / sizeof(COCO_CLASSES[0]))

const char* get_coco_class_name(int category_index)
{
    if (category_index >= 0 && category_index < COCO_CLASS_COUNT) {
        return COCO_CLASSES[category_index];
    }
    return "unknown";
}

std::list<dl::detect::result_t> app_coco_detect(uint16_t *frame, int width, int height)
{
    dl::image::img_t img;
    img.data = frame;
    img.width = width;
    img.height = height;
    img.pix_type = dl::image::DL_IMAGE_PIX_TYPE_RGB565;

    auto &detect_results = detect->run(img);

    return detect_results;
}

COCODetect *get_coco_detect()
{
    if (detect == NULL) {
        detect = new COCODetect();
    }

    return detect;
}

void delete_coco_detect()
{
    if (detect) {
        delete detect;
        detect = NULL;
    }
} 