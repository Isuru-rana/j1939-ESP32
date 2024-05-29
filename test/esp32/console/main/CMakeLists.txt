idf_component_register(SRCS "console.cpp" "main.cpp" "twai_init.cpp"
    INCLUDE_DIRS "."
    REQUIRES console embr esp_helper j1939 can)
