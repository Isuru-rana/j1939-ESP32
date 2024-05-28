#pragma once

#include <estd/algorithm.h>

#include "enum.h"
#include "base.h"

namespace embr { namespace j1939 { namespace sm { namespace tp { inline namespace v0 {

struct originator_state : enum_base
{
    union
    {
        const uint8_t* payload_;
        errors error_;
    };
    // DEBT: Feature flag in bit struct in case we do feel like burning up ROM to save a little RAM
#if FEATURE_ESTD_AGGRESIVE_BITFIELD
#endif
    // DEBT: Tracking this as 3 discrete bytes would likely save low level bit operations
    // and space.  That's presuming we bring back the bit struct in the first place.  As
    // it stands we're well smaller than responder_state counterpart, so we have wiggle room
    const uint32_t pgn_ ;// : 24;
    // Not implemented yet.  Theoretically it's very easy to auto-advance payload pointer, presuming
    // that's what is desired (not always!)
    bool auto_payload_;
    const uint16_t total_size_;

    // Last sequence number sent out
    uint8_t last_sequence_;
    uint8_t max_packets_per_cts_;   // Can be adjusted down by CTS message
    uint8_t current_packet_per_cts_;
    const uint8_t responder_address_;

    constexpr explicit originator_state(uint16_t total_size, uint8_t responder_address, uint32_t pgn) :
        payload_{nullptr},
        pgn_{pgn},
        auto_payload_{false},
        total_size_{total_size},
        last_sequence_{0},
        max_packets_per_cts_{0xFF},
        current_packet_per_cts_{0},
        responder_address_{responder_address}
    {}

    constexpr unsigned max_sequence() const
    {
        return (total_size_ + 7) / 7;
    }

    /// Absolute position of payload during ORIGINATOR_SENDING_DT
    /// Remember this is 1-index-based
    constexpr uint16_t last_position() const
    {
        return last_sequence_ * 7;
    }

    constexpr bool resequence_requested() const
    {
        return payload_ == nullptr && last_sequence_ > 0;
    }

    constexpr bool hold_requested() const
    {
        return max_packets_per_cts_ == 0;
    }

    uint16_t payload_size() const
    {
        const unsigned remaining_to_send = total_size_ - (last_sequence_ * 7);

        return estd::min(remaining_to_send, 7U);
    }

    // Last sent sequence OR last resequence-requested seq
    uint8_t last_sequence() const { return last_sequence_; }

    constexpr bool sent_everything() const
    {
        return last_position() >= total_size_;
    }

    constexpr bool bam() const { return responder_address_ == 0xFF; }

    template <class TimePoint>
    pdu<pgns::tp_cm> build_abort(const context<TimePoint>& ctx, abort_reasons r) const
    {
        pdu<pgns::tp_cm> cm;

        cm.destination_address(responder_address_);
        prep_abort(cm, ctx, r);

        return cm;
    }
};

}}}}}
