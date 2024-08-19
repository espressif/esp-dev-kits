idf_component_register(
    SRCS main.cpp
    INCLUDE_DIRS .)

idf_component_get_property(LVGL_LIB lvgl__lvgl COMPONENT_LIB)
target_compile_options(
    ${LVGL_LIB} 
    PRIVATE 
        -DLV_LVGL_H_INCLUDE_SIMPLE
        -DLV_USE_DEMO_MUSIC
)

if(CONFIG_ESP_UI_MEMORY_USE_CUSTOM)
    idf_component_get_property(ESP32_UI_LIB espressif__esp-ui COMPONENT_LIB)
    target_compile_options(${ESP32_UI_LIB} PUBLIC "-DESP_UI_MEMORY_CUSTOM_MALLOC(x)=heap_caps_aligned_alloc(128, x, MALLOC_CAP_SPIRAM)" "-DESP_UI_MEMORY_CUSTOM_FREE(x)=free(x)")
endif()

if(CONFIG_LV_MEM_CUSTOM)
    target_compile_options(
        ${LVGL_LIB} 
        PRIVATE 
            "-DLV_MEM_CUSTOM_ALLOC(x)=heap_caps_malloc(x, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT)"
    )
endif()

spiffs_create_partition_image(storage ../spiffs FLASH_IN_PROJECT)