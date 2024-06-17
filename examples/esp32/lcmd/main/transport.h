#pragma once

#include <can/platform/esp-idf/transport.hpp>
#include <can/loopback.h>

// Non blocking
using transport_type = embr::can::esp_idf::twai_transport<false>;
using loopback_type = embr::can::impl::loopback_transport<3, transport_type::frame>;

