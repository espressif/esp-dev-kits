#ifndef BSP_CAMERA_H
#define BSP_CAMERA_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*bsp_camera_frame_cb_t)(void *frame_data, int frame_size);

esp_err_t bsp_camera_init(void);

void bsp_camera_frame_cb_register(bsp_camera_frame_cb_t cb);

#ifdef __cplusplus
}
#endif

#endif