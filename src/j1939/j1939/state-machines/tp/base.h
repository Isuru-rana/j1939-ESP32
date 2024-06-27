#pragma once

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
        // all in ms

        using mst = estd::chrono::milliseconds;

        static constexpr unsigned bam = 50;         // DEBT: Would be better if this was configurable

        static constexpr unsigned Tr = 200;
        static constexpr unsigned Th = 500;
        static constexpr unsigned T1 = 750;
        static constexpr unsigned T2 = 1250;
        static constexpr unsigned T3 = 1250;
        static constexpr unsigned T4 = 1050;
    };



    struct policy_type : cs::v1::base::policy_type
    {
        using whitelist = pgn_list<pgns::tp_dt, pgns::tp_cm>;
    };


    enum states
    {
        IDLE = ROLE_UNINITIALIZED << role_shift,
        // Invalid state observed, but occurred at a time which doesn't hurt us
        WARN,
        // Active listener mode, think of this as a reservation for a particular originator address
        ANTICIPATING_RTS,
        RECEIVING,
        SENDING_ABORT,
        SENT_ABORT,
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
    bool ready_for_payload() const
    {
        return state_ == ORIGINATOR_SENT_DT ||
            state_ == ORIGINATOR_RECEIVED_CTS ||
            state_ == ORIGINATOR_SENT_BAM;
    }

};


}}}}}
