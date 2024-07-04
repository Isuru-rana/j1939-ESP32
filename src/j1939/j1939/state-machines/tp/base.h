/*
 * References:
 *
 * 1. J1939-21 (DEC2006)
 */
#pragma once

#include <estd/chrono.h>

#include "../../cs/base.h"

#include "enum.h"
#include "feature.h"
#include "context.h"
#include "originator.h"
#include "responder.h"

namespace embr { namespace j1939 { namespace sm { namespace tp { inline namespace v0 {

template <class TimePoint>
inline void prep_abort(
    pdu<pgns::tp_cm>& cm,
    const context<TimePoint>& ctx,
    enum_base::abort_reasons r)
{
    cm.source_address(ctx.self_address);
    cm.control(enum_base::modes::abort);
    cm.abort_reason(r);
}


struct policy
{
    static constexpr bool time_check = true;
};


class base :
    public tp::v0::enum_base,
    public cs::v1::base
{
    static constexpr unsigned role_shift = 8;

public:
    struct idle_state
    {
        uint8_t anticipated_address_;
    };

    using responder_state = tp::v0::responder_state;
    using originator_state = tp::v0::originator_state;

    // [1] 5.10.2.4
    struct timeouts
    {
        using mst = estd::chrono::milliseconds;

        // DEBT: bam and Tr are both somewhat variable.
        // bam is between 50-200mS, at discretion of us
        // Tr (seems to be) official upper limit of BAM
        // Tr is upper limit of non-bam data sends, but there is no lower limit

        static constexpr mst bam = mst{50};         // DEBT: Would be better if this was configurable

        static constexpr mst Tr = mst{200};         // "provide a response [...] within 0.2s" [1] 5.12.3
        static constexpr mst Th = mst{500};         // max gap between hold messages [1] Figure C1
        static constexpr mst T1 = mst{750};         // maximum gap between rx last packet & next packet [1] 5.10.2.4
        static constexpr mst T2 = mst{1250};        // maximum gap between CTS tx and DT rx [1] 5.10.2.4
        static constexpr mst T3 = mst{1250};        // "must wait at least 1.25s" for a response [1] 5.12.3 including ACK [1] 5.10.2.4
        static constexpr mst T4 = mst{1050};        // "hold connection open" timeout [1] 5.10.2.4
    };



    struct policy_type : cs::v1::base::policy_type
    {
        using whitelist = pgn_list<pgns::tp_dt, pgns::tp_cm>;
    };


    enum states
    {
        // At the ready
        IDLE = ROLE_UNINITIALIZED << role_shift,
        // Invalid state observed, but occurred at a time which doesn't hurt us
        WARN,
        // Active listener mode, think of this as a reservation for a particular originator address
        ANTICIPATING_RTS,
        RECEIVING,
        SENDING_ABORT,
        SENT_ABORT,

        // Won't respond to anything
        OFFLINE,

        // Originator node states
        ORIGINATOR = ROLE_ORIGINATOR << role_shift,
        ORIGINATOR_SENDING_RTS,
        ORIGINATOR_SENT_RTS,
        ORIGINATOR_SENDING_BAM,
        ORIGINATOR_SENT_BAM,
        ORIGINATOR_WAITING_CTS,
        ORIGINATOR_RECEIVED_CTS,
        ORIGINATOR_SENDING_DT,
        ORIGINATOR_SENT_DT,
        ORIGINATOR_SENT_ALL_DT,
        ORIGINATOR_RECEIVED_ABORT,
        ORIGINATOR_RECEIVED_EOM_ACK,
        ORIGINATOR_TIMEOUT,     // Timed out waiting for responder
        ORIGINATOR_ERROR,

        // Responder node states
        RESPONDER = ROLE_RESPONDER << role_shift,
        RESPONDER_RECEIVED_RTS,
        RESPONDER_RECEIVED_BAM,
        RESPONDER_SENDING_CTS,
        RESPONDER_SENT_CTS,
        RESPONDER_SENDING_CTS_HOLD,
        RESPONDER_SENT_CTS_HOLD,
        RESPONDER_RECEIVING_DT,
        RESPONDER_RECEIVED_DT,
        // Need this because BAM doesn't do EOM - though if we're clever we can use
        // responder().last_one()
        RESPONDER_RECEIVED_ALL_DT,
        RESPONDER_SENDING_EOM_ACK,
        RESPONDER_SENT_EOM_ACK,
        RESPONDER_SENDING_ABORT,
        RESPONDER_SENT_ABORT,
        RESPONDER_TIMEOUT,      // Timeout out waiting for originator
        RESPONDER_ERROR,
    };

protected:
    states state_ = IDLE;

#if UNIT_TESTING
public:
#endif

    // For responder role only, requests that a CTS of 0 can_send (hold) emit
    void request_hold();

public:
    constexpr states state() const { return state_; }

    // DEBT: Do state transitions if necessary
    void take_offline() { state_ = OFFLINE; }

    roles role() const;

    void set_offline()
    {
#if FEATURE_EMBR_J1939_STRICT_STATES
        assert(state_ == IDLE || state_ == WARN);
#endif

        state_ = OFFLINE;
    }

    void set_online()
    {
#if FEATURE_EMBR_J1939_STRICT_STATES
        assert(state_ == OFFLINE);
#endif

        state_ = IDLE;
    }

    // DEBT: Poor naming, only applies to responder mode
    constexpr bool payload_present() const
    {
        return state_ == RESPONDER_RECEIVING_DT;
    }

    // DEBT: Poor naming, only applies to originator mode
    ATTR_NODISCARD constexpr bool ready_for_payload() const
    {
        return state_ == ORIGINATOR_SENT_DT ||
            state_ == ORIGINATOR_RECEIVED_CTS ||
            state_ == ORIGINATOR_SENT_BAM;
    }

};

// DEBT: Move this into sm::v1 once FEATURE_EMBR_J1939_TP_FUTURE is fully settled down
// (we're close)
template <class TimePoint>
class to_schedule
{
public:
    using time_point = TimePoint;
    using duration = typename time_point::duration;

protected:
    time_point next_event_;

public:
    time_point next_event() const { return next_event_; }

    // DEBT: Experimenting, may want to change name.  May prefer old one with
    // explicit duration on it
    ATTR_NODISCARD constexpr bool elapsed(const sm::v0::context<TimePoint>& ctx) const
    {
        // time_point{} = zero, effectively

        return ctx.current >= next_event_ && next_event_ != time_point{};
    }
};


}}}}}
