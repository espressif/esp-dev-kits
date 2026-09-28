set(PROJ_SRC ${CMAKE_CURRENT_LIST_DIR})

file(GLOB_RECURSE PROJ_SRCS_C ${PROJ_SRC}/*.c)
file(GLOB_RECURSE PROJ_SRCS_CPP ${PROJ_SRC}/*.cpp)
list(FILTER PROJ_SRCS_C EXCLUDE REGEX "/tests/")
list(FILTER PROJ_SRCS_CPP EXCLUDE REGEX "/tests/")

idf_component_register(
    SRCS  ${PROJ_SRCS_C} ${PROJ_SRCS_CPP}
    INCLUDE_DIRS
        ${PROJ_SRC}
    WHOLE_ARCHIVE
    REQUIRES espressif__esp-boost
    REQUIRES nvs_flash
    REQUIRES espressif__esp_board_manager
    REQUIRES bmm350
)

set_source_files_properties(
    ${PROJ_SRCS_CPP}
    PROPERTIES
        COMPILE_FLAGS "-Wno-missing-field-initializers"
)
