#pragma once

#include "../pdu.h"

#include "../data_field/oel.hpp"
#include "../data_field/lighting_command.hpp"

#include <j1939/addresses.h>

#include "lcmd.h"

namespace embr { namespace j1939 { namespace sm { inline namespace v0 {

template <class TimePoint>
void lighting_command<TimePoint>::prep(pdu<pgns::lcmd>& out_p, const context& c)
{
    constexpr const estd::chrono::milliseconds flash_delay(500);
    bool on_already = state_ != STATE_FLASH_ON;

    using signal = enum_type<spns::turn_signal_switch>;
    using hazard = enum_type<spns::hazard_light_switch>;

    switch(last_oel_.turn_signal_switch())
    {
        case signal::right_turn_to_be_flashing:
            next_event_ = c.current + flash_delay;
            c.next(flash_delay);
            out_p.right_turn_signal(
                on_already ?
                    spn::control_commands::disable :
                    spn::control_commands::enable);
            break;

        case signal::left_turn_to_be_flashing:
            next_event_ = c.current + flash_delay;
            c.next(flash_delay);
            out_p.left_turn_signal(
                on_already ?
                    spn::control_commands::disable :
                    spn::control_commands::enable);
            break;

        case signal::no_turn_being_signaled:
            out_p.right_turn_signal(spn::control_commands::disable);
            out_p.left_turn_signal(spn::control_commands::disable);
            break;
    }

    switch(last_oel_.hazard_light_switch())
    {
        case hazard::enabled:
            //out_p.right_turn_signal(spn::control_commands::enable);
            //out_p.left_turn_signal(spn::control_commands::enable);
            break;
    }

    state_ = on_already ? STATE_FLASH_OFF : STATE_FLASH_ON;

}


template <class TimePoint>
template <class Transport>
bool lighting_command<TimePoint>::process_incoming(Transport& t, const pdu<pgns::oel>& p, const context& c)
{
    last_oel_ = p.payload();

    using traits = transport_traits<Transport>;

    pdu<pgns::lcmd> out_p(c.self_address, addresses::global);

    prep(out_p, c);

    traits::send(t, out_p);

    return true;
}

template <class TimePoint>
template <class Transport>
bool lighting_command<TimePoint>::process_outgoing(Transport& t, const context& c)
{
    if(c.current < next_event_) return false;

    using traits = transport_traits<Transport>;

    pdu<pgns::lcmd> out_p(c.self_address, addresses::global);

    prep(out_p, c);

    traits::send(t, out_p);

    return false;
}

}}}}
