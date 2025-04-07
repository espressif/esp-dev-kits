SET(SOURCES screens/ui_ScreenCamera.c
    screens/ui_ScreenAlbum.c
    screens/ui_ScreenUSB.c
    screens/ui_ScreenPicSave.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
    ui_events.c
    images/ui_img_button_select_png.c
    images/ui_img_up_button_png.c
    images/ui_img_button_down_png.c
    images/ui_img_sdcard_no_png.c
    images/ui_img_sdcard_yes_png.c
    images/ui_img_button_menu_png.c
    images/ui_img_camera_small_png.c
    images/ui_img_interval_small_png.c
    images/ui_img_video_small_png.c
    images/ui_img_reddot_recording_png.c
    images/ui_img_usb_page1_png.c
    images/ui_img_usb_page2_png.c
    images/ui_img_usb_page_warning_png.c
    images/ui_img_album_big_png.c
    images/ui_img_camera_big_png.c
    images/ui_img_interval_big_png.c
    images/ui_img_video_big_png.c
    images/ui_img_usb_big_png.c
    images/ui_img_settings_big_png.c
    fonts/ui_font_FontKoHoBold14.c
    fonts/ui_font_FontKoHoBold18.c
    fonts/ui_font_FontKoHoBold28.c
    fonts/ui_font_FontKoHoMiniBold14.c
    fonts/ui_font_FontKoHoMiniBold18.c
    fonts/ui_font_FontKoHoMiniBold22.c
    fonts/ui_font_FontKoHoMiniBold24.c)

add_library(ui ${SOURCES})
