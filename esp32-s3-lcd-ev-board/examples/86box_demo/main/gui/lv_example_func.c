
#include "lv_demo.h"
#include "./lv_example_func.h"

#if LV_BUILD_EXAMPLES && LV_USE_FLEX

#define TIME_ON_TRIGGER  50

/**********************
 *   STATIC FUNCTIONS
 **********************/

void lv_func_create_layer(lv_layer_t * create_layer)
{
    bool result = false;
    result = create_layer->enter_cb(create_layer);
    if(true == result){
        LV_LOG_USER("[+] Create lv_layer:%s, %x", create_layer->lv_obj_name, create_layer->lv_obj_user);
    }

    if((true == result) && (NULL == create_layer->timer_handle)){
        create_layer->timer_handle = lv_timer_create(create_layer->timer_cb, TIME_ON_TRIGGER, NULL);
        //lv_timer_set_repeat_count(create_layer->timer_handle, 10);
        LV_LOG_USER("[+] Create lv_timer:%s", create_layer->lv_obj_name);
    }

    if(create_layer->lv_show_layer){
        create_layer->lv_show_layer->lv_obj_user = create_layer->lv_obj_layer;
        result = create_layer->lv_show_layer->enter_cb(create_layer->lv_show_layer);
        if(true == result){
            LV_LOG_USER("[+] Create show lv_layer:%s", create_layer->lv_show_layer->lv_obj_name);
        }

        if((true == result) && (NULL == create_layer->lv_show_layer->timer_handle)){
            create_layer->lv_show_layer->timer_handle = lv_timer_create(create_layer->lv_show_layer->timer_cb, TIME_ON_TRIGGER, NULL);
            LV_LOG_USER("[+] Create show lv_timer:%s", create_layer->lv_show_layer->lv_obj_name);
        }
    }
}

void lv_func_goto_layer(lv_layer_t * src_layer, lv_layer_t * dst_layer)
{
    bool result = false;

    lv_timer_enable(false);

    if(src_layer){
        if(src_layer->lv_obj_layer){

            if(src_layer->lv_show_layer){
                result = src_layer->exit_cb(src_layer->lv_show_layer);
                LV_LOG_USER("[-] Delete show lv_layer:%s", src_layer->lv_show_layer->lv_obj_name);
                lv_obj_del_async(src_layer->lv_show_layer->lv_obj_layer);
                src_layer->lv_show_layer->lv_obj_layer = NULL;

                if(src_layer->lv_show_layer->timer_handle){
                    LV_LOG_USER("[-] Delete show lv_timer:%s", src_layer->lv_show_layer->lv_obj_name);
                    lv_timer_del(src_layer->lv_show_layer->timer_handle);
                    src_layer->lv_show_layer->timer_handle = NULL;
                }
            }

            result = src_layer->exit_cb(src_layer);
            LV_LOG_USER("[-] Delete lv_layer :%s", src_layer->lv_obj_name);
            lv_obj_del_async(src_layer->lv_obj_layer);
            src_layer->lv_obj_layer = NULL;
        }

        if(src_layer->timer_handle){
            LV_LOG_USER("[-] Delete lv_timer :%s", src_layer->lv_obj_name);
            lv_timer_del(src_layer->timer_handle);
            src_layer->timer_handle = NULL;
        }
    }

    if(dst_layer){
        if(NULL == dst_layer->lv_obj_layer){
            lv_func_create_layer(dst_layer);
        }
        else{
            LV_LOG_USER("%s != NULL", dst_layer->lv_obj_name);
        }
    }

    lv_timer_enable(true);
}

#endif