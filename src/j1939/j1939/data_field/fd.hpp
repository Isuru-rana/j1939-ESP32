#pragma once

#include "base.h"
#include "../slots/percent.hpp"

#include "../slots/macro/push.h"

namespace embr { namespace j1939 {

namespace spn {

template <>
struct type_traits<spns::estimated_percent_fan_speed> :
    internal::slot_type_traits<slots::SAEpc03> {};

template <>
struct type_traits<spns::fan_drive_state> :
    internal::type_traits_base<uint8_t>
{
    enum class enum_type : uint8_t
    {
        off = 0,
        engine_system_general,
        excessive_engine_air_temperature,
        excessive_engine_oil_temperature,
        excessive_engine_coolant_temperature,
        excessive_engine_transmission_oil_temperature,
        excessive_engine_hydraulic_oil_temperature,
        default_operation,
        not_defined,
        manual_control,
        transmission_retarder,
        ac_system,
        timer,
        engine_brake,
        other,
        not_available,
        noop = not_available
    };

    using value_type = enum_type;
};


template<>
constexpr descriptor get_descriptor<spns::estimated_percent_fan_speed>()
{
    return { 1, 1, 8 };
}

template<>
constexpr descriptor get_descriptor<spns::fan_drive_state>()
{
    return { 2, 1, 4 };
}

template<>
constexpr descriptor get_descriptor<spns::fan_speed>()
{
    return { 3, 1, 16 };
}

}

template<class Container>
struct data_field<pgns::fan_drive_1, Container> :
    internal::data_field_base<Container>
{
    typedef internal::data_field_base<Container> base_type;

    ESTD_CPP_FORWARDING_CTOR(data_field)

    EMBR_J1939_PROPERTY(estimated_percent_fan_speed);
    EMBR_J1939_PROPERTY(fan_drive_state);
};

namespace pgn {

template <>
struct traits<pgns::fan_drive_1> : internal::traits_base
{
    static constexpr const char* name()
    {
        return "Fan Drive 1";
    }

    static constexpr const char* abbrev() { return "FD1"; }
};

}

}}

#include "../slots/macro/pop.h"
