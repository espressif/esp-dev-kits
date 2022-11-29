#include <stdio.h>
#include "esp_system.h"
#ifdef ESP_IDF_VERSION
#include "esp_log.h"
#include "bsp_indev.h"
#endif
#include "lvgl.h"
#include "lvgl_port.h"
#include "ui.h"
#include "ui_menu.h"
#include <math.h>
#include "lv_example_pub.h"

static const char *TAG = "ui";
static lv_group_t *group;

void ui_init(void)
{
    group = lv_group_create();
    lv_group_set_default(group);
    lv_indev_t *indev = lv_indev_get_next(NULL);
    if (LV_INDEV_TYPE_ENCODER == lv_indev_get_type(indev)) {
        printf("add group for encoder\n");
        lv_indev_set_group(indev, group);
        lv_group_focus_freeze(group, false);
    }

    // {
    //     double x=1.5;
    //     for (size_t i = 0; i < 20; i++)
    //     {
    //         x=(pow(x*x+1, 1.0/3.0));
    //         printf("x=%f\n", x);
    //     }

    // }

    lv_home_create();
}

void ui_add_obj_to_encoder_group(lv_obj_t *obj)
{
    lv_group_add_obj(group, obj);
}

void ui_remove_all_objs_from_encoder_group(void)
{
    lv_group_remove_all_objs(group);
}

uint32_t ui_get_num_offset(uint32_t num, int32_t max, int32_t offset)
{
    if (num >= max) {
        printf("[ERROR] num should less than max\n");
        return num;
    }

    uint32_t i;
    if (offset >= 0) {
        i = (num + offset) % max;
    } else {
        offset = max + (offset % max);
        i = (num + offset) % max;
    }

    return i;
}