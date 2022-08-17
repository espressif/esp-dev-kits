//#include "../lv_examples.h"
#include "lv_demo.h"
#include "./lv_example_func.h"
#include "./lv_example_image.h"

#if LV_BUILD_EXAMPLES

#define LV_DEMO_MUSIC_HANDLE_SIZE  20

static bool show_set_layer_enter_cb(struct lv_layer_t * layer);
static bool show_set_layer_exit_cb(struct lv_layer_t * layer);
static void show_set_layer_timer_cb(lv_timer_t * tmr);

lv_layer_t show_set_layer ={
    .lv_obj_name    = "show_set_layer",
    .lv_obj_layer   = NULL,
    .enter_cb       = show_set_layer_enter_cb,
    .exit_cb        = show_set_layer_exit_cb,
    .timer_cb       = show_set_layer_timer_cb,
};

const warm_fan_info_list_t title_set_info[5] ={
    {&set_icon_temp_y,      &set_icon_temp_B,       "天气"},
    {&set_icon_screen_y,    &set_icon_screen_B,     "关闭屏幕"},
    {&set_icon_WIFI_y,      &set_icon_WIFI_B,       "WiFi"},
    {&set_icon_BLE_y,       &set_icon_BLE_B,        "蓝牙"},
    {&set_icon_set_y,       &set_icon_set_B,        "设置"},
    
};

static uint32_t period_clock_next;

static lv_obj_t * main_cont, * player;
static lv_obj_t * label_set_time;
#if 1

static void ctrlBtn_set_event_cb(lv_event_t * e)
{
    uint8_t i;
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * button = lv_event_get_target(e);

    if(code == LV_EVENT_SHORT_CLICKED) {
        //lv_obj_t * parent = lv_event_get_user_data(e);
        uint8_t btn = lv_event_get_user_data(e);
        reload_screenOff_timer();

        lv_obj_t * btnchild = lv_obj_get_child(button, 0);

        LV_LOG_USER("code %d,btn:%X,checked:%X,child_cnt:%d", code, btn,\
                (button->state&LV_STATE_CHECKED), lv_obj_get_child_cnt(button));
        
        if(LV_STATE_CHECKED == (button->state&LV_STATE_CHECKED)){
            lv_obj_clear_state(button, LV_STATE_CHECKED);
            lv_img_set_src(btnchild, title_set_info[btn].addr_open);
            LV_LOG_USER("clear checked:%X", button->state&LV_STATE_CHECKED);
        }
        else {
            lv_obj_add_state(button, LV_STATE_CHECKED);
            lv_img_set_src(btnchild, title_set_info[btn].addr_close);
            LV_LOG_USER("set checked:%X", button->state&LV_STATE_CHECKED);
        }
    }
}

static void slider_show_event_cb(lv_event_t * e)
{
   lv_obj_t * slider = lv_event_get_target(e);
   char buf[8];
   lv_snprintf(buf,sizeof(buf),"%d%d",(int)lv_slider_get_value(slider));
    reload_screenOff_timer();
   LV_LOG_USER("%s", buf);
   //lv_label_set_text(slider_label,buf);
   //lv_obj_align_to(slider_label,slider,LV_ALIGN_RIGHT_MID,0,10);
}

static void top_pulldown_event_cb(lv_event_t * e)
{
    uint8_t i;
    lv_obj_t * obj, *nowObj;
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_SCROLL_END) {
        lv_point_t scroll_end;
        lv_obj_get_scroll_end(main_cont, &scroll_end);
        LV_LOG_USER("scroll_end,[%d,%d], [%d,%d]", scroll_end.x, scroll_end.y,\
                lv_obj_get_scroll_x(main_cont), lv_obj_get_scroll_y(main_cont));

        if((480 == lv_obj_get_scroll_y(main_cont))){
            LV_LOG_USER("top");
            lv_obj_set_style_opa(player, LV_OPA_0, 0);
            lv_obj_set_style_bg_color(player, lv_color_hex(0x000000), 0);
        }
        else if(0 == lv_obj_get_scroll_y(main_cont)){
            LV_LOG_USER("bottom");
        }
    }
    else if(code == LV_EVENT_SCROLL_BEGIN) {
        LV_LOG_USER("LV_EVENT_SCROLL_BEGIN, [%d:%d]", lv_obj_get_scroll_x(main_cont),lv_obj_get_scroll_y(main_cont));
        lv_obj_set_style_opa(player, LV_OPA_0, 0);
        lv_obj_set_style_bg_color(player, lv_color_hex(0x000000), 0);
    }
    else if(code == LV_EVENT_SCROLL){
        reload_screenOff_timer();
        //LV_LOG_USER("LV_EVENT_SCROLL, [%d:%d]", lv_obj_get_x(main_cont),lv_obj_get_y(main_cont));
    }
}

static lv_obj_t * lv_example_slider_backLight(lv_obj_t * parent)
{
    /*Create a transition*/
    static const lv_style_prop_t props[] = {LV_STYLE_BG_COLOR, 0};
    static lv_style_transition_dsc_t transition_dsc;
    lv_style_transition_dsc_init(&transition_dsc, props, lv_anim_path_linear, 300, 0, NULL);

    static lv_style_t style_main;
    static lv_style_t style_indicator;
    static lv_style_t style_pressed_color;

    lv_style_init(&style_main);
    lv_style_set_bg_opa(&style_main, LV_OPA_COVER);
    lv_style_set_bg_color(&style_main, lv_color_hex(0xFFFFFF));
    lv_style_set_radius(&style_main, LV_RADIUS_CIRCLE);
    lv_style_set_pad_ver(&style_main, -2); /*Makes the indicator larger*/

    lv_style_init(&style_indicator);//click
    lv_style_set_bg_opa(&style_indicator, LV_OPA_COVER);
    lv_style_set_bg_color(&style_indicator, lv_color_hex(COLOUR_YELLOW));
    lv_style_set_radius(&style_indicator, LV_RADIUS_CIRCLE);
    lv_style_set_transition(&style_indicator, &transition_dsc);

    lv_style_init(&style_pressed_color);//slider
    lv_style_set_bg_color(&style_pressed_color, lv_color_hex(COLOUR_YELLOW));

    /*Create a slider and add the style*/
    lv_obj_t * slider = lv_slider_create(parent);
    lv_obj_remove_style_all(slider);        /*Remove the styles coming from the theme*/
    lv_obj_set_size(slider, lv_pct(90), 20);

    lv_obj_add_style(slider, &style_main, LV_PART_MAIN);
    lv_obj_add_style(slider, &style_indicator, LV_PART_INDICATOR);//click
    lv_obj_add_style(slider, &style_pressed_color, LV_PART_INDICATOR | LV_STATE_PRESSED);
    lv_obj_add_style(slider, &style_pressed_color, LV_PART_KNOB | LV_STATE_PRESSED);

    lv_obj_add_event_cb(slider,slider_show_event_cb,LV_EVENT_VALUE_CHANGED,NULL); //设置回调显示
    lv_slider_set_value(slider,50,LV_ANIM_OFF);
    //lv_slider_set_range(slider,5,100);

    return slider;
}

#if 0
static void top_set_list_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_CLICKED) {
        
        lv_obj_t * parent = lv_obj_get_parent(obj);
        uint32_t i;
        for(i = 0; i < lv_obj_get_child_cnt(parent); i++) {
            lv_obj_t * child = lv_obj_get_child(parent, i);
            if(child == obj) {
                lv_obj_add_state(child, LV_STATE_CHECKED);
                LV_LOG_USER("child:%X,%s set checked", child, lv_list_get_btn_text(parent, child));
            }
            else {
                lv_obj_clear_state(child, LV_STATE_CHECKED);
                //LV_LOG_USER("child:%X,%s clear checked", child, lv_list_get_btn_text(parent, child));
            }
        }
    }
}
void lv_create_set_pulldown(lv_obj_t * parent)
{
    uint8_t i = 0;
    lv_color_t color;
    lv_obj_t * setpage, *setlist;
    lv_obj_t * btn, *setDetail;
    setpage =  lv_tileview_add_tile(parent, 0, 0, LV_DIR_BOTTOM);
    lv_obj_set_style_bg_color(setlist, lv_color_hex(0x1F1F1F), LV_PART_MAIN);
    //lv_obj_add_style(setpage, &style_btn_black, LV_STATE_DEFAULT);
    tile_bottomSet = setpage;
    /*
     *setlist init
     */
    setlist = lv_list_create(setpage);
    lv_obj_set_size(setlist, LV_PCT(30), LV_PCT(100));
    
    lv_obj_set_style_bg_color(setlist, lv_color_hex(0x3F3F3F), LV_PART_MAIN);
    lv_obj_set_style_border_width(setlist, 0, 0);
    lv_obj_set_style_radius(setlist, 0, 0);

    setDetail = lv_list_create(setpage);
    lv_obj_set_size(setDetail, LV_PCT(60), LV_PCT(90));
    lv_obj_align(setDetail, LV_ALIGN_RIGHT_MID, -24, 0);
    
    lv_obj_set_style_bg_color(setDetail, lv_color_hex(0x3F3F3F), LV_PART_MAIN);
    lv_obj_set_style_border_width(setDetail, 0, 0);
    lv_obj_set_style_radius(setDetail, 0, 10);

    for(i = 0; i< 5; i++){
        switch(i)
        {
            case 0:
            btn = lv_list_add_btn(setlist, NULL, "设置");
            break;
            case 1:
            btn = lv_list_add_btn(setlist, NULL, "账号中心");
            break;
            case 2:
            btn = lv_list_add_btn(setlist, NULL, "本机信息");
            break;
            case 3:
            btn = lv_list_add_btn(setlist, NULL, "无线网络");
            break;
            /*
            case 4:
            btn = lv_list_add_btn(setlist, NULL, "蓝牙连接");
            break;
            */
            case 4:
            btn = lv_list_add_btn(setlist, NULL, "屏幕设置");
            break;
        }
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x3F3F3F), LV_PART_MAIN);
        lv_obj_set_size(btn, LV_PCT(100), (0 == i)? LV_PCT(20):LV_PCT(16));
        lv_obj_set_style_text_color(btn, lv_color_white(), 0);
        lv_obj_set_style_text_font(btn, &font_lanting16, 0);
        lv_obj_add_event_cb(btn, top_set_list_event_cb, LV_EVENT_CLICKED, NULL);

        //lv_obj_align(btn->, LV_ALIGN_BOTTOM_MID, 0, 0);
        //LV_LOG_USER("btn:%d, setlist:%d", lv_obj_get_child_cnt(btn), lv_obj_get_child_cnt(setlist));
        lv_obj_t * btnchild = lv_obj_get_child(btn, 0);
        //lv_obj_align(btnchild, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
        lv_obj_set_pos(btnchild, 100, 20);
        //print_position(btnchild, "btnchild");
    }
}
#endif
static void lv_create_set_pulldown(lv_obj_t * parent)
{
    int i;
    uint8_t timeBuf[30];

    static lv_coord_t grid_2_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    //static lv_coord_t grid_2_col_dsc[] = {150, 150, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t grid_2_row_dsc[] = {
                50,  /*power key*/
                50,  /*current temp*/
                50,  /*Title "current temp*/
                70,  /*Title "current temp*/
                LV_GRID_TEMPLATE_LAST
        };
    
    lv_obj_t * setpage, *bg_setpage, *set_btn, * bg_set_btn, * label_set;
    //setpage =  lv_tileview_add_tile(parent, 0, 0, LV_DIR_BOTTOM);
    setpage = parent;
    //tile_bottomSet = setpage;

    bg_setpage = lv_list_create(setpage);
    lv_obj_set_style_opa(bg_setpage, LV_OPA_100, 0);
    lv_obj_set_size(bg_setpage, LV_PCT(100), LV_PCT(60));
    lv_obj_align(bg_setpage, LV_ALIGN_TOP_LEFT, 0, 15);
    
    lv_obj_set_style_bg_color(bg_setpage, lv_color_hex(COLOUR_BG1_GREY), LV_PART_MAIN);
    lv_obj_set_style_border_width(bg_setpage, 0, 0);
    lv_obj_set_style_radius(bg_setpage, 0, 10);

    lv_obj_set_grid_dsc_array(bg_setpage, grid_2_col_dsc, grid_2_row_dsc);

    for(i = 0; i< sizeof(title_set_info)/sizeof(title_set_info[0]); i++){
        set_btn = lv_btn_create(bg_setpage);
        lv_obj_remove_style_all(set_btn);
        lv_obj_set_size(set_btn, 70, 70);
        lv_obj_add_event_cb(set_btn, ctrlBtn_set_event_cb, LV_EVENT_SHORT_CLICKED, i);
        lv_obj_clear_state(set_btn, LV_STATE_CHECKED);
        lv_obj_set_grid_cell(set_btn, LV_GRID_ALIGN_CENTER, i, 1, LV_GRID_ALIGN_CENTER, 1, 1);

        bg_set_btn = lv_img_create(set_btn);
        lv_img_set_src(bg_set_btn, title_set_info[i].addr_open);
        lv_obj_align(bg_set_btn, LV_ALIGN_CENTER, 0, 0);

        label_set = lv_label_create(bg_setpage);
        lv_obj_remove_style_all(label_set);
        lv_obj_set_style_text_color(label_set, lv_color_white(), 0);
        lv_obj_set_style_text_font(label_set, &font_lanting16, 0);

        lv_label_set_text(label_set, title_set_info[i].btn_name);
        lv_obj_set_grid_cell(label_set, LV_GRID_ALIGN_CENTER, i, 1, LV_GRID_ALIGN_CENTER, 2, 1);
    }

    label_set_time = lv_label_create(bg_setpage);
    lv_obj_remove_style_all(label_set_time);
    lv_obj_set_style_text_color(label_set_time, lv_color_white(), 0);
    lv_obj_set_style_text_font(label_set_time, &font_num36, 0);

    uint32_t timenow = sys_clock_getSecond();
    sprintf(timeBuf, "%02d:%02d", (timenow/60)%24,timenow%60);
    lv_label_set_text(label_set_time, timeBuf);
    lv_obj_center(label_set_time);
    //lv_obj_align(label_home_time, LV_ALIGN_CENTER, 50, 0);
    lv_obj_set_grid_cell(label_set_time, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    print_position(label_set_time, "label_home_time");

    lv_obj_t * labelData = lv_label_create(bg_setpage);
    lv_obj_remove_style_all(labelData);
    lv_obj_set_style_text_color(labelData, lv_color_white(), 0);
    lv_obj_set_style_text_font(labelData, &font_num24, 0);

    lv_label_set_text(labelData, "07.25");
    lv_obj_center(labelData);
    //lv_obj_align(labelData, LV_ALIGN_CENTER, 50, 0);
    lv_obj_set_grid_cell(labelData, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    print_position(labelData, "labelData");

    lv_obj_t * slider = lv_example_slider_backLight(bg_setpage);
    lv_obj_set_grid_cell(slider, LV_GRID_ALIGN_CENTER, 0, 5, LV_GRID_ALIGN_CENTER, 3, 1);
}

lv_obj_t * lv_create_pulldown_bg(lv_obj_t * parent)
{
    /*A transparent container in which the player section will be scrolled*/
    main_cont = lv_obj_create(parent);
    lv_obj_clear_flag(main_cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(main_cont, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_remove_style_all(main_cont);                            /*Make it transparent*/
    lv_obj_set_size(main_cont, lv_pct(100), lv_pct(100));
    lv_obj_set_scroll_snap_y(main_cont, LV_SCROLL_SNAP_CENTER);    /*Snap the children to the center*/

    /*Create a container for the player*/
    player = lv_obj_create(main_cont);
    lv_obj_set_size(player, LV_HOR_RES, LV_VER_RES + LV_DEMO_MUSIC_HANDLE_SIZE * 2);
    lv_obj_set_y(player, - LV_DEMO_MUSIC_HANDLE_SIZE);
    LV_LOG_USER("player, %d:%d, y:%d",LV_HOR_RES, LV_VER_RES + LV_DEMO_MUSIC_HANDLE_SIZE * 2, - LV_DEMO_MUSIC_HANDLE_SIZE);
    lv_obj_set_style_opa(player, LV_OPA_0, 0);
    lv_obj_set_style_bg_color(player, lv_color_hex(0x000000), 0);
    lv_obj_set_style_border_width(player, 0, 0);
    lv_obj_set_style_pad_all(player, 0, 0);
    lv_obj_set_scroll_dir(player, LV_DIR_VER);

    /* A transparent placeholder below the player container
     * It is used only to snap it to center.*/
    lv_obj_t * placeholder = lv_obj_create(main_cont);
    lv_obj_remove_style_all(placeholder);
    lv_obj_set_size(placeholder, lv_pct(100), LV_VER_RES -2 *  LV_DEMO_MUSIC_HANDLE_SIZE);
    lv_obj_set_y(placeholder, LV_VER_RES + LV_DEMO_MUSIC_HANDLE_SIZE);
    lv_obj_clear_flag(placeholder, LV_OBJ_FLAG_CLICKABLE);
    LV_LOG_USER("placeholder, %d:%d, y:%d",LV_HOR_RES, LV_VER_RES -2 *  LV_DEMO_MUSIC_HANDLE_SIZE, LV_VER_RES + LV_DEMO_MUSIC_HANDLE_SIZE);

    lv_obj_update_layout(main_cont);
    lv_obj_add_event_cb(main_cont, top_pulldown_event_cb, LV_EVENT_ALL, 0);

    lv_create_set_pulldown(player);

    //lv_obj_scroll_to_y(main_cont, (LV_VER_RES - 1 *  LV_DEMO_MUSIC_HANDLE_SIZE), LV_ANIM_OFF);
    lv_obj_scroll_by(main_cont, 0, 0-(LV_VER_RES - 0 *  LV_DEMO_MUSIC_HANDLE_SIZE), LV_ANIM_OFF);
    return player;
}
#endif

static bool show_set_layer_enter_cb(struct lv_layer_t * layer)
{
	bool ret = false;
    LV_LOG_USER("Enter:%s", layer->lv_obj_name);
	if(NULL == layer->lv_obj_layer){
		ret = true;

        layer->lv_obj_layer = lv_obj_create(lv_scr_act());
        lv_obj_remove_style_all(layer->lv_obj_layer);
        lv_obj_set_size(layer->lv_obj_layer, 1, 1);

        lv_create_pulldown_bg(layer->lv_obj_user);
        period_clock_next = lv_tick_get()+2000;
	}

	return ret;
}

static bool show_set_layer_exit_cb(struct lv_layer_t * layer)
{
    LV_LOG_USER("");
}

static void show_set_layer_timer_cb(lv_timer_t * tmr)
{
    int32_t isTmOut;
    uint32_t temp;
    uint8_t timeBuf[30];

    isTmOut = (lv_tick_get() - (period_clock_next + 1000));
    if(isTmOut > 0){
        period_clock_next = lv_tick_get();
        temp = sys_clock_getSecond();
        sprintf(timeBuf, "%02d:%02d", (temp/60)%24,temp%60);
        if(label_set_time){
            lv_label_set_text(label_set_time, timeBuf);
        }
    }
}

#endif
