idf_component_register(
    SRC_DIRS "." "./app/usb_camera" "./app/app_menu" "./app/usb_wireless_disk" "./app/app_manual"
    "./app/app_hid_device"
    INCLUDE_DIRS "." "./app"
    EMBED_FILES "./app/usb_wireless_disk/favicon.ico" "./app/usb_wireless_disk/upload_script.html"
)

spiffs_create_partition_image(storage ../spiffs_image FLASH_IN_PROJECT)

target_compile_options(${COMPONENT_LIB} PRIVATE 
                                        -Wno-unused-variable
                                        -Wno-unused-function
                                        -Wno-incompatible-pointer-types)
