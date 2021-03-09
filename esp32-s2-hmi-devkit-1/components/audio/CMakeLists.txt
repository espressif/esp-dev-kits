idf_component_register(
    SRC_DIRS
        "." 
    INCLUDE_DIRS
        "include"
    REQUIRES
        file_system)

target_compile_options(${COMPONENT_LIB} PRIVATE
    -Wno-implicit-fallthrough)  # See audio_hal.c:102