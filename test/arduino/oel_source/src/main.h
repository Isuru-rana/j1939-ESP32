#pragma once

#include <j1939/cas/internal/prng_address_manager.h>
#include <j1939/state-machines/network.hpp>
#include <j1939/NAME/name.h>

using clock = estd::chrono::arduino_clock;

namespace app {

using namespace embr;
using namespace j1939;

// DEBT: Copy/pasted this from joystick example, change it up a bit
using proto_name = j1939::layer0::NAME<true,
    industry_groups::construction,
    vehicle_systems::non_specific,
    function_fields::joystick_control>;

using address_manager = j1939::internal::prng_address_manager;

using network = j1939::sm::v1::network<
    address_manager,
    clock::time_point>;

}