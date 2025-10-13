set(priv_req qrcode nvs_flash esp_event rmaker_common vfs wifi_provisioning)

if ("${IDF_VERSION_MAJOR}.${IDF_VERSION_MINOR}" VERSION_GREATER_EQUAL "5.1")
    list(APPEND priv_req network_provisioning openthread)
endif()

idf_component_register(SRCS "app_wifi_internal.c" "app_network.c" "app_thread_internal.c"
                    INCLUDE_DIRS "."
                    PRIV_INCLUDE_DIRS "private_include"
                    PRIV_REQUIRES ${priv_req})

if(CONFIG_APP_WIFI_SHOW_DEMO_INTRO_TEXT)
    target_compile_definitions(${COMPONENT_TARGET} PRIVATE "-D RMAKER_DEMO_PROJECT_NAME=\"${CMAKE_PROJECT_NAME}\"")
endif()
