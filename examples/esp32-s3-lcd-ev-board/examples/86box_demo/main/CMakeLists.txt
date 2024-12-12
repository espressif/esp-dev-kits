file(GLOB_RECURSE LV_DEMOS_SOURCES ./*.c)

idf_component_register(
    SRC_DIRS
        "."
        "gui"
        "gui/font"
        "gui/assert"
        "gui_music"
        "gui_music/assets"
        "app"

    INCLUDE_DIRS
        "."
        "gui"
        "gui_music"
        "app")

set_source_files_properties(
    ${LV_DEMOS_SOURCES}
    PROPERTIES COMPILE_FLAGS
    "-DLV_LVGL_H_INCLUDE_SIMPLE -DLV_USE_DEMO_MUSIC")

spiffs_create_partition_image(storage ../spiffs FLASH_IN_PROJECT)

