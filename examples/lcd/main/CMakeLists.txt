set(srcs "main.c")

idf_component_register(SRCS ${srcs}
                    EMBED_FILES image.jpg)

