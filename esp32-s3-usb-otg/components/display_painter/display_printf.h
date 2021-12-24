// Copyright 2021 Espressif Systems (Shanghai) Co. Ltd.
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

#ifndef _DISPLAY_PRINTF_H_
#define _DISPLAY_PRINTF_H_

#include "display_painter.h"
#include "screen_driver.h"

#define DISPLAY_PRINT_LEVEL DISPLAY_PRINTF_LEVEL_VERBOSE
#define DISPLAY_MAX_MUTEX_WAIT_MS 50 
/**
 * @brief 
 *
 */
typedef enum {
    DISPLAY_PRINTF_LEVEL_NONE,       /*!< No log output */
    DISPLAY_PRINTF_LEVEL_ERROR,      /*!< Critical errors, software module can not recover on its own */
    DISPLAY_PRINTF_LEVEL_WARN,       /*!< Error conditions from which recovery measures have been taken */
    DISPLAY_PRINTF_LEVEL_INFO,       /*!< Information messages which describe normal flow of events */
    DISPLAY_PRINTF_LEVEL_DEBUG,      /*!< Extra information which is not necessary for normal use (values, pointers, sizes, etc). */
    DISPLAY_PRINTF_LEVEL_VERBOSE     /*!< Bigger chunks of debugging information, or frequent messages which can potentially flood the output. */
} display_level_t;

void display_printf_clear(void);
void display_printf_set_font(font_t font);
font_t display_printf_get_font();
void display_printf(display_level_t level, const char *tag, uint16_t color, const char *format, ...);
void display_printf_line(const char *tag, uint16_t line, uint16_t color, const char *format, ...);

#define DISPLAY_PRINTF_COLOR_E COLOR_RED
#define DISPLAY_PRINTF_COLOR_W COLOR_YELLOW
#define DISPLAY_PRINTF_COLOR_I COLOR_GREEN
#define DISPLAY_PRINTF_COLOR_D COLOR_WHITE
#define DISPLAY_PRINTF_COLOR_V COLOR_WHITE

#define DISPLAY_PRINTF_COLOR(C) DISPLAY_PRINTF_COLOR_ ## C

#define display_printf_LOCAL(level, tag, format, ...) do {                     \
        if (level==DISPLAY_PRINTF_LEVEL_ERROR )          { display_printf(DISPLAY_PRINTF_LEVEL_ERROR,      tag, DISPLAY_PRINTF_COLOR(E), format __VA_OPT__(,) __VA_ARGS__); } \
        else if (level==DISPLAY_PRINTF_LEVEL_WARN )      { display_printf(DISPLAY_PRINTF_LEVEL_WARN,       tag, DISPLAY_PRINTF_COLOR(W), format __VA_OPT__(,) __VA_ARGS__); } \
        else if (level==DISPLAY_PRINTF_LEVEL_DEBUG )     { display_printf(DISPLAY_PRINTF_LEVEL_DEBUG,      tag, DISPLAY_PRINTF_COLOR(D), format __VA_OPT__(,) __VA_ARGS__); } \
        else if (level==DISPLAY_PRINTF_LEVEL_VERBOSE )   { display_printf(DISPLAY_PRINTF_LEVEL_VERBOSE,    tag, DISPLAY_PRINTF_COLOR(V), format __VA_OPT__(,) __VA_ARGS__); } \
        else                                             { display_printf(DISPLAY_PRINTF_LEVEL_INFO,       tag, DISPLAY_PRINTF_COLOR(I), format __VA_OPT__(,) __VA_ARGS__); } \
    } while(0)

#define DISPLAY_PRINTF_LOCAL(level, tag, format, ...) do {               \
        if ( DISPLAY_PRINT_LEVEL >= level ) display_printf_LOCAL(level, tag, format __VA_OPT__(,) __VA_ARGS__); \
    } while(0)

#define DISPLAY_PRINTF_INIT(lcd) painter_init(lcd)
#define DISPLAY_PRINTF_SET_FONT(font) display_printf_set_font(font)
#define DISPLAY_PRINTF_CLEAR() display_printf_clear()
#define DISPLAY_PRINTF_LINE(tag, line, color, format, ... ) display_printf_line(tag, line, color, format __VA_OPT__(,) __VA_ARGS__);
#define DISPLAY_PRINTFE( tag, format, ... ) DISPLAY_PRINTF_LOCAL(DISPLAY_PRINTF_LEVEL_ERROR,    tag, format __VA_OPT__(,) __VA_ARGS__)
#define DISPLAY_PRINTFW( tag, format, ... ) DISPLAY_PRINTF_LOCAL(DISPLAY_PRINTF_LEVEL_WARN,    tag, format __VA_OPT__(,) __VA_ARGS__)
#define DISPLAY_PRINTFI( tag, format, ... ) DISPLAY_PRINTF_LOCAL(DISPLAY_PRINTF_LEVEL_INFO,    tag, format __VA_OPT__(,) __VA_ARGS__)
#define DISPLAY_PRINTFD( tag, format, ... ) DISPLAY_PRINTF_LOCAL(DISPLAY_PRINTF_LEVEL_DEBUG,    tag, format __VA_OPT__(,) __VA_ARGS__)
#define DISPLAY_PRINTFV( tag, format, ... ) DISPLAY_PRINTF_LOCAL(DISPLAY_PRINTF_LEVEL_VERBOSE,    tag, format __VA_OPT__(,) __VA_ARGS__)

#endif