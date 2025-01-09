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

#define WIDTH  1280
#define HEIGHT 720

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

void draw_rectangle_rgb(uint16_t *buffer, int width, int height, int x1, int y1, int x2, int y2, int x_offset, int y_offset, uint8_t r, uint8_t g, uint8_t b, int thickness)
{
    // Apply offset to the coordinates
    x1 += x_offset;
    x2 += x_offset;
    y1 += y_offset;
    y2 += y_offset;

    // Clip coordinates to the buffer dimensions
    if (x1 < 0) x1 = 0;
    if (y1 < 0) y1 = 0;
    if (x2 >= width) x2 = width - 1;
    if (y2 >= height) y2 = height - 1;

    // Convert RGB888 to RGB565
    uint16_t color = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);

    // Draw the top and bottom horizontal lines with thickness
    for (int t = 0; t < thickness; ++t) {
        for (int x = x1; x <= x2; ++x) {
            if (y1 + t >= 0 && y1 + t < height && x >= 0 && x < width) {
                buffer[(y1 + t) * width + x] = color;
            }
            if (y2 - t >= 0 && y2 - t < height && x >= 0 && x < width) {
                buffer[(y2 - t) * width + x] = color;
            }
        }
    }

    // Draw the left and right vertical lines with thickness
    for (int t = 0; t < thickness; ++t) {
        for (int y = y1; y <= y2; ++y) {
            if (x1 + t >= 0 && x1 + t < width && y >= 0 && y < height) {
                buffer[y * width + (x1 + t)] = color;
            }
            if (x2 - t >= 0 && x2 - t < width && y >= 0 && y < height) {
                buffer[y * width + (x2 - t)] = color;
            }
        }
    }
}


static void draw_large_green_point(uint16_t *buffer, int x, int y) {
    uint16_t green = 0x07E0; 
    
    for (int dx = -3; dx <= 3; ++dx) {
        for (int dy = -3; dy <= 3; ++dy) {
            int nx = x + dx;
            int ny = y + dy;

            if (nx >= 0 && nx < WIDTH && ny >= 0 && ny < HEIGHT) {
                buffer[ny * WIDTH + nx] = green;
            }
        }
    }
}

void draw_green_points(uint16_t *buffer, const std::vector<int> &landmarks) 
{
    for (int i = 0; i < 5; i++) {
        int x = landmarks[2 * i];     
        int y = landmarks[2 * i + 1]; 

        draw_large_green_point(buffer, x, y);
    }
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
