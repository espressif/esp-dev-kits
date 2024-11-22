/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#ifndef LV_EXAMPLE_IMAGE_H
#define LV_EXAMPLE_IMAGE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

LV_IMG_DECLARE(set_icon_BLE_B);
LV_IMG_DECLARE(set_icon_BLE_y);
LV_IMG_DECLARE(set_icon_WIFI_B);
LV_IMG_DECLARE(set_icon_WIFI_y);
LV_IMG_DECLARE(set_icon_screen_B);
LV_IMG_DECLARE(set_icon_screen_y);
LV_IMG_DECLARE(set_icon_set_B);
LV_IMG_DECLARE(set_icon_set_y);

LV_IMG_DECLARE(watch_bg);
LV_IMG_DECLARE(hour);
LV_IMG_DECLARE(minute);
LV_IMG_DECLARE(second);

LV_IMG_DECLARE(new_watch_bg);
LV_IMG_DECLARE(watch_hour);
LV_IMG_DECLARE(watch_min);
LV_IMG_DECLARE(watch_sec);

LV_IMG_DECLARE(mic_logo);

LV_IMG_DECLARE(icon_160_light);
LV_IMG_DECLARE(icon_160_home);
LV_IMG_DECLARE(icon_160_theam);

LV_IMG_DECLARE(icon_320_AC);
LV_IMG_DECLARE(icon_320_weather);

LV_IMG_DECLARE(icon_child_ac_small);
LV_IMG_DECLARE(icon_child_weather);
LV_IMG_DECLARE(icon_child_ac);
LV_IMG_DECLARE(icon_child_ac_bg);
LV_IMG_DECLARE(icon_child_ac_add1);
LV_IMG_DECLARE(icon_child_ac_add2);
LV_IMG_DECLARE(icon_child_ac_dec1);
LV_IMG_DECLARE(icon_child_ac_dec2);
LV_IMG_DECLARE(icon_child_ac_pow1);
LV_IMG_DECLARE(icon_child_ac_pow2);
LV_IMG_DECLARE(icon_child_home);
LV_IMG_DECLARE(icon_child_theam);
LV_IMG_DECLARE(icon_child_light);

LV_IMG_DECLARE(icon_child_ac_add1_warm);
LV_IMG_DECLARE(icon_child_ac_dec2_warm);
LV_IMG_DECLARE(icon_child_ac_warm);
LV_IMG_DECLARE(icon_child_ac_add2_warm);
LV_IMG_DECLARE(icon_child_ac_pow1_warm);
LV_IMG_DECLARE(icon_child_weather_warm);
LV_IMG_DECLARE(icon_child_ac_bg_warm);
LV_IMG_DECLARE(icon_child_ac_pow2_warm);
LV_IMG_DECLARE(icon_child_ac_dec1_warm);
LV_IMG_DECLARE(icon_child_ac_small);

LV_IMG_DECLARE(icon_light_change);
LV_IMG_DECLARE(icon_light_color);
LV_IMG_DECLARE(icon_light_light);
LV_IMG_DECLARE(icon_light_normal);
LV_IMG_DECLARE(icon_light_shaw);

LV_IMG_DECLARE(icon_light_color_warm);
LV_IMG_DECLARE(icon_light_light_warm);
LV_IMG_DECLARE(icon_light_normal_warm);

LV_IMG_DECLARE(icon_display);
LV_IMG_DECLARE(icon_voice);
LV_IMG_DECLARE(hand_down);

/********************************
 * font
********************************/
LV_FONT_DECLARE(helveticaneue_32);
LV_FONT_DECLARE(helveticaneue_36);
LV_FONT_DECLARE(Montserrat_Bold_116);

/*
 0x20-0x7F
关闭屏幕蓝牙设置声音亮度与显示升级关于待机时间秒分钟小时永久生产厂家主板名称子板名称设备版本号语音版本语言
请输入密码已未连接静音亮度调节屏幕首页当前版本已是最新版本设备名称设备型号序列号地址灯上海北京深圳屏幕中文
是否确认恢复出厂设置取消确认打开电空高温降低乐鑫开启离线语音助手连接网络本出厂固件提供了两个演示: 下一步
精美的UI 和语音控制，请选择进入您想体验的演示倾听中默认命令词语音助手操作步骤请说唤醒设备等待屏幕显示请说，说出命令例如
 */
LV_FONT_DECLARE(SourceHanSansCN_Normal_20);

/*
主题照明主页空调天气打开关闭空调电灯升高降低温度请说上海北京深圳
*/
LV_FONT_DECLARE(SourceHanSansCN_Normal_18);

/*
空调
*/
LV_FONT_DECLARE(SourceHanSansCN_Normal_26);
/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_EXAMPLE_IMAGE_H*/
