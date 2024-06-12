#pragma once

#include "base.h"

namespace embr { namespace j1939 {

namespace pgn {

template <>
struct traits<pgns::ecu_identification_information> : internal::traits_base
{
    static constexpr const char* name()
    {
        return "ECU Identification Information";
    }

    static constexpr const char* abbrev() { return "ECUID"; }
};

}

}}
