idf_component_register(
    SRCS "src/audio_malloc.c"
    INCLUDE_DIRS "include"
)

add_prebuilt_library(esp_codec "${CMAKE_CURRENT_SOURCE_DIR}/libs/${CONFIG_IDF_TARGET}/libesp_codec.a" REQUIRES esp_jpeg)
target_link_libraries(${COMPONENT_LIB}  "-Wl,--start-group" esp_codec "-Wl,--end-group")
