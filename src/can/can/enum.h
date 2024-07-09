#pragma once

namespace embr { namespace can {

inline namespace v1 {

// NOTE: Consider melding all this with service code
enum class bus_state
{
    uninitialized,
    offline,
    online,
    bus_off,
    recovering,
    unknown             // other unspecified error
};

enum class status
{
    ok,
    timeout,
    fail,
    bad_driver,
    not_supported
};

// Used during frame_traits::create, otherwise favor granular mutators
enum frame_flags : unsigned
{
    FRAME_RTR = 0x01,
    FRAME_EXT = 0x02,
    FRAME_SS = 0x04
};

}

}}