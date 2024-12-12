idf_component_register(
    SRC_DIRS
        "."
        "app"
        "icon"
        "qrgen"
        "ui"
    INCLUDE_DIRS
        "include"
        "qrgen"
        "app/include")

spiffs_create_partition_image(storage ../spiffs FLASH_IN_PROJECT)
