set(COMPONENT_ADD_INCLUDEDIRS
    ./include
    )


register_component()

target_link_libraries(${COMPONENT_LIB} INTERFACE "-L ${CMAKE_CURRENT_LIST_DIR}")

if("${target}" STREQUAL "esp32")
target_link_libraries(${COMPONENT_LIB} INTERFACE speech_synthesis_esp32 
                                                    voice_set_xiaole_esp32 )

    # target_link_libraries(${COMPONENT_LIB} INTERFACE "-L${CMAKE_CURRENT_LIST_DIR}/controller/lib")
endif()

if("${target}" STREQUAL "esp32s2")
target_link_libraries(${COMPONENT_LIB} INTERFACE speech_synthesis_esp32s2 
                                                    voice_set_xiaole_esp32s2 )

    # target_link_libraries(${COMPONENT_LIB} INTERFACE "-L${CMAKE_CURRENT_LIST_DIR}/controller/lib")
endif()