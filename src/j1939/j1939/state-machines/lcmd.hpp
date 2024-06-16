#pragma once

#include "../pdu.h"

#include "../data_field/ccvs.hpp"
#include "../data_field/oel.hpp"
#include "../data_field/lighting_command.hpp"

#include <j1939/addresses.h>

#include "lcmd.h"

namespace embr { namespace j1939 { namespace sm { inline namespace v0 {

template <class TimePoint>
constexpr lighting_command<TimePoint>::lighting_command() :
    state_{STATE_IDLE}
{

}

template <class TimePoint>
void lighting_command<TimePoint>::prep(pdu<pgns::lcmd>& out_p, const context& c)
{
    if(state_ == STATE_IDLE)
    {
        next_event_ = c.current;
    }

    bool on_already = state_ == STATE_FLASH_ON;

    using signal = enum_type<spns::turn_signal_switch>;
    using hazard = enum_type<spns::hazard_light_switch>;    // aka spn::measured

    const spn::control_commands cmd = on_already ?
        spn::control_commands::disable :
        spn::control_commands::enable;

    // TODO: Switch these next_event_ to +=

    switch(last_oel_.turn_signal_switch())
    {
        case signal::right_turn_to_be_flashing:
            next_event_ += flash_delay();
            //c.next(flash_delay);
            out_p.right_turn_signal(cmd);
            break;

        case signal::left_turn_to_be_flashing:
            next_event_ += flash_delay();
            //c.next(flash_delay);
            out_p.left_turn_signal(cmd);
            break;

        case signal::no_turn_being_signaled:
            out_p.right_turn_signal(spn::control_commands::disable);
            out_p.left_turn_signal(spn::control_commands::disable);
            state_ = STATE_IDLE;
            break;

        default: break;
    }

    //const hazard hcmd = on_already ?
    //    hazard::disabled :
    //    hazard::enabled;

    switch(last_oel_.hazard_light_switch())
    {
        case hazard::enabled:
            next_event_ += flash_delay();
            out_p.right_turn_signal(cmd);
            out_p.left_turn_signal(cmd);
            break;

        case hazard::disabled:
            out_p.right_turn_signal(spn::control_commands::disable);
            out_p.left_turn_signal(spn::control_commands::disable);
            state_ = STATE_IDLE;
            break;

        default: break;
    }

    state_ = on_already ? STATE_FLASH_OFF : STATE_FLASH_ON;

}


template <class TimePoint>
template <class Transport>
bool lighting_command<TimePoint>::process_incoming(Transport& t, const pdu<pgns::oel>& p, const context& c)
{
    // DEBT: Looks like we might not really care.  Caching to (potentially) minimize output response traffic
    // seems a very narrow edge case
    last_oel_ = p.payload();

    using traits = transport_traits<Transport>;

    pdu<pgns::lcmd> out_p(c.self_address, null_t{});

    prep(out_p, c);

    traits::send(t, out_p);

    return true;
}

template <class TimePoint>
template <class Transport>
bool lighting_command<TimePoint>::process_incoming(Transport& t, const pdu<pgns::ccvs>& p, const context& c)
{
    using traits = transport_traits<Transport>;

    pdu<pgns::lcmd> out_p(c.self_address, null_t{});

    switch(p.brake_switch())
    {
        case spn::measured::on:
            out_p.left_stop(spn::control_commands::enable);
            out_p.right_stop(spn::control_commands::enable);
            out_p.center_stop(spn::control_commands::enable);
            traits::send(t, out_p);
            break;

        case spn::measured::off:
            out_p.left_stop(spn::control_commands::disable);
            out_p.right_stop(spn::control_commands::disable);
            out_p.center_stop(spn::control_commands::disable);
            traits::send(t, out_p);
            break;

        default: break;
    }

    return {};
}


template <class TimePoint>
template <class Transport>
bool lighting_command<TimePoint>::process_outgoing(Transport& t, const context& c)  // NOLINT
{
    if(c.current < next_event_) return false;

    using traits = transport_traits<Transport>;

    pdu<pgns::lcmd> out_p(c.self_address);

    prep(out_p, c);

    traits::send(t, out_p);

    return false;
}

}}}}
