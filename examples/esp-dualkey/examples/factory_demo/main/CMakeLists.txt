# Define component directories
set(COMPONENT_DIRS
    app_nvs
    app_rainmaker
    app_utils
    ble_hid
    btn_progress
    dual_button
    light_manager
    usb_hid
    via_keymap
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
