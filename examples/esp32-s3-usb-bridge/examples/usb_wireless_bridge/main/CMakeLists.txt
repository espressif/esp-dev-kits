idf_component_register(SRC_DIRS "." "espnow" "src"
                       INCLUDE_DIRS "." "espnow" "include" "public_include"
                       REQUIRES "tinyusb" "driver" "esp_timer" "ws2812_led" "usb"
                       LDFRAGMENTS "noflash.lf")

idf_component_get_property(tusb_lib espressif__tinyusb COMPONENT_LIB)
target_include_directories(${tusb_lib} PRIVATE public_include)