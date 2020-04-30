set(decode_image_srcs "decode_image.c")

idf_component_register(SRCS "${decode_image_srcs}"
                       INCLUDE_DIRS "include"
                       PRIV_REQUIRES tjpgd)
