#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifndef GT1151_IRQ_PIN
#define GT1151_IRQ_PIN     108
#endif

#ifndef GT1151_TOUCH_WIDTH
#define GT1151_TOUCH_WIDTH  800
#endif

#ifndef GT1151_TOUCH_HEIGHT
#define GT1151_TOUCH_HEIGHT 480
#endif

#ifndef GT1151_SUPPORT_POINTS
#define GT1151_SUPPORT_POINTS 10
#endif

int gt1151_pos_read(uint16_t *xpos, uint16_t *ypos);

int gt1151_init(uint8_t i2c_addr);

#ifdef __cplusplus
}
#endif
