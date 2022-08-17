//#include "../lv_examples.h"
#include "lv_demo.h"
#include "./lv_example_func.h"
#include "./lv_example_image.h"

#if LV_BUILD_EXAMPLES

static bool ctrl_layer_enter_cb(struct lv_layer_t * layer);
static bool ctrl_layer_exit_cb(struct lv_layer_t * layer);
static void ctrl_layer_timer_cb(lv_timer_t * tmr);

lv_layer_t ctrl_Layer ={
    .lv_obj_name    = "ctrl_Layer",
    .lv_obj_layer   = NULL,
    .lv_show_layer  = &show_set_layer,
    //.lv_show_layer  = NULL,
    .enter_cb       = ctrl_layer_enter_cb,
    .exit_cb        = ctrl_layer_exit_cb,
    .timer_cb       = ctrl_layer_timer_cb,
};

const water_heat_info_list_t water_heat_info[7] ={
    {&detail_powerBig_B, &detail_powerBig_Y, "电源"},
    {&detail_fun_1_B, &detail_fun_1_y, "瀑布洗"},
    {&detail_fun_2_B, &detail_fun_2_y, "舒适洗"},
    {&detail_fun_3_B, &detail_fun_3_y, "节能洗"},
    {&detail_fun_4_B, &detail_fun_4_y, "抑菌"},
    {&detail_fun_5_B, &detail_fun_5_y, "智慧洗"},
    {&detail_fun_6_B, &detail_fun_6_y, "自动关机"},
    
};

static uint32_t period_ctrl_next, list;

static lv_obj_t * label_temp_set;
static lv_obj_t * btn_heater_ctrl[USER_CTRLBTN_MAX];
static lv_obj_t * bg_heater_ctrl[USER_CTRLBTN_MAX];

static void return_btn_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * button = lv_event_get_target(e);
    //uint8_t coverflowPage = lv_event_get_user_data(e);

    LV_LOG_USER("SHORT_CLICKED %d,coverflowPage:%X, buttton:%X,%d", code, 0, button, button->state);

    if(code == LV_EVENT_SHORT_CLICKED) {
        LV_LOG_USER("SHORT_CLICKED %d,coverflowPage:%X, buttton:%X,%d", code, 0, button, button->state);
        //lv_obj_add_flag(lv_tv_mainmenu, LV_OBJ_FLAG_HIDDEN);
        if(1 == *(ctrl_Layer.lv_obj_user)){
            main_Layer.lv_obj_user = &ctrl_Layer;
            LV_LOG_USER("->0,1, %x,%x", main_Layer.lv_obj_user, &ctrl_Layer);
        }
        else{
            main_Layer.lv_obj_user = NULL;
            LV_LOG_USER("->0,0, %x,%x", main_Layer.lv_obj_user, &ctrl_Layer);
        }
        LV_LOG_USER("->0,1, %x,%x", main_Layer.lv_obj_user, &ctrl_Layer);
        lv_func_goto_layer(&ctrl_Layer, &main_Layer);
    }
}

static void ctrlBtn_tempset_event_cb(lv_event_t * e)
{
    uint8_t action, tempSet;
    uint8_t tempBuf[20];
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * button = lv_event_get_target(e);

    if(code == LV_EVENT_SHORT_CLICKED) {
        action = lv_event_get_user_data(e);
        tempSet = atoi(lv_label_get_text(label_temp_set));
        reload_screenOff_timer();
        LV_LOG_USER("action %d, %s, %s:%d", action, action? "+":"-", lv_label_get_text(label_temp_set), tempSet);

        if(action){
            if(tempSet < 45){
                tempSet++;
            }
        }
        else{
            if(tempSet >35){
                tempSet--;
            }
        }
        
        //lv_obj_set_style_text_color(label_temp_set, lv_color_hex(COLOUR_GREY), 0);
        sprintf(tempBuf, "%d", tempSet);
        lv_label_set_text(label_temp_set, tempBuf);
    }
}

static void ctrlBtn_event_cb(lv_event_t * e)
{
    uint8_t i;
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * button = lv_event_get_target(e);

    if(code == LV_EVENT_SHORT_CLICKED) {
        uint8_t btn = lv_event_get_user_data(e);
        reload_screenOff_timer();

        LV_LOG_USER("code %d,btn:%X,checked:%X,%d", code, btn,\
                (button->state&LV_STATE_CHECKED), ((LV_STATE_CHECKED == button->state&LV_STATE_CHECKED)));

        if(0 == btn){
            if(LV_STATE_CHECKED == (button->state&LV_STATE_CHECKED)) {
                for(i = 0; i< USER_CTRLBTN_MAX; i++){
                    lv_obj_clear_state(btn_heater_ctrl[i], LV_STATE_CHECKED);
                    if(0 == i){
                        lv_img_set_src(bg_heater_ctrl[i], water_heat_info[i].addr_open);
                    }
                    else{
                        lv_obj_set_style_text_color(bg_heater_ctrl[i], lv_color_hex(COLOUR_WHITE), 0);
                    }
                    //LV_LOG_USER("clear checked:%X", button->state&LV_STATE_CHECKED);
                }
            }
            else {
                lv_obj_add_state(btn_heater_ctrl[0], LV_STATE_CHECKED);
                lv_img_set_src(bg_heater_ctrl[0], water_heat_info[0].addr_close);

                lv_obj_add_state(btn_heater_ctrl[2], LV_STATE_CHECKED);
                lv_obj_set_style_text_color(bg_heater_ctrl[2], lv_color_hex(COLOUR_YELLOW), 0);
            }
        }
        else{
            if(LV_STATE_CHECKED == (button->state&LV_STATE_CHECKED)) {
                lv_obj_clear_state(button, LV_STATE_CHECKED);
                //lv_img_set_src(bg_heater_ctrl[btn], water_heat_info[btn].addr_open);
                lv_obj_set_style_text_color(bg_heater_ctrl[btn], lv_color_hex(COLOUR_WHITE), 0);
                LV_LOG_USER("clear checked:%X", button->state&LV_STATE_CHECKED);
            }
            else {
                lv_obj_add_state(button, LV_STATE_CHECKED);
                //lv_img_set_src(bg_heater_ctrl[btn], water_heat_info[btn].addr_close);
                lv_obj_set_style_text_color(bg_heater_ctrl[btn], lv_color_hex(COLOUR_YELLOW), 0);
                LV_LOG_USER("set checked:%X", button->state&LV_STATE_CHECKED);

                lv_obj_add_state(btn_heater_ctrl[0], LV_STATE_CHECKED);
                lv_img_set_src(bg_heater_ctrl[0], water_heat_info[0].addr_close);
            }
        }
    }
}

uint8_t lv_create_ctrlPage(lv_obj_t * parent)
{
    int i;
    uint8_t temp[30];
    lv_obj_t *labelName;

    static lv_coord_t grid_2_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    //static lv_coord_t grid_2_col_dsc[] = {150, 150, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t grid_2_row_dsc[] = {
                100,  /*power key*/

                20,  /*current temp*/
                70,  /*Title "current temp*/

                50,  /*+ - temp set*/
                50,  /*Title "set temp"*/
                90,  /*list 3*/
                60,  /*list 3 40 pitch left*/
                LV_GRID_TEMPLATE_LAST
        };
    lv_obj_set_grid_dsc_array(parent, grid_2_col_dsc, grid_2_row_dsc);

    lv_obj_t *btn_heater_add = lv_btn_create(parent);
    lv_obj_remove_style_all(btn_heater_add);

    lv_obj_set_size(btn_heater_add, 70, 70);
    lv_obj_set_grid_cell(btn_heater_add, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 1+2, 1);

    lv_obj_add_event_cb(btn_heater_add, ctrlBtn_tempset_event_cb, LV_EVENT_SHORT_CLICKED, 1);
    lv_obj_clear_state(btn_heater_add, LV_STATE_CHECKED);

    lv_obj_t *bg_heater_add = lv_img_create(btn_heater_add);
    lv_img_set_src(bg_heater_add, &detail_add);
    lv_obj_align(bg_heater_add, LV_ALIGN_CENTER, 0, 0);
    
    lv_obj_t *btn_heater_dec = lv_btn_create(parent);
    lv_obj_remove_style_all(btn_heater_dec);

    lv_obj_set_size(btn_heater_dec, 70, 70);
    lv_obj_set_grid_cell(btn_heater_dec, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 1+2, 1);

    lv_obj_add_event_cb(btn_heater_dec, ctrlBtn_tempset_event_cb, LV_EVENT_SHORT_CLICKED, 0);
    lv_obj_clear_state(btn_heater_dec, LV_STATE_CHECKED);

    lv_obj_t *bg_heater_dec = lv_img_create(btn_heater_dec);
    lv_img_set_src(bg_heater_dec, &detail_dec);
    lv_obj_align(bg_heater_dec, LV_ALIGN_CENTER, 0, 0);

    for(i = 0; i< USER_CTRLBTN_MAX; i++){

        btn_heater_ctrl[i] = lv_obj_create(parent);
        //lv_obj_remove_style_all(btn_heater_ctrl[i]);

        if(0 != i){
            //lv_obj_set_border_width(btn_heater_ctrl[i], 0);
            lv_obj_set_style_bg_color(btn_heater_ctrl[i], lv_color_hex(COLOUR_BG_GREY), LV_STATE_CHECKED);
            lv_obj_set_style_bg_color(btn_heater_ctrl[i], lv_color_hex(COLOUR_BG_GREY), LV_PART_MAIN);
            lv_obj_set_size(btn_heater_ctrl[i], 141, 55);
            lv_obj_set_style_border_width(btn_heater_ctrl[i], 0, 0);
            lv_obj_set_style_pad_all(btn_heater_ctrl[i], 0, 0);
            lv_obj_set_grid_cell(btn_heater_ctrl[i], LV_GRID_ALIGN_CENTER, (i-1)%3, 1, LV_GRID_ALIGN_CENTER, 5 +(i-1)/3, 1);
            LV_LOG_USER("[%d,%d]",(i-1)%3, 4 +(i-1)/3);
        }
        else{
            lv_obj_remove_style_all(btn_heater_ctrl[i]);
            lv_obj_set_grid_cell(btn_heater_ctrl[i], LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 0, 1);
        }

        lv_obj_add_event_cb(btn_heater_ctrl[i], ctrlBtn_event_cb, LV_EVENT_SHORT_CLICKED, i);
        lv_obj_clear_state(btn_heater_ctrl[i], LV_STATE_CHECKED);
        if(0 == i){
            bg_heater_ctrl[i] = lv_img_create(btn_heater_ctrl[i]);
            lv_img_set_src(bg_heater_ctrl[i], water_heat_info[i].addr_open);
        }
        else{
            bg_heater_ctrl[i] = lv_label_create(btn_heater_ctrl[i]);
            //lv_obj_remove_style_all(bg_heater_ctrl[i]);
            lv_obj_set_style_text_font(bg_heater_ctrl[i], &font_lantingMid18, 0);
            lv_obj_set_style_text_color(bg_heater_ctrl[i], lv_color_hex(COLOUR_WHITE), 0);
            lv_label_set_text(bg_heater_ctrl[i], water_heat_info[i].btn_name);
        }
        lv_obj_align(bg_heater_ctrl[i], LV_ALIGN_CENTER, 0, 0);
    }

    /*text start*/
    label_temp_set = lv_label_create(parent);
    lv_obj_remove_style_all(label_temp_set);
    //lv_obj_set_style_text_color(label_temp_set, lv_color_hex(COLOUR_GREY), 0);
    lv_obj_set_style_text_color(label_temp_set, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(label_temp_set, &font_num50, 0);
    sprintf(temp, "%d", 42);
    lv_label_set_text(label_temp_set, temp);
    lv_obj_set_grid_cell(label_temp_set, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 3, 1);

    lv_obj_t * labelStatus = lv_label_create(parent);
    lv_obj_remove_style_all(labelStatus);
    //lv_obj_set_style_text_color(labelStatus, lv_color_hex(COLOUR_GREY), 0);
    lv_obj_set_style_text_color(labelStatus, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(labelStatus, &font_lanting24, 0);
    memset(temp, 0, sizeof(temp));
    sprintf(temp, "%s", "设置温度");
    lv_label_set_text(labelStatus, temp);
    //lv_obj_center(labelData);
    lv_obj_set_grid_cell(labelStatus, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 4, 1);

    /*text start*/
    lv_obj_t * label_temp_current = lv_label_create(parent);
    lv_obj_remove_style_all(label_temp_current);
    lv_obj_set_style_text_color(label_temp_current, lv_color_hex(COLOUR_GREY), 0);
    lv_obj_set_style_text_font(label_temp_current, &font_num36, 0);
    sprintf(temp, "%d", 32);
    lv_label_set_text(label_temp_current, temp);
    lv_obj_set_grid_cell(label_temp_current, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);

    labelStatus = lv_label_create(parent);
    lv_obj_remove_style_all(labelStatus);
    lv_obj_set_style_text_color(labelStatus, lv_color_hex(COLOUR_GREY), 0);
    lv_obj_set_style_text_font(labelStatus, &font_lantingMid18, 0);
    memset(temp, 0, sizeof(temp));
    sprintf(temp, "%s", "当前温度");
    lv_label_set_text(labelStatus, temp);
    //lv_obj_center(labelData);
    lv_obj_set_grid_cell(labelStatus, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 2, 1);

    lv_obj_t * btn_return = lv_btn_create(parent);
    lv_obj_remove_style_all(btn_return);
    lv_obj_set_size(btn_return, 100, 60);
    lv_obj_add_event_cb(btn_return, return_btn_event_cb, LV_EVENT_SHORT_CLICKED, parent);
    lv_obj_set_grid_cell(btn_return, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_t * labelReturn = lv_label_create(btn_return);
    lv_obj_remove_style_all(labelReturn);
    lv_obj_set_style_text_color(labelReturn, lv_color_white(), 0);
    lv_obj_set_style_text_font(labelReturn, &font_lanting24, 0);
    lv_label_set_text(labelReturn, "返回");
    lv_obj_center(labelReturn);
}

static bool ctrl_layer_enter_cb(struct lv_layer_t * layer)
{
	bool ret = false;

    LV_LOG_USER("lv_obj_name:%s", layer->lv_obj_name);

	if(NULL == layer->lv_obj_layer){
		ret = true;
		layer->lv_obj_layer = lv_obj_create(lv_scr_act());
        lv_obj_remove_style_all(layer->lv_obj_layer);
        lv_obj_set_size(layer->lv_obj_layer, 480, 480);

        lv_obj_t * lv_sec_menu_bg = lv_img_create(layer->lv_obj_layer);
        lv_obj_remove_style_all(lv_sec_menu_bg);
        //lv_obj_set_size(lv_top_menu_btn[i], lv_pct(100), lv_pct(100));
        lv_obj_set_size(lv_sec_menu_bg, 480, 480);
        lv_obj_align(lv_sec_menu_bg, LV_ALIGN_CENTER, 0, 0);

        lv_img_set_src(lv_sec_menu_bg, &global_bg1);

        /*
        lv_obj_t * lv_ctrl_menu_bg = lv_img_create(lv_sec_menu_bg);
        lv_img_set_src(lv_ctrl_menu_bg, &bg);
        lv_obj_align(lv_ctrl_menu_bg, LV_ALIGN_CENTER, 0, 0);
        */
        lv_create_ctrlPage(lv_sec_menu_bg);

        //lv_create_pulldown_bg(layer->lv_obj_layer);

        reload_screenOff_timer();
        
        period_ctrl_next = lv_tick_get();
        list = 0;
	}

	return ret;
}

static bool ctrl_layer_exit_cb(struct lv_layer_t * layer)
{
    LV_LOG_USER("");
}

static void ctrl_layer_timer_cb(lv_timer_t * tmr)
{
    //uint32_t idle_period_Test;
    int32_t isTmOut;

    isTmOut = (lv_tick_get() - (period_ctrl_next + 500));
    if(isTmOut > 0){
        period_ctrl_next = lv_tick_get();
        //lv_func_goto_layer(&ctrl_Layer, &main_Layer);
    }
    trigger_screenOff_timer(&ctrl_Layer);
}

#endif
