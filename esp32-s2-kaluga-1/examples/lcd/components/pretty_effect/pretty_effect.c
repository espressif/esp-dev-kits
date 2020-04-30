// Copyright 2019 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <math.h>

#include "pretty_effect.h"
#include "decode_image.h"
#include "sdkconfig.h"

uint16_t **pixels;

/*!< This variable is used to detect the next frame.*/
static int prev_frame = -1;

/*!< Instead of calculating the offsets for each pixel we grab, we pre-calculate the valueswhenever a frame changes, then re-use*/
/*!< these as we go through all the pixels in the frame. This is much, much faster.*/
static int8_t xofs[LCD_WIDTH], yofs[LCD_HEIGHT];
static int8_t xcomp[LCD_WIDTH], ycomp[LCD_HEIGHT];

static inline uint16_t get_bgnd_pixel(int x, int y)
{
    /*!< Image has an 8x8 pixel margin, so we can also resolve e.g. [-3, 243]*/
    x += 8;
    y += 8;
    return pixels[y][x];
}
/*!< Calculate the pixel data for a set of lines (with implied line size of 320). Pixels go in dest, line is the Y-coordinate of the*/
/*!< first line to be calculated, linect is the amount of lines to calculate. Frame increases by one every time the entire image*/
/*!< is displayed; this is used to go to the next frame of animation.*/
void pretty_effect_calc_lines(uint16_t *dest, int line, int frame, int linect)
{
    if (frame != prev_frame) {
        /*!< We need to calculate a new set of offset coefficients. Take some random sines as offsets to make everything*/
        /*!< look pretty and fluid-y.*/
        for (int x = 0; x < LCD_WIDTH; x++) {
            xofs[x] = sin(frame * 0.15 + x * 0.06) * 4;
        }

        for (int y = 0; y < LCD_HEIGHT; y++) {
            yofs[y] = sin(frame * 0.1 + y * 0.05) * 4;
        }

        for (int x = 0; x < LCD_WIDTH; x++) {
            xcomp[x] = sin(frame * 0.11 + x * 0.12) * 4;
        }

        for (int y = 0; y < LCD_HEIGHT; y++) {
            ycomp[y] = sin(frame * 0.07 + y * 0.15) * 4;
        }

        prev_frame = frame;
    }

    for (int y = line; y < line + linect; y++) {
        for (int x = 0; x < LCD_WIDTH; x++) {
            *dest++ = get_bgnd_pixel(x + yofs[y] + xcomp[x], y + xofs[x] + ycomp[y]);
        }
    }
}

void pretty_effect_static_lines(uint16_t *dest, int line, int frame, int linect)
{
    for (int y = line; y < line + linect; y++) {
        for (int x = 0; x < LCD_WIDTH; x++) {
            *dest++ = get_bgnd_pixel(x, y);
        }
    }
}

esp_err_t pretty_effect_init(const uint8_t *image_jpg_start, int width, int height)
{
    return decode_image(&pixels, image_jpg_start, width, height);
}

esp_err_t pretty_effect_deinit(const uint8_t *image_jpg_start, int width, int height)
{
    if (pixels != NULL) {
        for (int i = 0; i < height; i++) {
            free((pixels)[i]);
        }
        free(pixels);
    }
    else
    {
        return ESP_FAIL;
    }
    return ESP_OK;
}