/**
 * References:
 *
 * 1. J1939-71 (MAR2011)
 */
#pragma once

#include "base.hpp"

#include "../slots/velocity.hpp"

#include "../slots/macro/push.h"

namespace embr { namespace j1939 {

namespace spn {

template <>
struct type_traits<spns::brake_switch> : internal::measured_type_traits
{
    static constexpr const char* name() { return "brake_switch"; }
};

template <>
struct type_traits<spns::parking_brake_switch> : internal::measured_type_traits
{
    static constexpr const char* name() { return "parking_brake_switch"; }
};

template <>
struct type_traits<spns::wheel_based_vehicle_speed> :
    internal::slot_type_traits<slots::SAEvl02>
{
    static constexpr const char* name() { return "wheel_based_vehicle_speed"; }
};

template<>
constexpr descriptor get_descriptor<spns::brake_switch>()
{
    return descriptor{ 4, 5, 2};
}

template<>
constexpr descriptor get_descriptor<spns::parking_brake_switch>()
{
    return { 1, 3, 2 };
}

template<>
constexpr descriptor get_descriptor<spns::wheel_based_vehicle_speed>()
{
    return descriptor{ 2, 1, 16};
}

}

template <class Container>
struct data_field<pgns::ccvs, Container> :
    internal::data_field_base<Container>
{
    typedef internal::data_field_base<Container> base_type;

    ESTD_CPP_FORWARDING_CTOR(data_field)

    EMBR_J1939_PROPERTY(brake_switch)
    EMBR_J1939_PROPERTY(parking_brake_switch)
    EMBR_J1939_PROPERTY(wheel_based_vehicle_speed)
};

namespace pgn {

template <>
struct traits<pgns::ccvs> : internal::traits_base
{
    using spns = internal::spns_list<
        s::brake_switch,
        s::parking_brake_switch,
        s::wheel_based_vehicle_speed>;

    static constexpr const char* name()
    {
        return "Cruise Control/Vehicle Speed";
    }

    static constexpr const char* abbrev() { return "CCVS"; }
};

}

}}

#include "../slots/macro/pop.h"
