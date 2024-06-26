#pragma once

#include "../cs/base.h"
#include "tp/context.h"

#include "../data_field/oel.hpp"

namespace embr { namespace j1939 { namespace sm { inline namespace v0 {

// DEBT: Consolidate with or displace ca::lighting command
template <class TimePoint>
class lighting_command : public cs::v1::base
{
    using base_type = cs::v1::base;

    TimePoint next_event_;

    // DEBT: Clumsy way to enforce the necessity of context
    template <class Transport, pgns pgn>
    static constexpr bool process_incoming(Transport&, const pdu<pgn>&) { return {}; }

public:
    using context = sm::v0::context<TimePoint>;

    struct policy_type : base_type::policy_type
    {
        using whitelist = pgn_list<pgns::oel, pgns::ccvs>;
    };

    enum states
    {
        STATE_IDLE,
        STATE_FLASH_OFF,            // off cycle of flashing phase, finishing at next_event_
        STATE_FLASH_ON,             // on cycle of flashing phase, finishing at next_event_
    };

protected:
    void main_light_switch();
    void prep(pdu<pgns::lcmd>&, const context&);

    j1939::layer1::data_field<pgns::oel> last_oel_;

    states state_;

public:
    constexpr estd::chrono::milliseconds flash_delay() const
    {
        return estd::chrono::milliseconds{500};
    }

    constexpr estd::chrono::milliseconds off_delay() const
    {
        return estd::chrono::milliseconds{4000};
    }

    constexpr lighting_command();

    using time_point = TimePoint;

    constexpr states state() const { return state_; }

    using base_type::process_incoming;

    time_point next_event() const { return next_event_; }

    template <class Transport>
    bool process_incoming(Transport&, const pdu<pgns::oel>&, const context&);

    template <class Transport>
    bool process_incoming(Transport&, const pdu<pgns::ccvs>&, const context&);

    template <class Transport>
    bool process_outgoing(Transport&, const context&);
};


}}}}
