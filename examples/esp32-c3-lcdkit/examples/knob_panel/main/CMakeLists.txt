idf_component_register(SRC_DIRS
                    "."
                    "./ir_nec"
                    "ui/fonts"
                    "ui/imgs"
                    "ui/imgs/image_language"
                    "ui/imgs/image_light"
                    "ui/imgs/image_standby"
                    "ui/imgs/image_wash"
                    "ui"
                    "ui/layer_manage"
                    INCLUDE_DIRS
                    "."
                    "./ir_nec"
                    "ui/layer_manage")

spiffs_create_partition_image(storage ../spiffs FLASH_IN_PROJECT)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-cast-function-type)
