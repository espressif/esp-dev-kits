idf_component_register(
    SRCS "esp32_p4_eye.c" "esp32_p4_eye_init.c"
    INCLUDE_DIRS "include"
    PRIV_INCLUDE_DIRS "priv_include"
    REQUIRES driver spiffs knob
    PRIV_REQUIRES fatfs esp_lcd
)

idf_component_set_property(${COMPONENT_NAME} WHOLE_ARCHIVE TRUE)