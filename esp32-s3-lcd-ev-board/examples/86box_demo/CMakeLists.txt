# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

set(EXTRA_COMPONENT_DIRS  ../../components)

#add_compile_options(-fdiagnostics-color=always -Wno-pointer-sign -Wno-return-type -Wno-parentheses -Wno-implicit-function-declaration -Wno-char-subscripts -Wno-unused-const-variable)
add_compile_options(-fdiagnostics-color=always -w)
project(86box_demo)
