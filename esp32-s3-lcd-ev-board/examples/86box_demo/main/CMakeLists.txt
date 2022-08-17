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
        "app")
spiffs_create_partition_image(storage ../spiffs FLASH_IN_PROJECT)

