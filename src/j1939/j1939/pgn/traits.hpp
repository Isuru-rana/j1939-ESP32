#pragma once

#include "traits.h"

// DEBT: Was fun while it lasted, but these guys belong in their respective .hpp implementations.
// Otherwise traits specializations appear for data_fields which aren't yet defined, which causes
// issues with things like 'decompose'

namespace embr { namespace j1939 { namespace pgn {


template <>
struct traits<pgns::fms_identity> : internal::traits_base
{
    static constexpr unsigned priority = 7;

    using spns = internal::spns_list<
        s::fms_sw_supported>;

    static constexpr const char* abbrev() { return "FMS"; }
};



template <>
struct traits<pgns::ecu_performance> : internal::traits_base
{
    using spns = internal::spns_list<
        s::keep_alive_battery_consumption,
        s::data_memory_usage>;

    static constexpr const char* name()
    {
        return "ECU Performance";
    }

    static constexpr const char* description()
    {
        return name();
    }

    static constexpr const char* abbrev()
    {
        return "EP";
    }
};


}}}
