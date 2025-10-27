idf_component_register(SRCS "app_insights.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES esp_insights esp_diagnostics esp_rainmaker)
