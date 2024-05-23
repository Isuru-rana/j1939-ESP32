/**
 *
 * Supports J1939-21 [1] 5.10 "Transport protocol Functions"
 *
 * References:
 *
 * 1. J1939-21 (DEC2006)
 */
#pragma once

#include <estd/variant.h>

#include "../addresses.h"
#include "../pdu.h"
#include "../ca.h"
#include "../data_field/transport_protocol.hpp"

#define FEATURE_EMBR_J1939_STRICT_STATES 1

// DEBT: I am so sure I did this before.  Can't seem to find it though

// v0 designates still in development, not functional
namespace embr { namespace j1939 { namespace sm { inline namespace v0 {

// DEBT: Probably we want a separate responder & originator state machine
// "SENDING" states are a signal for external party to pick up a message from
// state machine and send it
class transport_protocol : public impl::controller_application_base
{
public:
    enum states
    {
        IDLE,
        RECEIVING,

        // Originator node states
        ORIGINATOR_SENDING_RTS,
        ORIGINATOR_SENT_RTS,
        ORIGINATOR_SENDING_BAM,
        ORIGINATOR_SENT_BAM,
        ORIGINATOR_WAITING_CTS,
        ORIGINATOR_RECEIVED_CTS,
        ORIGINATOR_SENT_DT,
        ORIGINATOR_RECEIVED_EOM_ACK,

        // Responder node states
        RESPONDER_RECEIVED_RTS,
        RESPONDER_RECEIVED_BAM,
        RESPONDER_SENDING_CTS,
        RESPONDER_SENT_CTS,
        RESPONDER_RECEIVING_DT,
        RESPONDER_RECEIVED_DT,
        RESPONDER_SENT_EOM_ACK,
        RESPONDER_SENDING_ABORT,
        RESPONDER_SENT_ABORT,
    };

    using time_point = unsigned;

private:
    states state_ = IDLE;

    // DEBT: Would prefer this to come in via transport or some pseudo global thing
    // or perhaps only pass in traffic matched to global or our address in the first place
    uint8_t self_address_ = uint8_t(addresses::null_address);

    struct preamble
    {

    };

    // responder established connections state
    struct responder_established
    {
        pdu<pgns::tp_cm> originator_;
        const uint8_t* current_payload_;
        uint8_t current_sequence_;

        void init(const pdu<pgns::tp_cm>&);

        constexpr uint16_t received_bytes() const
        {
            return current_sequence_ * 7;
        }

        // NOTE: Only valid during limited states (TBD)
        uint16_t remaining_bytes() const
        {
            return originator_.total_size().value() - received_bytes();
        }
    };

    estd::internal::variant_storage<
        preamble,
        responder_established> storage_;

    /*
     * Union doesn't like non triviality of pdu_ object
    union
    {
        preamble preamble_;

        // Active during:
        // RESPONDER_RECEIVED_RTS, RESPONDER_RECEIVED_BAM
        // RESPONDER_SENT_CTS, RESPONDER_RECEIVING_DT
        responder_established established_;
    };  */

    using modes = pdu<pgns::tp_cm>::modes;

    responder_established& established()
    {
        auto v = storage_.get<responder_established>();
        return *v;
    }

    /*
     * DEBT: something goes wrong with const get on variant_storage
    const responder_established& established() const
    {
        const auto v = storage_.get<responder_established>();
        return *v;
    }   */

public:
    constexpr states state() const { return state_; }

    ///
    /// @return
    /// @remarks last pdu<tp_dt> passed in to process_incoming must still be in scope
    // DEBT: not const due to variant_storage glitch
    estd::span<const uint8_t> payload()
    {
#if FEATURE_EMBR_J1939_STRICT_STATES
        assert(state_ == RESPONDER_RECEIVING_DT);
#endif

        return { established().current_payload_, 7 };
    }

    template <class Transport, pgns pgn>
    static constexpr bool process_incoming(Transport& t, pdu<pgn> p) { return false; }

    // Using dispatcher methodology
    template <class Transport>
    bool process_incoming(Transport&, const pdu<pgns::tp_cm>&);

    // Using dispatcher methodology
    template <class Transport>
    bool process_incoming(Transport&, const pdu<pgns::tp_dt>&);

    // Combining time-bound operations since they are likely send related anyway
    template <class Transport>
    bool process_outgoing(Transport&, time_point);

    //bool process_time(time_point);

    // Indicates state machine should kick into originator mode
    void initiate_originator();

    // Indicate we've consumed the latest DT chunk
    void mark_dt_received();
};

}}}}