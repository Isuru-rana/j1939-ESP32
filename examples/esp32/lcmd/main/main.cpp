#include <estd/chrono.h>

#include <j1939/state-machines/lcmd.hpp>

#include "transport.h"

using clock_type = estd::chrono::freertos_clock;
using time_point = clock_type::time_point;

static embr::j1939::sm::v0::lighting_command<time_point> lcmd;

// NOTE: Not ready yet

extern "C" void app_main(void)
{
    transport_type primary;
    loopback_type loopback;

    transport_type::frame frame;
    loopback_type::frame lframe;

    for(;;)
    {
        while(loopback.receive(&lframe))
        {

        }    
    }
}
