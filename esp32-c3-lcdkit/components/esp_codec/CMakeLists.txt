idf_component_register(
    INCLUDE_DIRS
        "./include/codec"
        "./include/processing"
)

message("#### ${CMAKE_CURRENT_LIST_DIR}/lib/${CONFIG_IDF_TARGET}/libesp_processing.a")
target_link_libraries(${COMPONENT_LIB} INTERFACE "${CMAKE_CURRENT_LIST_DIR}/lib/${CONFIG_IDF_TARGET}/libesp_processing.a")