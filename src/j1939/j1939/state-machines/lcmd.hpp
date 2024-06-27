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
void lighting_command<TimePoint>::main_light_switch(pdu<pgns::lcmd>& out_p, const context&)
{
    using cc = spn::control_commands;
    using type = enum_type<spns::main_light_switch>;
    using htype = enum_type<spns::high_low_beam_switch>;
    // high beam only activates if expressly specified on.  Otherwise default to low beam
    const bool hibeam = last_oel_.high_low_beam_switch() == htype::high_beam_selected;

    // NOTE: Incomplete
    switch(last_oel_.main_light_switch())
    {
        case type::off:
            out_p.high_beam_headlight(spn::control_commands::disable);
            out_p.low_beam_headlight(spn::control_commands::disable);
            break;

        case type::park_on:
            // DEBT: fog lights are not really parking lights, right
            out_p.front_fog_lights(cc::enable);
            out_p.rear_fog_lights(cc::enable);
            break;

        case type::headlight_and_park_on:
            out_p.front_fog_lights(cc::enable);
            out_p.rear_fog_lights(cc::enable);
            out_p.low_beam_headlight(!hibeam ? cc::enable : cc::disable);
            out_p.high_beam_headlight(hibeam ? cc::enable : cc::disable);
            break;

        case type::headlight_on:
            out_p.front_fog_lights(cc::disable);
            out_p.rear_fog_lights(cc::disable);
            out_p.low_beam_headlight(!hibeam ? cc::enable : cc::disable);
            out_p.high_beam_headlight(hibeam ? cc::enable : cc::disable);
            break;

        case type::delayed_off:
            // DEBT: Supersedes blinker flash state.  Ideally should coexist
            // DEBT: Will loop right now between IDLE and DELAYED_OFF states
            // since we anticipate last_oel_ will continue to indicate 'delayed_off'.
            state_ = STATE_DELAYED_OFF;
            // DEBT: Get off delay from  last_oel_.operators_desired_delay_lamp_off_time()
            next_event_ += off_delay();
            break;

        default:
            break;
    }
}


// DEBT: This main 'prep' may be poorly named
// and is somewhat specific to process_outgoing/timer specificity
template <class TimePoint>
void lighting_command<TimePoint>::prep(pdu<pgns::lcmd>& out_p, const context& c)
{
    using cc = spn::control_commands;

    if(state_ == STATE_IDLE)
    {
        // Lazy init
        next_event_ = c.current;
    }
    else if(state_ == STATE_DELAYED_OFF)
    {
        state_ = STATE_IDLE;
        out_p.high_beam_headlight(cc::disable);
        out_p.low_beam_headlight(cc::disable);
        return;
    }

    bool flash_requested = false;
    const bool on_already = state_ == STATE_FLASH_ON;

    using signal = enum_type<spns::turn_signal_switch>;
    using hazard = enum_type<spns::hazard_light_switch>;    // aka spn::measured

    const auto cmd = on_already ? cc::disable : cc::enable;

    switch(last_oel_.turn_signal_switch())
    {
        case signal::right_turn_to_be_flashing:
            flash_requested = true;
            //c.next(flash_delay);
            out_p.right_turn_signal(cmd);
            out_p.left_turn_signal(spn::control_commands::disable); // DEBT: Cache this to know to leave this as noop
            break;

        case signal::left_turn_to_be_flashing:
            flash_requested = true;
            out_p.left_turn_signal(cmd);
            out_p.right_turn_signal(spn::control_commands::disable);
            break;

        case signal::no_turn_being_signaled:
            out_p.right_turn_signal(spn::control_commands::disable);
            out_p.left_turn_signal(spn::control_commands::disable);
            state_ = STATE_IDLE;
            break;

        // NOTE: Hazard lights will override this solid-on behavior
        case signal::error:
            out_p.right_turn_signal(spn::control_commands::enable);
            out_p.left_turn_signal(spn::control_commands::enable);
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
            flash_requested = true;
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

    if(flash_requested)
    {
        state_ = on_already ? STATE_FLASH_OFF : STATE_FLASH_ON;
        next_event_ += flash_delay();
    }
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
    main_light_switch(out_p, c);

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
