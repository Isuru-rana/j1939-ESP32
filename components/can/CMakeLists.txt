idf_component_register(SRCS 
                    "can/statics.cpp" "can/transport.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES nvs_flash embr j1939 can esp_helper)