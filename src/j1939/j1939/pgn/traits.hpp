#pragma once

#include "traits.h"

// DEBT: Was fun while it lasted, but these guys belong in their respective .hpp implementations.
// Otherwise traits specializations appear for data_fields which aren't yet defined, which causes
// issues with things like 'decompose'

namespace embr { namespace j1939 { namespace pgn {


template <>
struct traits<pgns::fan_drive_1>
{
    static constexpr const char* name()
    {
        return "Fan Drive 1";
    }

    static constexpr const char* abbrev() { return "FD1"; }
};


template <>
struct traits<pgns::cab_message1> : internal::traits_base
{
    using spns = internal::spns_list<
        s::requested_percent_fan_speed,
        s::cab_interior_temperature_command,
        s::battery_main_switch_hold_request,
        s::operator_seat_direction_switch,
        s::seat_belt_switch,
        s::park_brake_command,
        s::engine_automatic_start_enable_switch,
        s::auxiliary_heater_mode_request,
        s::request_cab_zone_heating>;

    static constexpr const char* name()
    {
        return "Cab Message 1";
    }

    static constexpr const char* description()
    {
        return "Message containing parameters originating from the vehicle cab.";
    }

    static constexpr const char* abbrev() { return "CM1"; }
};


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


template <>
struct traits<pgns::cab_message_3> : internal::traits_base
{
    static constexpr const char* name()
    {
        return "Cab Message 3";
    }

    static constexpr const char* description()
    {
        return "Provides information from Cab mounted operator inputs.";
    }

    static constexpr const char* abbrev() { return "CM3"; }
};

}}}