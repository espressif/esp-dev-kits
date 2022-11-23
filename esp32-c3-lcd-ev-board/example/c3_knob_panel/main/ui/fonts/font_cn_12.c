/*******************************************************************************
 * Size: 12 px
 * Bpp: 2
 * Opts: 
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifndef FONT_CN_12
#define FONT_CN_12 1
#endif

#if FONT_CN_12

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */

    /* U+0021 "!" */
    0x33, 0x33, 0x33, 0x13,

    /* U+0022 "\"" */
    0xca, 0x14, 0x40,

    /* U+0023 "#" */
    0x6, 0x14, 0x9, 0x24, 0x2e, 0xb8, 0xc, 0x30,
    0xc, 0x30, 0x2e, 0xf8, 0x18, 0x90, 0x24, 0x80,

    /* U+0024 "$" */
    0x2, 0x40, 0x2, 0x40, 0x1f, 0xe0, 0x32, 0x58,
    0x32, 0x40, 0x1f, 0x40, 0x2, 0xb4, 0x2, 0x4c,
    0x62, 0x4c, 0x1f, 0xe0, 0x2, 0x40,

    /* U+0025 "%" */
    0x2a, 0x2, 0x5, 0x14, 0x90, 0x51, 0x58, 0x2,
    0xa2, 0x0, 0x0, 0x8a, 0x80, 0x25, 0x45, 0x6,
    0x14, 0x50, 0x80, 0xa8,

    /* U+0026 "&" */
    0xa, 0xc0, 0x6, 0x18, 0x0, 0x85, 0x0, 0x29,
    0x0, 0x19, 0xc8, 0xc, 0xf, 0x3, 0x3, 0xc0,
    0x7f, 0x4c,

    /* U+0027 "'" */
    0xe4,

    /* U+0028 "(" */
    0xc, 0x24, 0x30, 0x30, 0x20, 0x20, 0x20, 0x30,
    0x30, 0x24, 0xc,

    /* U+0029 ")" */
    0x80, 0x83, 0x8, 0x24, 0x92, 0x48, 0x30, 0x88,
    0x0,

    /* U+002A "*" */
    0x8, 0xa, 0xa0, 0xe0, 0x88,

    /* U+002B "+" */
    0x2, 0x0, 0xc, 0x0, 0x30, 0x1f, 0xfd, 0x3,
    0x0, 0xc, 0x0,

    /* U+002C "," */
    0x20, 0xc2, 0x4,

    /* U+002D "-" */
    0x3f, 0x80,

    /* U+002E "." */
    0x20, 0xc0,

    /* U+002F "/" */
    0x3, 0x0, 0x80, 0x50, 0x30, 0x8, 0x5, 0x3,
    0x0, 0x80, 0x90, 0x0,

    /* U+0030 "0" */
    0xb, 0xc0, 0xc0, 0xc2, 0x2, 0x58, 0x6, 0x60,
    0x18, 0x80, 0x93, 0x3, 0x2, 0xf0,

    /* U+0031 "1" */
    0x2d, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5,

    /* U+0032 "2" */
    0xb, 0xd0, 0xc0, 0xc1, 0x2, 0x40, 0xc, 0x1,
    0xc0, 0x28, 0x2, 0x40, 0xf, 0xfd,

    /* U+0033 "3" */
    0x1f, 0xd0, 0xc0, 0xc0, 0x3, 0x1, 0xf4, 0x0,
    0x30, 0x0, 0x57, 0x2, 0x47, 0xf4,

    /* U+0034 "4" */
    0x0, 0xe0, 0xa, 0x80, 0x62, 0x2, 0x8, 0x30,
    0x21, 0xff, 0xe0, 0x2, 0x0, 0x8,

    /* U+0035 "5" */
    0x2f, 0xe0, 0xc0, 0x3, 0x6d, 0xd, 0xc, 0x0,
    0x18, 0x40, 0x63, 0x3, 0x2, 0xf4,

    /* U+0036 "6" */
    0xb, 0xe0, 0x90, 0x43, 0x6d, 0xd, 0xc, 0x30,
    0x14, 0xc0, 0x52, 0x43, 0x2, 0xf4,

    /* U+0037 "7" */
    0x7f, 0xf4, 0x0, 0x80, 0xc, 0x0, 0x60, 0x3,
    0x0, 0xc, 0x0, 0x60, 0x1, 0x40,

    /* U+0038 "8" */
    0x1e, 0xd0, 0xc0, 0xc3, 0x3, 0x3, 0xb4, 0x24,
    0x71, 0x80, 0x63, 0x2, 0x47, 0xf8,

    /* U+0039 "9" */
    0x1f, 0xc0, 0xc0, 0xc2, 0x2, 0x4c, 0xd, 0x1a,
    0xa4, 0x0, 0x93, 0x3, 0x7, 0xf0,

    /* U+003A ":" */
    0x30, 0x80, 0x0, 0x20, 0xc0,

    /* U+003B ";" */
    0x30, 0x80, 0x0, 0x20, 0xc2, 0x4,

    /* U+003C "<" */
    0x0, 0x0, 0x1, 0x80, 0x78, 0x1d, 0x0, 0x38,
    0x0, 0xa, 0x40, 0x2, 0x0,

    /* U+003D "=" */
    0x3f, 0xf4, 0x0, 0x0, 0x0, 0xf, 0xfd,

    /* U+003E ">" */
    0x0, 0xd, 0x0, 0x2d, 0x0, 0x1d, 0x2, 0xd2,
    0xd0, 0x80, 0x0,

    /* U+003F "?" */
    0x2f, 0x86, 0x6, 0x0, 0x50, 0x18, 0x3, 0x0,
    0x0, 0x0, 0x0, 0x60,

    /* U+0040 "@" */
    0x2, 0xaa, 0x0, 0x8a, 0x58, 0x22, 0x48, 0x52,
    0x30, 0x85, 0x23, 0x18, 0x82, 0x19, 0xa0, 0x14,
    0x0, 0x0, 0x6a, 0xa0,

    /* U+0041 "A" */
    0x2, 0x80, 0x2, 0x80, 0x9, 0x60, 0xc, 0x30,
    0x18, 0x24, 0x3f, 0xfc, 0x60, 0xc, 0x80, 0x6,

    /* U+0042 "B" */
    0x3f, 0xf8, 0x30, 0x9, 0x30, 0x9, 0x3f, 0xf8,
    0x30, 0x6, 0x30, 0x3, 0x30, 0x6, 0x3f, 0xf8,

    /* U+0043 "C" */
    0xb, 0xf8, 0xa, 0x1, 0x83, 0x0, 0x11, 0x80,
    0x0, 0x60, 0x0, 0xc, 0x0, 0x42, 0x80, 0x70,
    0x2f, 0xe0,

    /* U+0044 "D" */
    0x3f, 0xe4, 0xc, 0x3, 0x43, 0x0, 0x30, 0xc0,
    0xc, 0x30, 0x3, 0xc, 0x0, 0xc3, 0x0, 0xd0,
    0xff, 0x90,

    /* U+0045 "E" */
    0x3f, 0xfc, 0x30, 0x0, 0x30, 0x0, 0x3f, 0xf4,
    0x30, 0x0, 0x30, 0x0, 0x30, 0x0, 0x3f, 0xfc,

    /* U+0046 "F" */
    0x3f, 0xfc, 0xc0, 0x3, 0x0, 0xf, 0xfd, 0x30,
    0x0, 0xc0, 0x3, 0x0, 0xc, 0x0,

    /* U+0047 "G" */
    0xb, 0xf8, 0xa, 0x1, 0x83, 0x0, 0x1, 0x81,
    0xfc, 0x60, 0x2, 0xc, 0x0, 0xc2, 0x80, 0x60,
    0x1f, 0xe0,

    /* U+0048 "H" */
    0x30, 0x3, 0x30, 0x3, 0x30, 0x3, 0x3f, 0xff,
    0x30, 0x3, 0x30, 0x3, 0x30, 0x3, 0x30, 0x3,

    /* U+0049 "I" */
    0x33, 0x33, 0x33, 0x33,

    /* U+004A "J" */
    0x0, 0x60, 0x6, 0x0, 0x60, 0x6, 0x0, 0x64,
    0x6, 0xa0, 0x92, 0xf8,

    /* U+004B "K" */
    0x30, 0xd, 0xc, 0xc, 0x3, 0xc, 0x0, 0xcd,
    0x0, 0x3d, 0xc0, 0xc, 0xc, 0x3, 0x1, 0xc0,
    0xc0, 0x18,

    /* U+004C "L" */
    0x30, 0x0, 0xc0, 0x3, 0x0, 0xc, 0x0, 0x30,
    0x0, 0xc0, 0x3, 0x0, 0xf, 0xfe,

    /* U+004D "M" */
    0x34, 0x0, 0xb3, 0xc0, 0xf, 0x39, 0x2, 0x73,
    0x30, 0x33, 0x32, 0x45, 0x33, 0x8, 0xc3, 0x30,
    0xa8, 0x33, 0x3, 0x3,

    /* U+004E "N" */
    0x34, 0x3, 0x3c, 0x3, 0x33, 0x3, 0x31, 0x83,
    0x30, 0x93, 0x30, 0x33, 0x30, 0x1f, 0x30, 0xb,

    /* U+004F "O" */
    0x7, 0xf8, 0xa, 0x1, 0xc3, 0x0, 0x19, 0x80,
    0x3, 0x60, 0x0, 0xcc, 0x0, 0x62, 0x80, 0x70,
    0x1f, 0xe0,

    /* U+0050 "P" */
    0x3f, 0xf4, 0x30, 0xd, 0x30, 0x5, 0x30, 0xd,
    0x3f, 0xf4, 0x30, 0x0, 0x30, 0x0, 0x30, 0x0,

    /* U+0051 "Q" */
    0x7, 0xf8, 0xa, 0x1, 0xc3, 0x0, 0x19, 0x80,
    0x3, 0x60, 0x0, 0xcc, 0x1, 0x62, 0x80, 0xf0,
    0x1f, 0xea, 0x0, 0x0, 0x0,

    /* U+0052 "R" */
    0x3f, 0xf4, 0x30, 0xd, 0x30, 0xc, 0x3f, 0xf4,
    0x30, 0x30, 0x30, 0x28, 0x30, 0xc, 0x30, 0x6,

    /* U+0053 "S" */
    0x1f, 0xe0, 0x30, 0x1c, 0x30, 0x0, 0x1a, 0x40,
    0x0, 0x68, 0x0, 0xc, 0x70, 0xc, 0x1f, 0xf4,

    /* U+0054 "T" */
    0xff, 0xfc, 0xc, 0x0, 0x30, 0x0, 0xc0, 0x3,
    0x0, 0xc, 0x0, 0x30, 0x0, 0xc0,

    /* U+0055 "U" */
    0x30, 0x3, 0x30, 0x3, 0x30, 0x3, 0x30, 0x3,
    0x30, 0x3, 0x30, 0x2, 0x24, 0x9, 0xb, 0xf4,

    /* U+0056 "V" */
    0xc0, 0x9, 0x60, 0xc, 0x30, 0x18, 0x24, 0x30,
    0xc, 0x60, 0xc, 0x90, 0x6, 0xc0, 0x3, 0x80,

    /* U+0057 "W" */
    0x90, 0x38, 0x6, 0x60, 0x3c, 0xc, 0x30, 0x58,
    0xc, 0x20, 0x86, 0x18, 0x18, 0xc3, 0x24, 0xc,
    0x82, 0x30, 0xe, 0x41, 0xa0, 0x7, 0x0, 0xd0,

    /* U+0058 "X" */
    0x60, 0xc, 0x24, 0x34, 0xd, 0x90, 0x3, 0x80,
    0x7, 0xc0, 0xc, 0x60, 0x34, 0x24, 0x90, 0xc,

    /* U+0059 "Y" */
    0x34, 0x3, 0x43, 0x2, 0x80, 0x61, 0xc0, 0xa,
    0xc0, 0x0, 0xd0, 0x0, 0x20, 0x0, 0x8, 0x0,
    0x2, 0x0,

    /* U+005A "Z" */
    0x7f, 0xfc, 0x0, 0x24, 0x0, 0x60, 0x1, 0x80,
    0x3, 0x0, 0xc, 0x0, 0x30, 0x0, 0xbf, 0xfd,

    /* U+005B "[" */
    0x38, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
    0x30, 0x30, 0x38,

    /* U+005C "\\" */
    0x90, 0x8, 0x3, 0x0, 0x50, 0x8, 0x3, 0x0,
    0x50, 0x8, 0x3, 0x0,

    /* U+005D "]" */
    0x2c, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8,
    0x8, 0x8, 0x2c,

    /* U+005E "^" */
    0xd, 0x5, 0xc3, 0x14, 0x83,

    /* U+005F "_" */
    0xff, 0xc0,

    /* U+0060 "`" */
    0x30, 0x8,

    /* U+0061 "a" */
    0x1e, 0xd0, 0x80, 0xc0, 0x1b, 0x9, 0xc, 0x60,
    0x70, 0xe9, 0x90,

    /* U+0062 "b" */
    0x20, 0x0, 0x80, 0x2, 0x0, 0xe, 0xb4, 0x30,
    0x30, 0x80, 0x52, 0x1, 0x4c, 0xc, 0x3a, 0xd0,

    /* U+0063 "c" */
    0x1e, 0x80, 0xc0, 0xc6, 0x0, 0x18, 0x0, 0x30,
    0x30, 0x7f, 0x40,

    /* U+0064 "d" */
    0x0, 0x20, 0x0, 0x80, 0x2, 0x7, 0xa8, 0x30,
    0x31, 0x80, 0x86, 0x2, 0xc, 0xc, 0x1e, 0xa0,

    /* U+0065 "e" */
    0x1e, 0xd0, 0x80, 0xc7, 0xab, 0x18, 0x0, 0x30,
    0x20, 0x7f, 0x40,

    /* U+0066 "f" */
    0x1d, 0x20, 0x20, 0xbd, 0x20, 0x20, 0x20, 0x20,
    0x20,

    /* U+0067 "g" */
    0x1e, 0xa0, 0xc0, 0xc6, 0x2, 0x18, 0x8, 0x30,
    0x30, 0x7a, 0x80, 0x2, 0xc, 0xc, 0x1e, 0xc0,

    /* U+0068 "h" */
    0x20, 0x0, 0x80, 0x2, 0x0, 0xe, 0xb4, 0x30,
    0x30, 0x80, 0x82, 0x2, 0x8, 0x8, 0x20, 0x20,

    /* U+0069 "i" */
    0x20, 0x2, 0x22, 0x22, 0x20,

    /* U+006A "j" */
    0xc, 0x10, 0x3, 0xc, 0x30, 0xc3, 0xc, 0x30,
    0xcd,

    /* U+006B "k" */
    0x20, 0x0, 0x80, 0x2, 0x0, 0x8, 0x24, 0x22,
    0x40, 0xf8, 0x3, 0x24, 0x8, 0x30, 0x20, 0x30,

    /* U+006C "l" */
    0x22, 0x22, 0x22, 0x22, 0x20,

    /* U+006D "m" */
    0x3a, 0xca, 0xd3, 0x3, 0x3, 0x20, 0x30, 0x32,
    0x3, 0x3, 0x20, 0x30, 0x32, 0x3, 0x3,

    /* U+006E "n" */
    0x2a, 0xd0, 0xc0, 0xc2, 0x2, 0x8, 0x8, 0x20,
    0x20, 0x80, 0x80,

    /* U+006F "o" */
    0x1f, 0xd0, 0xc0, 0xc6, 0x1, 0x98, 0x6, 0x30,
    0x30, 0x7f, 0x40,

    /* U+0070 "p" */
    0x3a, 0xd0, 0xc0, 0xc2, 0x1, 0x48, 0x5, 0x30,
    0x30, 0xeb, 0x42, 0x0, 0x8, 0x0, 0x20, 0x0,

    /* U+0071 "q" */
    0x1e, 0xa0, 0xc0, 0xc6, 0x2, 0x18, 0x8, 0x30,
    0x30, 0x7a, 0x80, 0x2, 0x0, 0x8, 0x0, 0x20,

    /* U+0072 "r" */
    0x27, 0xd, 0x2, 0x0, 0x80, 0x20, 0x8, 0x0,

    /* U+0073 "s" */
    0x2f, 0x86, 0x4, 0x29, 0x0, 0x1d, 0x50, 0x62,
    0xf8,

    /* U+0074 "t" */
    0x10, 0x30, 0x30, 0xb9, 0x30, 0x30, 0x30, 0x30,
    0x2d,

    /* U+0075 "u" */
    0x20, 0x20, 0x80, 0x82, 0x2, 0x8, 0x8, 0x30,
    0x30, 0x7a, 0xc0,

    /* U+0076 "v" */
    0x90, 0x31, 0x81, 0x83, 0xc, 0x5, 0x20, 0xe,
    0x40, 0x2c, 0x0,

    /* U+0077 "w" */
    0x80, 0xe0, 0x56, 0xb, 0xc, 0x31, 0x60, 0xc2,
    0x21, 0x94, 0x1a, 0xe, 0x0, 0xd0, 0xb0,

    /* U+0078 "x" */
    0x60, 0x92, 0x58, 0xf, 0x0, 0xf0, 0x25, 0x89,
    0x9,

    /* U+0079 "y" */
    0x90, 0x31, 0x81, 0x83, 0x8, 0x5, 0x30, 0xe,
    0x40, 0x2c, 0x0, 0x60, 0x2, 0x0, 0x38, 0x0,

    /* U+007A "z" */
    0x6a, 0xd0, 0x18, 0x7, 0x0, 0xc0, 0x30, 0xb,
    0xfd,

    /* U+007B "{" */
    0x1c, 0x30, 0x30, 0x30, 0x30, 0x90, 0x30, 0x30,
    0x30, 0x30, 0x18,

    /* U+007C "|" */
    0xff, 0xff, 0xfc,

    /* U+007D "}" */
    0x28, 0xc, 0xc, 0xc, 0x8, 0x7, 0x8, 0xc,
    0xc, 0xc, 0x28,

    /* U+007E "~" */
    0x2c, 0x18, 0xc8, 0x95, 0xa, 0x0,

    /* U+007F "" */

    /* U+2103 "℃" */
    0x24, 0xa, 0x80, 0x48, 0x74, 0x70, 0x24, 0xc0,
    0x18, 0x1, 0x80, 0x0, 0x1, 0x80, 0x0, 0x1,
    0x80, 0x0, 0x1, 0x80, 0x0, 0x0, 0xc0, 0xc,
    0x0, 0x70, 0x34, 0x0, 0x1b, 0x90,

    /* U+5149 "光" */
    0x0, 0x20, 0x0, 0x14, 0x20, 0x30, 0xc, 0x20,
    0x90, 0x3, 0x21, 0x80, 0x0, 0x20, 0x0, 0x7f,
    0xff, 0xfc, 0x1, 0x45, 0x0, 0x2, 0x45, 0x0,
    0x3, 0x5, 0xc, 0x9, 0x5, 0xc, 0x74, 0x3,
    0xf8, 0x0, 0x0, 0x0,

    /* U+8C03 "调" */
    0x0, 0x0, 0x0, 0x22, 0xaa, 0xb0, 0x98, 0x20,
    0x80, 0x22, 0xe6, 0x28, 0x82, 0x8, 0x22, 0x6a,
    0xa0, 0x88, 0x0, 0x82, 0x23, 0xb2, 0xe, 0xc8,
    0x88, 0x32, 0x2a, 0x20, 0x24, 0x6, 0x80, 0x0,
    0x0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 61, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 0, .adv_w = 47, .box_w = 2, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 4, .adv_w = 73, .box_w = 3, .box_h = 3, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 7, .adv_w = 126, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 23, .adv_w = 120, .box_w = 8, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 45, .adv_w = 160, .box_w = 10, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 65, .adv_w = 136, .box_w = 9, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 83, .adv_w = 45, .box_w = 1, .box_h = 3, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 84, .adv_w = 83, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 95, .adv_w = 83, .box_w = 3, .box_h = 11, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 104, .adv_w = 86, .box_w = 5, .box_h = 4, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 109, .adv_w = 116, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 120, .adv_w = 49, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 123, .adv_w = 92, .box_w = 5, .box_h = 1, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 125, .adv_w = 49, .box_w = 3, .box_h = 2, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 127, .adv_w = 73, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 139, .adv_w = 116, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 153, .adv_w = 116, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 161, .adv_w = 116, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 175, .adv_w = 116, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 189, .adv_w = 116, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 203, .adv_w = 116, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 217, .adv_w = 116, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 231, .adv_w = 116, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 245, .adv_w = 116, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 259, .adv_w = 116, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 273, .adv_w = 49, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 278, .adv_w = 48, .box_w = 3, .box_h = 8, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 284, .adv_w = 116, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 297, .adv_w = 116, .box_w = 7, .box_h = 4, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 304, .adv_w = 116, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 315, .adv_w = 99, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 327, .adv_w = 164, .box_w = 10, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 347, .adv_w = 127, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 363, .adv_w = 136, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 379, .adv_w = 138, .box_w = 9, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 397, .adv_w = 140, .box_w = 9, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 415, .adv_w = 123, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 431, .adv_w = 116, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 445, .adv_w = 143, .box_w = 9, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 463, .adv_w = 143, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 479, .adv_w = 46, .box_w = 2, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 483, .adv_w = 105, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 495, .adv_w = 130, .box_w = 9, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 513, .adv_w = 108, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 527, .adv_w = 173, .box_w = 10, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 547, .adv_w = 141, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 563, .adv_w = 151, .box_w = 9, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 581, .adv_w = 126, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 597, .adv_w = 151, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 618, .adv_w = 130, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 634, .adv_w = 124, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 650, .adv_w = 112, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 664, .adv_w = 138, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 680, .adv_w = 120, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 696, .adv_w = 190, .box_w = 12, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 720, .adv_w = 122, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 736, .adv_w = 120, .box_w = 9, .box_h = 8, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 754, .adv_w = 123, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 770, .adv_w = 83, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 781, .adv_w = 73, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 793, .adv_w = 83, .box_w = 4, .box_h = 11, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 804, .adv_w = 89, .box_w = 5, .box_h = 4, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 809, .adv_w = 81, .box_w = 6, .box_h = 1, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 811, .adv_w = 58, .box_w = 4, .box_h = 2, .ofs_x = 0, .ofs_y = 7},
    {.bitmap_index = 813, .adv_w = 112, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 824, .adv_w = 113, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 840, .adv_w = 107, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 851, .adv_w = 113, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 867, .adv_w = 110, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 878, .adv_w = 60, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 887, .adv_w = 113, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 903, .adv_w = 112, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 919, .adv_w = 41, .box_w = 2, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 924, .adv_w = 43, .box_w = 3, .box_h = 12, .ofs_x = -1, .ofs_y = -3},
    {.bitmap_index = 933, .adv_w = 101, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 949, .adv_w = 41, .box_w = 2, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 954, .adv_w = 173, .box_w = 10, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 969, .adv_w = 112, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 980, .adv_w = 114, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 991, .adv_w = 113, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 1007, .adv_w = 113, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 1023, .adv_w = 66, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1031, .adv_w = 97, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1040, .adv_w = 61, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1049, .adv_w = 112, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1060, .adv_w = 100, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1071, .adv_w = 156, .box_w = 10, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1086, .adv_w = 95, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1095, .adv_w = 101, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 1111, .adv_w = 95, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1120, .adv_w = 83, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 1131, .adv_w = 45, .box_w = 1, .box_h = 11, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 1134, .adv_w = 83, .box_w = 4, .box_h = 11, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 1145, .adv_w = 116, .box_w = 7, .box_h = 3, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 1151, .adv_w = 192, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1151, .adv_w = 190, .box_w = 12, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1181, .adv_w = 192, .box_w = 12, .box_h = 12, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 1217, .adv_w = 192, .box_w = 11, .box_h = 12, .ofs_x = 0, .ofs_y = -2}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_1[] = {
    0x0, 0x3046, 0x6b00
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 96, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 8451, .range_length = 27393, .glyph_id_start = 97,
        .unicode_list = unicode_list_1, .glyph_id_ofs_list = NULL, .list_length = 3, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
    }
};

/*-----------------
 *    KERNING
 *----------------*/


/*Map glyph_ids to kern left classes*/
static const uint8_t kern_left_class_mapping[] =
{
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 2, 0, 3, 4, 5,
    3, 0, 0, 0, 6, 7, 0, 0,
    3, 8, 9, 10, 0, 11, 12, 13,
    14, 15, 16, 17, 0, 0, 0, 0,
    0, 0, 18, 19, 20, 0, 19, 21,
    22, 23, 0, 0, 24, 0, 23, 23,
    19, 19, 0, 25, 26, 27, 23, 28,
    29, 30, 31, 32, 0, 0, 0, 0,
    0, 0, 0, 0
};

/*Map glyph_ids to kern right classes*/
static const uint8_t kern_right_class_mapping[] =
{
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 0, 2, 0, 0, 0,
    2, 0, 0, 3, 0, 0, 0, 0,
    2, 0, 2, 0, 0, 4, 5, 6,
    7, 8, 9, 10, 0, 0, 0, 0,
    0, 0, 11, 0, 12, 12, 12, 13,
    14, 0, 0, 0, 0, 0, 15, 15,
    12, 0, 12, 15, 16, 17, 15, 18,
    19, 20, 21, 22, 0, 0, 0, 0,
    0, 0, 0, 0
};

/*Kern values between classes*/
static const int8_t kern_class_values[] =
{
    0, -6, 0, -13, -6, -12, -8, 0,
    -14, 0, 0, -3, 0, 0, 0, 0,
    0, -5, -3, 0, -4, 0, 0, 0,
    0, -4, 0, 0, 0, 0, -3, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -6, 0, -4, -4,
    0, -3, -4, -6, -5, -3, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -2, 0, -2,
    0, 0, 0, 0, 0, 0, 0, 0,
    -13, 0, -19, -7, 0, 0, 0, 0,
    0, -3, -8, -6, 0, -6, -4, -6,
    0, 0, 0, 0, 0, 0, 0, -7,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, -12,
    -10, 0, -8, 0, 0, -7, 0, -18,
    -6, -18, -17, 0, -21, 0, 0, 0,
    0, 0, 0, 0, 0, -13, -10, 0,
    -11, 0, -12, 0, -9, 0, 0, 0,
    0, -5, 0, -2, -6, -6, 0, -6,
    0, -3, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -6, 0, -3, -3, -2,
    -6, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -4, -4, 0, -7, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -12, -6, -15, -14,
    0, 0, 0, 0, 0, 0, -7, -7,
    0, -8, -4, -6, 0, -6, -4, -8,
    -6, -6, -6, 0, -3, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -12, -3, -16, 0, 0, 0, 0, 0,
    0, 0, -3, -10, 0, -4, -2, -3,
    0, 0, 0, -3, 0, -6, -8, -4,
    -12, 0, 0, 0, 0, 0, 0, 0,
    -4, -10, 0, -11, -6, -8, 0, 0,
    0, 0, 0, 0, 0, -6, 0, 0,
    0, 0, 0, 0, 0, 0, 0, -3,
    0, -3, 0, 0, 0, -5, -3, 0,
    0, 0, -14, -5, -18, 0, 0, 0,
    0, 0, 0, 0, -10, -11, 0, -10,
    -10, -8, 0, -5, -4, -6, -5, -7,
    0, -3, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -2, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -7, 0, -5, -3, 0, -5, 0,
    0, 0, -2, 0, 0, 0, 0, -3,
    -3, 0, -2, 0, -3, 0, 0, -8,
    0, -4, -10, -3, -11, -2, 0, 0,
    -2, 0, 0, 0, 0, -4, -3, -6,
    -4, -2, 0, 0, 0, -2, 0, -6,
    -6, 0, -3, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -3, 0, 0,
    -3, 0, -3, 0, 0, 0, 0, 0,
    0, 0, -6, -7, -5, -5, 0, 0,
    -4, 0, 0, 0, 0, 0, 0, 0,
    0, -4, 0, 0, -6, 0, -10, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, -5,
    0, -4, -8, 0, -9, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -3, 0, -10, 0, -4,
    -3, 0, -7, 0, 0, -3, 0, -3,
    0, -2, -4, -4, -4, 0, -5, 0,
    -7, 0, -17, -11, 0, 0, 0, -9,
    -9, 0, -3, -3, 0, -3, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -6, 0, -2, -7, 0, -8, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -4, 0, 0, -4, 0, 0, -4,
    -3, -4, 0, 0, -3, 0, 0, 0,
    0, 0, -5, 0, -12, -6, 0, 0,
    0, -5, -5, 0, -4, -4, 0, -4,
    0, 0, 0, 0, 0, 0, 0, 0,
    -3, 0, -8, -4, 0, 0, 0, -3,
    -4, 0, -4, -3, 0, -3, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -8, 0, -3, 0, 0, -6, 0,
    -4, -6, 0, -3, 0, 0, 0, 0,
    0, 0, 0, 0, -5, 0, -12, -6,
    0, 0, 0, 0, -5, 0, -3, -4,
    0, -4, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -6, 0, -3,
    0, 0, -7, 0, -4, -6, 0, -6,
    0, 0, 0, 0, 0, 0, 0, 0
};


/*Collect the kern class' data in one place*/
static const lv_font_fmt_txt_kern_classes_t kern_classes =
{
    .class_pair_values   = kern_class_values,
    .left_class_mapping  = kern_left_class_mapping,
    .right_class_mapping = kern_right_class_mapping,
    .left_class_cnt      = 32,
    .right_class_cnt     = 22,
};

/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LV_VERSION_CHECK(8, 0, 0)
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = &kern_classes,
    .kern_scale = 16,
    .cmap_num = 2,
    .bpp = 2,
    .kern_classes = 1,
    .bitmap_format = 0,
#if LV_VERSION_CHECK(8, 0, 0)
    .cache = &cache
#endif
};


/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LV_VERSION_CHECK(8, 0, 0)
const lv_font_t font_cn_12 = {
#else
lv_font_t font_cn_12 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 13,          /*The maximum line height required by the font*/
    .base_line = 3,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -4,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc           /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
};



#endif /*#if FONT_CN_12*/

