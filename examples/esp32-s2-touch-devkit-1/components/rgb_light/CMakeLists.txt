idf_component_register(SRCS "rgb_light.c" "led_strip/led_strip_rmt_ws2812.c"
                        INCLUDE_DIRS "." "led_strip/include"
                        PRIV_REQUIRES "driver")
