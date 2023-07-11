set(COMPONENT_ADD_INCLUDEDIRS "include")
set(COMPONENT_SRCDIRS "src")

register_component()
add_definitions(-DARM)

target_compile_options(${COMPONENT_LIB} PRIVATE
                                        -Wno-unused-but-set-variable)
