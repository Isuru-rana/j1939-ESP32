#pragma once

#include "base.hpp"

#include "../slots/ascii.hpp"

namespace embr { namespace j1939 {

namespace spn {

template <>
struct type_traits<spns::driver1_identification> :
    internal::ascii_type_traits<1728> {};

template <>
struct type_traits<spns::driver2_identification> :
    internal::ascii_type_traits<1728> {};

}

namespace pgn {

template <>
struct traits<pgns::driver_identificationn> : internal::traits_base
{
    static constexpr bool variable = true;

    static constexpr const char* name()
    {
        return "Driver's Identification";
    }

    static constexpr const char* abbrev() { return "DI"; }

    using spns = internal::spns_list<
        s::driver1_identification,
        s::driver2_identification>;
};

}

}}