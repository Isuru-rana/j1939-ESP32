#pragma once

#include "../../pdu.h"
#include "../../data_field/transport_protocol.hpp"

#include "enum.h"

namespace embr { namespace j1939 { namespace sm { namespace tp { inline namespace v0 {

// responder established connections state
struct responder_state : enum_base
{
    // NOTE: We permit modification of 'max_packets' here specifically on responder
    // side.  '0' / hold is handled via RESPONDER_SENT_CTS_HOLD
    pdu<pgns::tp_cm> originator_;
    // DEBT: In theory, we could flow through the original transport frame and use a pointer
    // to that.  In reality, we're only talking 8 bytes here
    layer1::data_field<pgns::tp_dt> last_dt_;
    uint8_t current_packet_per_cts_;
    uint8_t retransmit_counter_;

    explicit responder_state(const pdu<pgns::tp_cm>&);

    // Always represents last received sequence number
    constexpr uint8_t seq() const
    {
        return last_dt_.sequence_number();
    }

    // While in RESPONDER_RECEIVING_DT, this is your guy
    constexpr uint16_t receiving_bytes() const
    {
        // Since seq is last received seq, and it's 1-index-based, we need to bump
        // down by one for position calculations
        return (seq() - 1) * 7;
    }

    // NOTE: Always on 7 byte boundaries, and not used directly by state machine
    // (only for benefit of external parties)
    constexpr uint16_t received_bytes() const
    {
        return seq() * 7;
    }

    bool last_one() const
    {
        return last_dt_.sequence_number() == originator_.total_packets().value();
    }

    // DEBT: Need a better name - this indicates if maximum packets per CTS flow is reached
    bool last_one_per_batch() const
    {
        return originator_.max_packets() == current_packet_per_cts_;
    }

    /*  Only used by tests, switching them to 'received_bytes'
    // NOTE: Only valid during limited states, and never goes to 0
    // (that's up to you to figure out)
    uint16_t remaining_bytes() const
    {
        const uint16_t total = originator_.total_size().value();
        if(last_one())
            return total % 7;
        else
            return total - received_bytes();
    }   */

    bool bam() const
    {
        return originator_.destination_address() == uint8_t(addresses::global);
    }

    // Requested pgn
    j1939::pgns pgn() const { return (j1939::pgns)originator_.payload().pgn(); }

    uint8_t max_packets() const { return originator_.max_packets(); }

    void prep_cts(pdu<pgns::tp_cm>& cm, uint8_t sa) const;

    estd::span<const uint8_t> payload()
    {
        const unsigned sz = last_one() ?
            originator_.total_size().value() % 7 :
            7;

        return {last_dt_.packetized_data(), sz };
    }

    template <class TimePoint>
    pdu<pgns::tp_cm> build_abort(const context<TimePoint>& ctx, abort_reasons r) const
    {
        pdu<pgns::tp_cm> cm(
            ctx.self_address,
            originator_.source_address(),
            r,
            originator_.payload().pgn());

        /*
        pdu<pgns::tp_cm> cm{null_t{}};

        cm.destination_address(originator_.source_address());
        prep_abort(cm, ctx, r); */

        return cm;
    }
};

}}}}}
