


idf_component_register(
    SRC_DIRS
        "src/boards"
        "src/peripherals"
        "src/storage"
        "src/indev"
        "src/codec"
    INCLUDE_DIRS
        "src/boards"
        "src"
        "include"
    REQUIRES
        esp_hid
        esp_lcd
        fatfs
        bus
        i2c_devices
        spiffs)
