file(GLOB_RECURSE SRCS src/*.c)

idf_component_register(
    SRCS ${SRCS}
    INCLUDE_DIRS "include"
    PRIV_INCLUDE_DIRS "priv_include"
    REQUIRES driver esp_lcd esp_adc
    PRIV_REQUIRES esp_timer spiffs esp_psram
)
