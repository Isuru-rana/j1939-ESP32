#pragma once

#include "../../can_id.h"
#include "../../pdu/header.h"

#include "dispatch.h"

namespace embr { namespace j1939 {

#ifdef __cpp_concepts
namespace concepts {
template <class F>
concept Functor = requires(F f)
{
    { f(pgns{}) };
};

}
#endif

namespace internal {

template <class Key, Key key, class F>
bool dispatch_one(F&& f, Key compare_to)
{
    if(compare_to == key) return false;

    f();

    return true;
}

#if __cpp_fold_expressions
template <class Key, Key ...keys, class F>
void dispatch_assist(estd::integer_sequence<Key, keys...>, F&& f, Key key)
{
    (... || dispatch_one<Key, keys>(std::forward<F>(f), key));
}
#endif


#define J1939_DISPATCH_TARGET(n)    \
case pgns::n:   return exec_dispatch<Policy, pgns::n>{}(std::forward<F>(f), std::forward<Args>(args)...);
//case pgns::n:   return f(in_place_pgn<pgns::n>{}, std::forward<Args>(args)...);


// Want to do this, but the variadic portion is a little tricky
//template <ESTD_CPP_CONCEPT(concepts::Functor) F>
template <class Policy, class F, class ...Args>
auto dispatch(F&& f, pgns pgn_, Args&&...args) -> decltype(f(pgns{}, args...))
{
    // NOTE: Would be interesting to do this with estd::variadic and/or a fold expression, but I am concerned that it would
    // destroy the optimizer
#if __cpp_fold_expressions
    //using s = estd::make_integer_sequence<unsigned, int(pgns::mf3_end)>;

    // Nifty idea, but constexpr recursion depth kills this
    //dispatch_assist(s{}, std::forward<F>(f), pgn_);
#endif


    switch(pgn_)
    {
        J1939_DISPATCH_TARGET(ac_switching_device_status)
        J1939_DISPATCH_TARGET(acknowledgement)
        J1939_DISPATCH_TARGET(address_claimed)
        J1939_DISPATCH_TARGET(ambient_conditions)
        J1939_DISPATCH_TARGET(auxiliary_analog_information)
        J1939_DISPATCH_TARGET(auxiliary_input_output_status_1)
        J1939_DISPATCH_TARGET(battery_status)
        J1939_DISPATCH_TARGET(bjm1)
        J1939_DISPATCH_TARGET(bjm2)
        J1939_DISPATCH_TARGET(bjm3)
        J1939_DISPATCH_TARGET(brakes)
        J1939_DISPATCH_TARGET(cab_illumination_message)
        J1939_DISPATCH_TARGET(ccvs)
        J1939_DISPATCH_TARGET(charger_status)
        J1939_DISPATCH_TARGET(cm1)
        J1939_DISPATCH_TARGET(cm3)
        J1939_DISPATCH_TARGET(commanded_address)
        J1939_DISPATCH_TARGET(dash_display)
        J1939_DISPATCH_TARGET(dc_detailed_status)
        J1939_DISPATCH_TARGET(direct_lamp_control_data1)
        J1939_DISPATCH_TARGET(ecm_information)
        J1939_DISPATCH_TARGET(ecu_identification)
        J1939_DISPATCH_TARGET(electronic_brake_system1)
        J1939_DISPATCH_TARGET(external_brake_request)
        J1939_DISPATCH_TARGET(extended_joystick_message_1)
        J1939_DISPATCH_TARGET(ejm2)
        J1939_DISPATCH_TARGET(fan_drive_1)
        J1939_DISPATCH_TARGET(fan_drive_2)
        J1939_DISPATCH_TARGET(gnss_position_data)
        J1939_DISPATCH_TARGET(lighting_command)
        J1939_DISPATCH_TARGET(lighting_data)
        J1939_DISPATCH_TARGET(mf0)
        J1939_DISPATCH_TARGET(mf1)
        J1939_DISPATCH_TARGET(NAME_management_message)
        J1939_DISPATCH_TARGET(oel)
        J1939_DISPATCH_TARGET(operator_indicators)
        J1939_DISPATCH_TARGET(request)
        J1939_DISPATCH_TARGET(sensor_electrical_power_1)
        J1939_DISPATCH_TARGET(sensor_electrical_power_2)
        J1939_DISPATCH_TARGET(shutdown)
        J1939_DISPATCH_TARGET(switch_bank_control)
        J1939_DISPATCH_TARGET(switch_bank_status)
        J1939_DISPATCH_TARGET(system_time)
        J1939_DISPATCH_TARGET(tp_cm)
        J1939_DISPATCH_TARGET(tp_dt)
        J1939_DISPATCH_TARGET(time_date)
        J1939_DISPATCH_TARGET(time_date_adjust)
        J1939_DISPATCH_TARGET(trip_fan_information)
        J1939_DISPATCH_TARGET(vehicle_direction_speed)
        J1939_DISPATCH_TARGET(vehicle_hours)
        J1939_DISPATCH_TARGET(vehicle_position)
        J1939_DISPATCH_TARGET(vep1)
        J1939_DISPATCH_TARGET(vep2)
        J1939_DISPATCH_TARGET(vep3)

        default:    return f(pgn_, std::forward<Args>(args)...);
    }
}

#undef J1939_DISPATCH_TARGET

constexpr pgns get_pgn(const can_id& id)
{
    return id.is_pdu1() ?
        pgns(pdu1_header(id).range()) :
        pgns(pdu2_header(id).range());
}

template <class F, class ...Args>
auto dispatch(F&& f, can_id id, Args&&...args) -> decltype(f(pgns{}, args...))
{
    return dispatch<dispatch_default_policy>(
        std::forward<F>(f),
        get_pgn(id),
        std::forward<Args>(args)...);
}


}

}}
