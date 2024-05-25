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


#define FEATURE_EMBR_J1939_STRICT_STATES 1
#define FEATURE_EMBR_J1939_STRICT_PROTOCOL 1

// DEBT: I am so sure I did this before.  Can't seem to find it though

// v0 designates still in development, not functional
namespace embr { namespace j1939 { namespace sm { inline namespace v0 {

// DEBT: Probably we want a separate responder & originator state machine
// "SENDING" states are a signal for external party to pick up a message from
// state machine and send it
class transport_protocol : public impl::controller_application_base
{
public:
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

    enum roles
    {
        ROLE_UNINITIALIZED,
        ROLE_ORIGINATOR,
        ROLE_RESPONDER
    };

    enum frame_errors
    {
        FRAME_NOMINAL,  // A-OK
        // Generic
        FRAME_ERROR,
        FRAME_TIMEOUT,
        FRAME_WARN,
        FRAME_INVALID_STATE
    };

    // EXPERIMENTAL, not used
    enum frame_states
    {
        FRAME_IDLE,
        FRAME_RECEIVING,
        FRAME_RECEIVED,
        FRAME_SENDING,
        FRAME_SENT
    };

    // EXPERIMENTAL, not used
    enum frame_types
    {
        FRAME_CTS,
        FRAME_RTS,
        FRAME_ACK,
        FRAME_ABORT,

        FRAME_DT,
    };

    // EXPERIMENTAL, not used - consider eventually merging with embr service
    // architecture
    struct frame_tracker
    {
        roles role_ : 4;
        frame_states state_ : 4;
        frame_types type_ : 4;
        frame_errors error_ : 4;
    };

    enum states
    {
        IDLE,
        // Invalid state observed, but occurred at a time which doesn't hurt us
        WARN,
        RECEIVING,
        SENDING_ABORT,
        SENT_ABORT,

        // Originator node states
        ORIGINATOR_SENDING_RTS,
        ORIGINATOR_SENT_RTS,
        ORIGINATOR_SENDING_BAM,
        ORIGINATOR_SENT_BAM,
        ORIGINATOR_WAITING_CTS,
        ORIGINATOR_RECEIVED_CTS,
        ORIGINATOR_SENDING_DT,
        ORIGINATOR_SENT_DT,
        ORIGINATOR_RECEIVED_ABORT,
        ORIGINATOR_RECEIVED_EOM_ACK,
        ORIGINATOR_TIMEOUT,     // Timed out waiting for responder
        ORIGINATOR_ERROR,

        // Responder node states
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

    enum errors
    {
        ORIGINATOR_ERROR_MISMATCHED_PGM
    };

    using time_point = unsigned;
    using duration = unsigned;

    struct context
    {
        const time_point current;
        const uint8_t self_address;
    };

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

    struct preamble
    {

    };

    // responder established connections state
    struct responder_state
    {
        // NOTE: We permit modification of 'max_packets' here specifically on responder
        // side.  '0' / hold is handled via RESPONDER_SENT_CTS_HOLD
        pdu<pgns::tp_cm> originator_;
        // DEBT: In theory, we could flow through the original transport frame and use a pointer
        // to that.  In reality, we're only talking 8 bytes here
        layer1::data_field<pgns::tp_dt> current_dt_;
        uint8_t current_packet_per_cts_;
        uint8_t retransmit_counter_;

        explicit responder_state(const pdu<pgns::tp_cm>&);

        // Always represents last received sequence number
        constexpr uint8_t seq() const
        {
            return current_dt_.sequence_number();
        }

        // While in RESPONDER_RECEIVING_DT, this is your guy
        constexpr uint16_t receiving_bytes() const
        {
            return (seq() - 1) * 7;
        }

        // NOTE: Doesn't account for last packet
        constexpr uint16_t received_bytes() const
        {
            return seq() * 7;
        }

        constexpr bool last_one() const
        {
            return current_dt_.sequence_number() == originator_.total_packets().value();
        }

        // DEBT: Need a better name - this indicates if maximum packets per CTS flow is reached
        constexpr bool last_one_per_batch() const
        {
            return originator_.max_packets() == current_packet_per_cts_;
        }

        // NOTE: Only valid during limited states, and never goes to 0
        // (that's up to you to figure out)
        uint16_t remaining_bytes() const
        {
            const uint16_t total = originator_.total_size().value();
            if(last_one())
                return total % 7;
            else
                return total - received_bytes();
        }

        bool bam() const
        {
            return originator_.destination_address() == uint8_t(addresses::global);
        }

        // Requested pgn
        j1939::pgns pgn() const { return (j1939::pgns)originator_.payload().pgn(); }

        uint8_t max_packets() const { return originator_.max_packets(); }
    };

    struct originator_state
    {
        union
        {
            const uint8_t* current_payload_;
            errors error_;
        };
        const uint32_t pgn_;
        const uint16_t total_size_;
        uint8_t current_sequence_;
        uint8_t max_packets_per_cts_;   // Can be adjusted down by CTS message
        uint8_t current_packet_per_cts_;
        const uint8_t responder_address_;

        constexpr explicit originator_state(uint16_t total_size, uint8_t responder_address, uint32_t pgn) :
            current_payload_{nullptr},
            pgn_{pgn},
            total_size_{total_size},
            current_sequence_{0},
            max_packets_per_cts_{0xFF},
            current_packet_per_cts_{0},
            responder_address_{responder_address}
        {}

        uint16_t current_position() const
        {
            return current_sequence_ * 7;
        }

        bool resequence_requested() const
        {
            return current_payload_ == nullptr && current_sequence_ > 0;
        }

        bool hold_requested() const
        {
            return max_packets_per_cts_ == 0;
        }

        // DEBT: Shrink this down when we get to the very end of the line.  For now we
        // do a (usually harmless) read buffer overrun.  Obviously a no no, but unlikely
        // to cause any immediate problems
        uint16_t payload_size() const
        {
            return 7;
        }

        // Last sent sequence OR last resequence-requested seq
        uint8_t current_sequence() const { return current_sequence_; }

        bool bam() const { return responder_address_ == 0xFF; }
    };

    // DEBT: Default constructor seems a little ornery
    estd::internal::variant_storage<
        preamble,
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

    using modes = pdu<pgns::tp_cm>::modes;
    using abort_reasons = pdu<pgns::tp_cm>::abort_reasons;

#if UNIT_TESTING
public:
#endif

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

    const responder_state& responder() const
    {
        return *storage_.get<responder_state>();
    }

    const originator_state& originator() const
    {
        return *storage_.get<originator_state>();
    }

    void prep_cts(pdu<pgns::tp_cm>&, const context&);
    pdu<pgns::tp_cm> build_abort(const context&, abort_reasons);

public:
    constexpr states state() const { return state_; }

    roles role() const;

    // DEBT: Poor naming, only applies to responder mode
    bool payload_present()
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

        // DEBT: Un-hardcode 7
        return { responder().current_dt_.packetized_data(), 7 };
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

        originator().current_payload_ = v;
        state_ = ORIGINATOR_SENDING_DT;
    }

    template <class Transport, pgns pgn>
    static constexpr bool process_incoming(Transport& t, pdu<pgn> p, context) { return false; }

    // Using dispatcher methodology
    template <class Transport>
    bool process_incoming(Transport&, const pdu<pgns::tp_cm>&, const context&);

    // Using dispatcher methodology
    template <class Transport>
    bool process_incoming(Transport&, const pdu<pgns::tp_dt>&, const context&);

    // Combining time-bound operations since they are likely send related anyway
    template <class Transport>
    bool process_outgoing(Transport&, const context&);

    //bool process_time(time_point);

    // Indicates state machine should kick into originator mode
    void initiate_originator(uint16_t sz, const context&, uint8_t responder_address, uint32_t pgn);

    // Indicate we've consumed the latest DT chunk
    void mark_dt_received();

    time_point next_event() const;
};

}}}}
