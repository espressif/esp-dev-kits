#ifndef LVGL_PORT_H
#define LVGL_PORT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    struct {
        uint16_t width;
        uint16_t height;
        uint32_t buf_size;
        int buf_caps;
    } display;
    uint8_t tick_period;
    struct {
        uint8_t period;
        uint8_t core_id;
        int priority;
    } task;
} lvgl_port_config_t;

void lvgl_sem_take(void);
void lvgl_sem_give(void);
void lvgl_port(lvgl_port_config_t *config);

#ifdef __cplusplus
}
#endif

#endif