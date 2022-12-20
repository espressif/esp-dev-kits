#ifndef LV_PORT_H
#define LV_PORT_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

void lv_port_init(void);

void lv_port_sem_take(void);

void lv_port_sem_give(void);

#ifdef __cplusplus
}
#endif

#endif
