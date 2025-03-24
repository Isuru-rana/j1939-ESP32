idf_component_register(SRCS 
                    "embr/internal/general.cpp" "embr/platform/esp-idf/debounce.cpp" "embr/platform/esp-idf/timer-scheduler.cpp" "embr/service/service_v1.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES nvs_flash embr j1939 can esp_helper)