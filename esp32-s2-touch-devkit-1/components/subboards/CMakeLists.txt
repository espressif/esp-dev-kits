idf_component_register(SRCS "button_board.c" "slider_board.c" "matrix_board.c" "touchpad_board.c" "proximity_board.c"
                        INCLUDE_DIRS "."
                        PRIV_REQUIRES touch_element buzzer digital_tube rgb_light)
