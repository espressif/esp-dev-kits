#ifndef __VIDOPLAYER_H
#define __VIDOPLAYER_H
#include "jpegd2.h"

#ifdef __cplusplus 
extern "C" {
#endif

void avi_play(const char *filename, uint8_t *outbuffer,
                const size_t outbuffer_width, const size_t outbuffer_height,
                lcd_write_cb lcd_cb, const size_t lcd_width, const size_t lcd_height);

#ifdef __cplusplus 
}
#endif

#endif

