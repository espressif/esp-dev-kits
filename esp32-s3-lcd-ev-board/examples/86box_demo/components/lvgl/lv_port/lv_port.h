#ifndef LV_PORT_H
#define LV_PORT_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize related work for lvgl.
 *
 */
void lv_port_init(void);

/**
 * @brief Take the semaphore.
 * @note  It should be called before manipulate lvgl gui.
 *
 */
void lv_port_sem_take(void);

/**
 * @brief Give the semaphore.
 * @note  It should be called after manipulate lvgl gui.
 *
 */
void lv_port_sem_give(void);

#ifdef __cplusplus
}
#endif

#endif
