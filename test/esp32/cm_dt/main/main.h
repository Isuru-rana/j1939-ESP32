#pragma once

#include <can/platform/esp-idf/transport.hpp>

#include "component_id.h"

// non blocking
using transport_type = embr::can::esp_idf::twai_transport<false>;
