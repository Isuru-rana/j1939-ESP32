#pragma once

#include "enum.h"

namespace embr { namespace j1939 { namespace sm { namespace tp { inline namespace v0 {

struct originator_state
{
    union
    {
        const uint8_t* current_payload_;
        errors error_;
    };
    const uint32_t pgn_;
    const uint16_t total_size_;
    uint8_t last_sequence_;
    uint8_t max_packets_per_cts_;   // Can be adjusted down by CTS message
    uint8_t current_packet_per_cts_;
    const uint8_t responder_address_;

    constexpr explicit originator_state(uint16_t total_size, uint8_t responder_address, uint32_t pgn) :
        current_payload_{nullptr},
        pgn_{pgn},
        total_size_{total_size},
        last_sequence_{0},
        max_packets_per_cts_{0xFF},
        current_packet_per_cts_{0},
        responder_address_{responder_address}
    {}

    constexpr unsigned max_position() const
    {
        return (total_size_ + 7) / 7;
    }

    /// Absolute position of payload during ORIGINATOR_SENDING_DT
    constexpr uint16_t current_position() const
    {
        return last_sequence_ * 7;
    }

    constexpr bool resequence_requested() const
    {
        return current_payload_ == nullptr && last_sequence_ > 0;
    }

    constexpr bool hold_requested() const
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
    uint8_t current_sequence() const { return last_sequence_; }

    constexpr bool sent_everything() const
    {
        return current_position() >= total_size_;
    }

    constexpr bool bam() const { return responder_address_ == 0xFF; }
};

}}}}}
