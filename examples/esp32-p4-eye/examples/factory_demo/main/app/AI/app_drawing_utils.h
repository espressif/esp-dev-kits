/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <stdint.h>
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Set the global screen dimensions
 * 
 * @param width     Width of the screen
 * @param height    Height of the screen
 */
void set_screen_dimensions(int width, int height);

/**
 * @brief Draw a rectangle with specified RGB color on a buffer
 * 
 * @param buffer      Pointer to RGB565 buffer
 * @param width       Width of the buffer
 * @param height      Height of the buffer
 * @param x1          X coordinate of the top-left corner
 * @param y1          Y coordinate of the top-left corner
 * @param x2          X coordinate of the bottom-right corner
 * @param y2          Y coordinate of the bottom-right corner
 * @param x_offset    X offset to apply to the coordinates
 * @param y_offset    Y offset to apply to the coordinates
 * @param r           Red component (0-255)
 * @param g           Green component (0-255)
 * @param b           Blue component (0-255)
 * @param thickness   Thickness of the rectangle border
 * @param swap_rgb565 Whether to swap the byte order of RGB565 values
 */
void draw_rectangle_rgb(uint16_t *buffer, int width, int height, int x1, int y1, int x2, int y2, 
                        int x_offset, int y_offset, uint8_t r, uint8_t g, uint8_t b, int thickness, bool swap_rgb565 = false);

/**
 * @brief Draw green points at specified landmarks
 * 
 * @param buffer      Pointer to RGB565 buffer
 * @param landmarks   Vector of landmark coordinates (x1, y1, x2, y2, ..., x5, y5)
 * @param swap_rgb565 Whether to swap the byte order of RGB565 values
 */
void draw_green_points(uint16_t *buffer, const std::vector<int> &landmarks, bool swap_rgb565 = false);

#ifdef __cplusplus
}
#endif 