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

#include "tp/base.h"
#include "tp/enum.h"
#include "tp/feature.h"
#include "tp/originator.h"
#include "tp/responder.h"


// DEBT: Although I did a version before, the state machine flavor is far more flexible

// v0 designates still in development, not functional
namespace embr { namespace j1939 { namespace sm { inline namespace v0 {

// DEBT: Probably we want a separate responder & originator state machine
// "SENDING" states are a signal for external party to pick up a message from
// state machine and send it
template <class TimePoint>
class transport_protocol : public tp::v0::base
{
    using base_type = tp::v0::base;

public:
    using base_type::process_incoming;

    using time_point = TimePoint;
    using duration = typename time_point::duration;
    using context = sm::v0::context<time_point>;

private:
    time_point last_event_;

    constexpr duration elapsed(const context& ctx) const
    {
        return ctx.current - last_event_;
    }

    constexpr bool elapsed(const context& ctx, duration d) const
    {
        return ctx.current - last_event_ >= d;
    }

    // DEBT: make an estd::chrono overload for >= with std on lhs and estd on rhs
    template <class Rep, class Period>
    constexpr bool elapsed(const context& ctx, const estd::chrono::duration<Rep, Period>& d) const
    {
        return estd::chrono::duration<
            typename duration::rep,
            typename duration::period>(ctx.current - last_event_) >= d;
    }

    // DEBT: Would prefer this to come in via transport or some pseudo global thing
    // or perhaps only pass in traffic matched to global or our address in the first place
    //uint8_t self_address_ = uint8_t(addresses::null_address);

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
        return *storage_.template get<idle_state>();
    }

    responder_state& responder()
    {
        return *storage_.template get<responder_state>();
    }

    originator_state& originator()
    {
        return *storage_.template get<originator_state>();
    }

public:
    // NOTE: Just a formality, idle_state doesn't need init, and since state machines
    // love lazy init, we don't care about last_event_ either
    transport_protocol() :      // NOLINT
        storage_{estd::in_place_index_t<0>{}}
    {}

    const responder_state& responder() const
    {
        return *storage_.template get<responder_state>();
    }

    const originator_state& originator() const
    {
        return *storage_.template get<originator_state>();
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
        initiate_originator(sz, {time_point{}, 0}, responder_address, pgn);
    }

    // TODO: Make "advanced" flavor so that priority is possible too
    void initiate_originator(can_id);

#if FEATURE_EMBR_J1939_TP_AUTO_PAYLOAD
    // auto-payload mode
    void initiate_originator(uint8_t responder_address, uint32_t pgn,
        const void* payload, uint16_t sz);
#endif

    void initiate_responder(uint8_t originator_address);

    time_point next_event() const;
};

}}

const char* to_string(sm::tp::v0::base::states v);

}}
