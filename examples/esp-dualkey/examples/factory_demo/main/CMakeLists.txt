# Define component directories
set(COMPONENT_DIRS
    ble_hid
    app_rainmaker
    usb_hid
    app_utils
    btn_progress
    app_nvs
    via_keymap
    light_manager
    dual_button
)

# Collect all C source files from component directories
set(COMPONENT_C_SOURCES)
foreach(DIR ${COMPONENT_DIRS})
    file(GLOB_RECURSE DIR_SOURCES ${DIR}/*.c)
    list(APPEND COMPONENT_C_SOURCES ${DIR_SOURCES})
endforeach()

# Register component with all sources and include directories
idf_component_register(
    SRCS "main.c" "esp_dualkey.c" ${COMPONENT_C_SOURCES}
    INCLUDE_DIRS "." ${COMPONENT_DIRS}
)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-unused-const-variable)

idf_component_get_property(tusb_lib espressif__tinyusb COMPONENT_LIB)

cmake_policy(SET CMP0079 NEW)
target_link_libraries(${tusb_lib} PRIVATE ${COMPONENT_LIB})
