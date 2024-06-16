#pragma once

#include <can/platform/esp-idf/transport.hpp>
#include <can/loopback.h>

using loopback_type = embr::can::loopback_transport;
using transport_type = embr::can::esp_idf::twai_transport<true>;

