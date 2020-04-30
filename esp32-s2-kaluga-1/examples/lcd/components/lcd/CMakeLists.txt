set(lcd_srcs "lcd.c")

idf_component_register(SRCS "${lcd_srcs}"
                       INCLUDE_DIRS "include"
                      PRIV_REQUIRES pretty_effect tjpgd decode_image)
