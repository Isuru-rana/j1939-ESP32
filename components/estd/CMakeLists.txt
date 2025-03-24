idf_component_register(SRCS 
                    "estd/ext/willemt/bipbuffer/bipbuffer.c" "estd/internal/string_convert.cpp" "estd/internal/utility.cpp" ""
                    INCLUDE_DIRS "."
                    REQUIRES nvs_flash embr j1939 can esp_helper)