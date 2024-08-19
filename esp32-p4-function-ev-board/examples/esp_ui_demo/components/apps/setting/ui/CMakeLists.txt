SET(SOURCES screens/ui_ScreenSettingMain.c
    screens/ui_ScreenSettingWiFi.c
    screens/ui_ScreenSettingBLE.c
    screens/ui_ScreenSettingVolume.c
    screens/ui_ScreenSettingLight.c
    screens/ui_ScreenSettingAbout.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
    images/ui_img_wifi_png.c
    images/ui_img_arrow_png.c
    images/ui_img_bluetooth_png.c
    images/ui_img_sound_png.c
    images/ui_img_light_png.c
    images/ui_img_about_png.c
    images/ui_img_return_png.c
    images/ui_img_482951309.c
    images/ui_img_sunny_2_png.c
    fonts/ui_font_FontPuHui96.c
    fonts/ui_font_FontPuhui20.c
    fonts/ui_font_FontPuhui30.c)

add_library(ui ${SOURCES})
