set(pretty_effect_srcs "pretty_effect.c")

idf_component_register(SRCS "${pretty_effect_srcs}"
                       INCLUDE_DIRS "include"
                       PRIV_REQUIRES tjpgd decode_image)
