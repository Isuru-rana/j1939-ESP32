/**
 * References
 *
 * 1. J1939-71 (REV. DEC2003)
 */
#pragma once

#include "base.hpp"

#include "../spn/fwd.h"
#include "../spn/traits.h"

namespace embr { namespace j1939 {

namespace spn {

template <>
struct type_traits<spns::number_of_software_identification_fields> : 
    internal::measured_type_traits
{
};


template <>
struct type_traits<spns::software_identification> :
    internal::ascii_type_traits<200>
{
    // TODO: ASCII type
};

}

namespace pgn {

template <>
struct traits<pgns::software_identification> : internal::traits_base
{
    static constexpr const char* name()
    {
        return "Software Identification";
    }

    static constexpr const char* abbrev() { return "SOFT"; }
};

}


// NOTE: Not sure we want a data field for software_identification pgn
// because it will far exceed our 32-byte utility limit

}}
