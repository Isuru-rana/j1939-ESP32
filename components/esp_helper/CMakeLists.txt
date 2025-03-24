idf_component_register(SRCS 
                    "esp-helper.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES nvs_flash embr j1939 can esp_helper)