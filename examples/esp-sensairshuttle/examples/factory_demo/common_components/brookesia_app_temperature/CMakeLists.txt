set(PROJ_SRC ${CMAKE_CURRENT_LIST_DIR})

file(GLOB_RECURSE PROJ_SRCS_C ${PROJ_SRC}/*.c)
file(GLOB_RECURSE PROJ_SRCS_CPP ${PROJ_SRC}/*.cpp)

list(REMOVE_ITEM PROJ_SRCS_C
    ${PROJ_SRC}/ui/images/environment_bg.c
)
list(APPEND PROJ_SRCS_C
    ${PROJ_SRC}/ui/images/environment_bg.c
)

idf_component_register(
    SRCS  ${PROJ_SRCS_C} ${PROJ_SRCS_CPP}
    INCLUDE_DIRS
        ${PROJ_SRC}
        ${PROJ_SRC}/bsec_IAQ/inc
    WHOLE_ARCHIVE
    REQUIRES driver esp_common espressif__esp_board_manager bme690
)

set_source_files_properties(
    ${PROJ_SRCS_CPP}
    PROPERTIES
        COMPILE_FLAGS "-Wno-missing-field-initializers"
)

# Link BSEC library
target_link_libraries(${COMPONENT_LIB} PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/bsec_IAQ/bin/esp/esp32_risc_v/libalgobsec.a"
)

# Add linker flags for the library
target_link_options(${COMPONENT_LIB} PUBLIC
    "-Wl,--allow-multiple-definition"
)
