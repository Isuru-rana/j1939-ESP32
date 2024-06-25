/**
 *
 * Supports J1939-21 [1] 5.10 "Transport protocol Functions"
 *
 * References:
 *
 * 1. J1939-21 (DEC2006)
 */
#pragma once

#include <estd/chrono.h>
#include <estd/variant.h>

#include "../addresses.h"
#include "../pdu.h"
#include "../ca.h"
#include "../data_field/transport_protocol.hpp"

#include "tp/enum.h"
#include "tp/feature.h"
#include "tp/originator.h"
#include "tp/responder.h"


// DEBT: I am so sure I did this before.  Can't seem to find it though

// v0 designates still in development, not functional
namespace embr { namespace j1939 { namespace sm { inline namespace v0 {

// DEBT: Probably we want a separate responder & originator state machine
// "SENDING" states are a signal for external party to pick up a message from
// state machine and send it
class transport_protocol :
    public tp::v0::enum_base,
    public cs::v1::base
{
    using base_type = cs::v1::base;

public:
    using base_type::process_incoming;

    struct policy_type : base_type::policy_type
    {
        using whitelist = pgn_list<pgns::tp_dt, pgns::tp_cm>;
    };

    // [1] 5.10.2.4
    struct timeouts
    {
        // all in ms

        static constexpr unsigned bam = 50;         // DEBT: Would be better if this was configurable
        static constexpr unsigned Tr = 200;
        static constexpr unsigned Th = 500;
        static constexpr unsigned T1 = 750;
        static constexpr unsigned T2 = 1250;
        static constexpr unsigned T3 = 1250;
        static constexpr unsigned T4 = 1050;
    };

    static constexpr unsigned role_shift = 8;

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

    using time_point = unsigned;
    using duration = unsigned;
    using context = sm::v0::context<time_point>;

private:
    time_point last_event_;

    duration elapsed(const context& ctx) const
    {
        return ctx.current - last_event_;
    }

    bool elapsed(const context& ctx, duration d) const
    {
        return ctx.current - last_event_ >= d;
    }

    states state_ = IDLE;

    // DEBT: Would prefer this to come in via transport or some pseudo global thing
    // or perhaps only pass in traffic matched to global or our address in the first place
    //uint8_t self_address_ = uint8_t(addresses::null_address);

    struct idle_state
    {
        uint8_t anticipated_address_;
    };

    using responder_state = tp::v0::responder_state;
    using originator_state = tp::v0::originator_state;

    // DEBT: Default constructor seems a little ornery
    estd::internal::variant_storage<
        idle_state,
        responder_state,
        originator_state
        > storage_;

    /*
     * Union doesn't like non triviality of pdu_ object
    union
    {
        preamble preamble_;

        // Active during:
        // RESPONDER_RECEIVED_RTS, RESPONDER_RECEIVED_BAM
        // RESPONDER_SENT_CTS, RESPONDER_RECEIVING_DT
        responder_state established_;
    };  */

#if UNIT_TESTING
public:
#endif

    idle_state& idle()
    {
        return *storage_.get<idle_state>();
    };

    responder_state& responder()
    {
        return *storage_.get<responder_state>();
    }

    originator_state& originator()
    {
        return *storage_.get<originator_state>();
    }

    // For responder role only, requests that a CTS of 0 can_send (hold) emit
    void request_hold();

public:
    // NOTE: Just a formality, idle_state doesn't need init, and since state machines
    // love lazy init, we don't care about last_event_ either
    transport_protocol() :      // NOLINT
        storage_{estd::in_place_index_t<0>{}}
    {}

    const responder_state& responder() const
    {
        return *storage_.get<responder_state>();
    }

    const originator_state& originator() const
    {
        return *storage_.get<originator_state>();
    }

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

    ///
    /// @return
    /// @remarks last pdu<tp_dt> passed in to process_incoming must still be in scope
    // DEBT: not const due to variant_storage glitch
    estd::span<const uint8_t> payload()
    {
#if FEATURE_EMBR_J1939_STRICT_STATES
        assert(payload_present());
#endif

        state_ = RESPONDER_RECEIVED_DT;

        return responder().payload();
    }

    // DEBT: Poor naming, only applies to originator mode
    bool ready_for_payload() const
    {
        return state_ == ORIGINATOR_SENT_DT ||
            state_ == ORIGINATOR_RECEIVED_CTS ||
            state_ == ORIGINATOR_SENT_BAM;
    }

    void payload(const uint8_t* v)
    {
#if FEATURE_EMBR_J1939_STRICT_STATES
        assert(ready_for_payload());
#endif

        originator().payload_ = v;
        state_ = ORIGINATOR_SENDING_DT;
    }

    // Using dispatcher methodology
    template <class Transport>
    bool process_incoming(Transport&, const pdu<pgns::tp_cm>&, const context&);

#if FEATURE_EMBR_J1939_TP_RESPONDER
    // Using dispatcher methodology
    template <class Transport>
    bool process_incoming(Transport&, const pdu<pgns::tp_dt>&, const context&);
#endif

    // Combining time-bound operations since they are likely send related anyway
    template <class Transport>
    bool process_outgoing(Transport&, const context&);

    //bool process_time(time_point);

    // Indicates state machine should kick into originator mode
    // DEBT: Really don't think we need context anymore, keeping around just in case
    void initiate_originator(uint16_t sz, const context&, uint8_t responder_address, uint32_t pgn);
    void initiate_originator(uint8_t responder_address, uint32_t pgn, uint16_t sz)
    {
        initiate_originator(sz, {0, 0}, responder_address, pgn);
    }

#if FEATURE_EMBR_J1939_TP_AUTO_PAYLOAD
    // auto-payload mode
    void initiate_originator(uint8_t responder_address, uint32_t pgn,
        const void* payload, uint16_t sz);
#endif

    void initiate_responder(uint8_t originator_address);

    time_point next_event() const;
};

}}

const char* to_string(sm::v0::transport_protocol::states v);

}}
