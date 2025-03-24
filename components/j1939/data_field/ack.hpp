#pragma once

#include "base.h"

#include "../slots/macro/push.h"

namespace embr { namespace j1939 {

namespace spn {

template <>
struct type_traits<spns::control_byte_mack>
{

};

template <>
struct type_traits<spns::tp_address_ack>
{

};


template <>
struct type_traits<spns::pgn_ack>
{

};


}

namespace pgn {

template <>
struct traits<pgns::acknowledgement> : internal::traits_base
{
    static constexpr const char* name() { return "Acknowledgement"; }
    static constexpr const char* abbrev() { return "ACK"; }
};


}


}}

#include "../slots/macro/pop.h"
