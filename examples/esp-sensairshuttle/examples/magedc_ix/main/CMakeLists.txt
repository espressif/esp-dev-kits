# Magnetic-bead detection, low-power Wi-Fi (HTTP/SSE), and speaker gesture toggle
# ESP-IDF 6 provides cJSON through the Component Registry manifest.
set(json_dependency)
if(IDF_VERSION_MAJOR LESS 6)
    set(json_dependency json)
endif()

idf_component_register(
    SRCS
        "main.cpp"
        "zone_storage.c"
        "zone_tone.c"
        "zone_wizard.c"
        "zone_polar_detect.c"
        "speaker_gesture.c"
        "calib_nvs.c"
        "wifi_stream.c"
        "web_portal.c"
    EMBED_FILES
        "assets/zone1.wav"
        "assets/zone2.wav"
        "assets/zone3.wav"
        "assets/zone4.wav"
        "assets/zone6.wav"
        "assets/zone7.wav"
        "assets/zone8.wav"
        "assets/zone9.wav"
    INCLUDE_DIRS "."
    PRIV_REQUIRES bmm350 nvs_flash esp_driver_usb_serial_jtag esp_app_format espressif__bmi270_sensor
                  esp_wifi esp_netif esp_event esp_http_server esp_pm esp_driver_i2s mdns
                  led_strip espressif__esp_board_manager espressif__esp_boards ${json_dependency}
)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-missing-field-initializers)
