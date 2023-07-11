set(COMPONENT_SRCS "app_main.c" "app_wifi.c"  "app_httpd.c" "chinese_tts.c" "decode_url.c")
set(COMPONENT_ADD_INCLUDEDIRS "include")

set(COMPONENT_REQUIRES
    nvs_flash
    esp_http_server
    es8311
    esp_tts
    )

set(COMPONENT_EMBED_FILES
	"www/SaveResponseSucc.html.gz"
	"www/SaveResponseError.html.gz"
    "www/settingPage.html.gz")


register_component()

target_compile_options(${COMPONENT_LIB} PRIVATE
                                        -Wno-discarded-qualifiers
                                        -Wno-unused-variable
                                        -Wno-incompatible-pointer-types)