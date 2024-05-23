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
    current_payload_ = nullptr;
    // DEBT: We don't handle out of order sequences for the time being
    current_sequence_ = 0;
}

template <class Transport>
bool transport_protocol::process_incoming(Transport&, const pdu<pgns::tp_cm>& p)
{
    if(p.destination_address() != self_address_)
        return false;

    switch(state_)
    {
        case IDLE:
        {
            switch(p.control())
            {
                // NOTE: Won't get here yet due to self_address_ filter
                case modes::bam:
                {
                    state_ = RESPONDER_RECEIVED_BAM;
                    established().init(p);
                    break;
                }

                case modes::rts:
                {
                    state_ = RESPONDER_RECEIVED_RTS;
                    established().init(p);
                    break;
                }

                // RTS & BAM is the only valid message for this to receive when idle
                default:
                    break;
            }
            break;
        }

        case ORIGINATOR_SENT_DT:
            switch(p.control())
            {
                case modes::ack:
                    // We could check here if we truly sent out everything we wanted to
                    state_ = ORIGINATOR_RECEIVED_EOM_ACK;
                    break;

                default:    break;
            }
            break;

        case ORIGINATOR_SENT_RTS:
            switch(p.control())
            {
                case modes::cts:
                    state_ = ORIGINATOR_RECEIVED_CTS;
                    break;

                default:    break;
            }
            break;

        default: break;
    }

    return false;
}

template <class Transport>
bool transport_protocol::process_incoming(Transport&, const pdu<pgns::tp_dt>& p)
{
    if(p.destination_address() != self_address_)
        return false;

    switch(state_)
    {
        case IDLE:
            break;

        case RESPONDER_SENT_CTS:
        {
            uint8_t seq = p.sequence_number();

            if(seq == established().current_sequence_ + 1)
            {
                state_ = RESPONDER_RECEIVING_DT;
                established().current_payload_ = p.data() + 1;
            }
            else
            {
                state_ = RESPONDER_SENDING_ABORT;
            }

            break;
        }

        default: break;
    }

    return false;
}


template <class Transport>
bool transport_protocol::process_outgoing(Transport& t, time_point)
{
    using traits = transport_traits<Transport>;

    switch(state_)
    {
        case RESPONDER_RECEIVED_RTS:
        {
            pdu<pgns::tp_cm> p;

            p.control(modes::cts);
            p.destination_address(established().originator_.source_address());

            //state_ = RESPONDER_SENDING_CTS;
            traits::send(t, p);
            state_ = RESPONDER_SENT_CTS;
            break;
        }

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

inline void transport_protocol::initiate_originator()
{
    //state_ = ORIGINATOR_SENDING_RTS;
    state_ = ORIGINATOR_SENT_RTS;
}

inline void transport_protocol::mark_dt_received()
{
#if FEATURE_EMBR_J1939_STRICT_STATES
    assert(state_ == RESPONDER_RECEIVING_DT);
#endif

    state_ = RESPONDER_RECEIVED_DT;
}

}}}}