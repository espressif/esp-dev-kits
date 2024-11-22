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
        # LVGL porting
        "lv_port"
        # LVGL Examples
        "lv_examples/src/assets"
        "lv_examples/src/lv_demo_benchmark"
        "lv_examples/src/lv_demo_music"
        "lv_examples/src/lv_demo_music/assets"
        "lv_examples/src/lv_demo_printer"
        "lv_examples/src/lv_demo_printer/images"
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
        # LVGL porting
        "lv_port/include"
        #LVGL examples
        "lv_examples"
        "lv_examples/src/assets"
        "lv_examples/src/lv_demo_music"
        "lv_examples/src/lv_demo_music/assets"
        "lv_examples/src/lv_demo_benchmark"
        "lv_examples/src/lv_demo_printer"
    REQUIRES
        bsp
        i2c_devices)

# LVGL might have warnings. Just ignore them
target_compile_options(${COMPONENT_LIB} PRIVATE
    -w)
