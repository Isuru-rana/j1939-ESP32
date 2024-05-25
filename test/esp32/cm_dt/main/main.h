#pragma once

#include <can/platform/esp-idf/transport.hpp>

// blocking
using transport_type = embr::can::esp_idf::twai_transport<true>;
