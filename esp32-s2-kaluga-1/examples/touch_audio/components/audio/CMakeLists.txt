set(COMPONENT_SRCS "audio.c")
set(COMPONENT_ADD_INCLUDEDIRS "include")

set(COMPONENT_REQUIRES es8311 board spiffs touch helix led_strip)

register_component()
