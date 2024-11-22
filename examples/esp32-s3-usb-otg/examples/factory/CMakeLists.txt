# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

set(PROJECT_VER "0.1.0.0")
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
set(EXTRA_COMPONENT_DIRS ${EXTRA_COMPONENT_DIRS} $ENV{IDF_PATH}/examples/common_components/qrcode "../../components")
project(usb_otg_dev)

add_custom_target(flash_test)

add_custom_command(
    TARGET flash_test POST_BUILD
    COMMAND esptool.py --chip esp32s3 -p /dev/ttyUSB0 -b 3000000 --before=default_reset --after=hard_reset write_flash --flash_mode dio --flash_freq 80m --flash_size 8MB 0x210000 ${PROJECT_DIR}/bin/test_project.bin
)