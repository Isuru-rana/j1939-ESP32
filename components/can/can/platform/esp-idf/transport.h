#pragma once

#include <driver/twai.h>

namespace embr::can::esp_idf {

namespace internal {

struct twai_transport
{
    // NOTE: Not attempting a homogeneous API on this one, just a
    // convenient way to pick up embr-defined config
    static void init();
};

}

}
