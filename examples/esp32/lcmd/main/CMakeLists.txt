idf_component_register(SRCS "lcmd_sink.cpp" "lcmd_source.cpp" "main.cpp"
    INCLUDE_DIRS "."
    REQUIRES nvs_flash embr j1939 can esp_helper)
