idf_component_register(
    SRCS
        main.c
        lv_lib_freetype/lv_freetype.c
    INCLUDE_DIRS
        lv_lib_freetype
        include)

add_prebuilt_library(prebuilt "lib/libfreetype.a" REQUIRES esp_rom)

target_link_libraries(${COMPONENT_LIB} PRIVATE prebuilt)

spiffs_create_partition_image(storage ../spiffs FLASH_IN_PROJECT)
