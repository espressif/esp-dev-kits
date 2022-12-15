idf_component_register(
    SRC_DIRS
        # LVGL Main SRC
        "lvgl/src/lv_core"
        "lvgl/src/lv_draw"
        "lvgl/src/lv_font"
        "lvgl/src/lv_gpu"
        "lvgl/src/lv_hal"
        "lvgl/src/lv_misc"
        "lvgl/src/lv_themes"
        "lvgl/src/lv_widgets"
        "lv_fonts"
    INCLUDE_DIRS
        # Config and defination at current path
        "."
        #LVGL main repo
        "lvgl"
        "lvgl/src/lv_core"
        "lvgl/src/lv_draw"
        "lvgl/src/lv_font"
        "lvgl/src/lv_gpu"
        "lvgl/src/lv_hal"
        "lvgl/src/lv_misc"
        "lvgl/src/lv_themes"
        "lvgl/src/lv_widgets"
 )
