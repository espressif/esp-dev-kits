set(APP_DIR app)
file(GLOB_RECURSE APP_SOURCES ${APP_DIR}/*.c)
set(LV_UI_DIR ui)
file(GLOB_RECURSE LV_UI_SOURCES ${LV_UI_DIR}/*.c)

idf_component_register(SRCS "main.c" ${LV_UI_SOURCES} ${APP_SOURCES}
                    INCLUDE_DIRS "." ${LV_UI_DIR} ${APP_DIR}
                    REQUIRES esp_driver_ppa nvs_flash)