/**
 *
 * Supports J1939-21 [1] 5.10 "Transport protocol Functions"
 *
 * References:
 *
 * 1. J1939-21 (DEC2006)
 */
#pragma once

#include "transport_protocol.h"

// DEBT: Put all these inliners out to a .cpp if we really end up not needing to
// templatize things

namespace embr { namespace j1939 { namespace sm { inline namespace v0 {

inline void transport_protocol::responder_established::init(const pdu<pgns::tp_cm>& p)
{
    originator_ = p;
    //current_payload_ = nullptr;
    // DEBT: We don't handle out of order sequences for the time being
    current_dt_.sequence_number(0);
}

template <class Transport>
bool transport_protocol::process_incoming(Transport&, const pdu<pgns::tp_cm>& p, const context& ctx)
{
    const uint8_t da = p.destination_address();
    if(da != ctx.self_address &&
        p.control() != modes::bam)
        return false;

    switch(state_)
    {
        case IDLE:
        {
            last_event_ = ctx.current;

            switch(p.control())
            {
                // NOTE: Won't get here yet due to self_address_ filter
                case modes::bam:
                {
                    state_ = RESPONDER_RECEIVED_BAM;
                    established().init(p);
                    return true;
                }

                case modes::rts:
                {
                    state_ = RESPONDER_RECEIVED_RTS;
                    established().init(p);
                    return true;
                }

                // "If a CTS is received while a connection is not established, it shall be ignored."
                case modes::cts:
                    return false;

                // RTS & BAM is the only valid message for this to receive when idle
                default:
                    state_ = WARN;
                    break;
            }
            break;
        }

        case ORIGINATOR_SENT_DT:
            switch(p.control())
            {
                // Handshake stuff, kind of an intermediate ack and occasionally re-requesting
                // already-sent packets
                case modes::cts:
                    if(p.to_send() != originator().current_sequence_ + 1)
                    {
                        // resend/retransmit time
                        // We double duty this pointer as a flag to indicate a retransmit is requested
                        originator().current_payload_ = nullptr;
                        originator().current_sequence_ = p.to_send().value();
                    }
                    state_ = ORIGINATOR_RECEIVED_CTS;
                    return true;

                case modes::ack:
                    // We could check here if we truly sent out everything we wanted to
                    state_ = ORIGINATOR_RECEIVED_EOM_ACK;
                    return true;

                default:    break;
            }
            break;

        case ORIGINATOR_SENT_RTS:
            switch(p.control())
            {
                case modes::cts:
                    state_ = ORIGINATOR_RECEIVED_CTS;
                    return true;

                default:    break;
            }
            break;

        default: break;
    }

    return false;
}

template <class Transport>
bool transport_protocol::process_incoming(Transport&, const pdu<pgns::tp_dt>& p,
    const context& ctx)
{
    bool bam = established().bam() && role() == ROLE_RESPONDER;

    if(p.destination_address() != ctx.self_address && !bam)
        return false;

    switch(state_)
    {
        // Warnings, soft as they are, auto reset back to IDLE
        case WARN:
            state_ = IDLE;
            break;

        case IDLE:
            break;

        case RESPONDER_RECEIVED_DT:
        case RESPONDER_SENT_CTS:
        {
            const uint8_t seq = p.sequence_number();
            const uint8_t expected_seq = established().current_dt_.sequence_number() + 1;

            if(seq == expected_seq)
            {
                state_ = RESPONDER_RECEIVING_DT;
                established().current_dt_ = p.payload();
            }
            else
            {
                state_ = RESPONDER_SENDING_ABORT;
            }

            return true;
        }

        default: break;
    }

    return false;
}


template <class Transport>
bool transport_protocol::process_outgoing(Transport& t, const context& ctx)
{
    using traits = transport_traits<Transport>;

    switch(state_)
    {
        case ORIGINATOR_SENDING_DT:
        {
            pdu<pgns::tp_dt> dt;

            uint8_t seq = ++originator().current_sequence_;

            estd::copy_n(originator().current_payload_, 7, dt.packetized_data());
            dt.sequence_number(seq);
            dt.source_address(ctx.self_address);
            dt.destination_address(originator().responder_address_);

            traits::send(t, dt);
            state_ = ORIGINATOR_SENT_DT;
            return true;
        }

        case ORIGINATOR_SENDING_RTS:
        {
            pdu<pgns::tp_cm> cm;
            const uint16_t& sz = originator().total_size_;

            cm.total_packets((sz + 7) / 7);
            cm.total_size(sz);
            cm.control(pdu<pgns::tp_cm>::rts);
            cm.destination_address(originator().responder_address_);
            cm.source_address(ctx.self_address);

            traits::send(t, cm);

            state_ = ORIGINATOR_SENT_RTS;
            last_event_ = ctx.current;
            return true;
        }

        // Got RTS, send CTS
        case RESPONDER_RECEIVED_RTS:
        {
            pdu<pgns::tp_cm> p;

            p.control(modes::cts);
            p.destination_address(established().originator_.source_address());
            p.source_address(ctx.self_address);
            p.to_send(1);
            p.can_send(established().originator_.max_packets());

            //state_ = RESPONDER_SENDING_CTS;
            traits::send(t, p);
            state_ = RESPONDER_SENT_CTS;
            return true;
        }

        // Kind of a special case, picks up state set in payload retrieval and transitions
        // to end phase if all packets received.  Might want to put this elsewhere
        case RESPONDER_RECEIVED_DT:
            if(established().last_one())
            {
                pdu<pgns::tp_cm> p = established().originator_;

                p.control(modes::ack);
                p.destination_address(established().originator_.source_address());
                p.source_address(ctx.self_address);

                traits::send(t, p);

                state_ = RESPONDER_SENT_EOM_ACK;
                return true;
            }
            break;

        // +++ Timeouts

        case RESPONDER_SENT_CTS:
            // [1] Section 5.12.3
            if(elapsed(ctx, timeouts::T2))
            {
                state_ = RESPONDER_TIMEOUT;
            }
            break;

        case ORIGINATOR_SENT_RTS:
            // [1] Section 5.12.3
            if(elapsed(ctx, timeouts::T3))
            {
                state_ = ORIGINATOR_TIMEOUT;
            }
            break;

        case ORIGINATOR_RECEIVED_CTS:
            // If responder asked for a hold, they will need to re-send a CTS
            // to keep us alive.  Otherwise, timeout
            // "a lack of a CTS for more than (T4) seconds after a CTS (0) message to “hold the
            //  connection open” will all cause a connection closure to occur" [1] Section 5.10.2.4
            if(originator().max_packets_per_cts_ == 0 &&
                elapsed(ctx, timeouts::T4))
            {
                state_ = ORIGINATOR_TIMEOUT;
            }
            break;

        // --- Timeouts

        default: break;
    }

    return false;
}

/*
inline bool transport_protocol::process_time(time_point)
{
    return false;
}
 */

inline void transport_protocol::initiate_originator(uint16_t sz, const context& ctx, uint8_t dest_address)
{
    state_ = ORIGINATOR_SENDING_RTS;
    storage_.emplace<originator_state>(sz, dest_address);
}

inline void transport_protocol::mark_dt_received()
{
#if FEATURE_EMBR_J1939_STRICT_STATES
    assert(state_ == RESPONDER_RECEIVING_DT);
#endif

    state_ = RESPONDER_RECEIVED_DT;
}

// DEBT: A little clumsy.  Might be better to track role explicitly and rework state machine
// into a 2 way sending/receiving/sent, etc and ack, cts, rts, etc.
inline auto transport_protocol::role() const -> roles
{
    switch(state_)
    {
        case ORIGINATOR_RECEIVED_CTS:
        case ORIGINATOR_SENDING_BAM:
        case ORIGINATOR_SENDING_DT:
        case ORIGINATOR_SENT_DT:
            return ROLE_ORIGINATOR;

        case RESPONDER_RECEIVED_DT:
        case RESPONDER_RECEIVING_DT:
        case RESPONDER_SENT_CTS:
            return ROLE_RESPONDER;

        default:    return ROLE_UNINITIALIZED;
    }
}

}}}}
