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

namespace embr { namespace j1939 { namespace sm {

namespace tp { inline namespace v0 {

inline responder_state::responder_state(const pdu<pgns::tp_cm>& p) :
    originator_{p},
    current_packet_per_cts_{0},
    retransmit_counter_{0}
{
    last_dt_.sequence_number(0);
}

}}

inline namespace v0 {

template <class TimePoint, class Policy>
template <class Transport>
auto transport_protocol<TimePoint, Policy>::process_incoming(
    Transport&, const pdu<pgns::tp_cm>& p, const context& ctx) -> result
{
    const uint8_t da = p.destination_address();
    if(da != ctx.self_address && p.control() != modes::bam)
        return result::ignore();

    switch(state_)
    {
        case ANTICIPATING_RTS:
            if(idle().anticipated_address_ != p.source_address())   return result::ignore();
            // FIX: Got a complaint once "Fallthrough" attribute is only allowed on empty statements. Really...
            ATTR_FALLTHROUGH;

        case IDLE:
        {
#if FEATURE_EMBR_J1939_TP_FUTURE == 0
            last_event_ = ctx.current;
#endif

            switch(p.control())
            {
#if FEATURE_EMBR_J1939_TP_RESPONDER
                // NOTE: Won't get here yet due to self_address_ filter
                case modes::bam:
                {
                    state_ = RESPONDER_RECEIVED_BAM;
                    storage_.template emplace<responder_state>(p);
#if FEATURE_EMBR_J1939_TP_FUTURE
                    next_event_ = ctx.current + timeouts::T1;
#endif
                    return result::ok();
                }

                case modes::rts:
                {
                    state_ = RESPONDER_RECEIVED_RTS;
                    storage_.template emplace<responder_state>(p);
#if FEATURE_EMBR_J1939_TP_FUTURE
                    next_event_ = ctx.current + timeouts::T1;
#endif
                    return result::more();
                }
#endif

                // "If a CTS is received while a connection is not established, it shall be ignored."
                case modes::cts:
                    return result::ignore();

                // RTS & BAM and sorta CTS are the only valid message for this to receive when idle
                default:
                    state_ = WARN;
                    break;
            }
            break;
        }

#if FEATURE_EMBR_J1939_TP_ORIGINATOR
        case ORIGINATOR_SENT_DT:
        case ORIGINATOR_SENT_ALL_DT:
        case ORIGINATOR_WAITING_CTS:
#if FEATURE_EMBR_J1939_STRICT_PROTOCOL
            if(originator().pgn_ != p.payload().pgn())
            {
                state_ = ORIGINATOR_ERROR;
                originator().error_ = tp::ORIGINATOR_ERROR_MISMATCHED_PGM;
            }
#endif
            switch(p.control())
            {
                // Handshake stuff, kind of an intermediate ack and occasionally re-requesting
                // already-sent packets
                case modes::cts:
                {
                    if(p.to_send() != originator().last_sequence_ + 1)
                    {
                        // resend/retransmit time
                        // We double duty this pointer as a flag to indicate a retransmit is requested
                        originator().payload_ = nullptr;
                        originator().last_sequence_ = p.to_send().value();
                    }
                    originator().current_packet_per_cts_ = 0;
                    originator().max_packets_per_cts_ = p.max_packets();
                    state_ = ORIGINATOR_RECEIVED_CTS;
                    // Auto payload can immediately send out a DT
                    // Otherwise, external party must load payload to what amounts to SENDING_DT phase
                    // which ORIGINATOR_RECEIVED_CTS currently sorta counts as
                    const bool auto_payload = originator().auto_payload_;
                    return auto_payload ? result::more() : result::ok();
                }

                case modes::ack:
                    // We could check here if we truly sent out everything we wanted to
                    state_ = ORIGINATOR_RECEIVED_EOM_ACK;
#if FEATURE_EMBR_J1939_TP_FUTURE
                    next_event_ = {};
#endif
                    return result::ok();

                case modes::abort:
                    state_ = ORIGINATOR_RECEIVED_ABORT;
#if FEATURE_EMBR_J1939_TP_FUTURE
                    next_event_ = {};
#endif
                    return result::ok();

                default:    break;
            }
            break;

        case ORIGINATOR_SENT_RTS:
            switch(p.control())
            {
                case modes::cts:
                {
                    // DEBT: Probably want to handle timeouts here in addition to
                    // 'outgoing' section
                    state_ = ORIGINATOR_RECEIVED_CTS;
                    const bool auto_payload = originator().auto_payload_;
                    return auto_payload ? result::more() : result::ok();
                }

                default:    break;
            }
            break;
#endif

        default: break;
    }

    return result::ignore();
}

#if FEATURE_EMBR_J1939_TP_RESPONDER
template <class TimePoint, class Policy>
template <class Transport>
auto transport_protocol<TimePoint, Policy>::process_incoming(
    Transport&,
    const pdu<pgns::tp_dt>& p,
    const context& ctx) -> result
{
    bool bam = responder().bam() && role() == ROLE_RESPONDER;

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

        case RESPONDER_SENT_CTS_HOLD:
            // TODO: This is some kind of error condition.  We're in hold state but got a DT anyway
            // Not finding in spec what to do in this case.  I suppose we can go into WARN mode
            // and treat them as lost packets
            state_ = WARN;
#if FEATURE_EMBR_J1939_TP_FUTURE
            next_event_ = {};       // No further events expected
#endif
            break;

        case RESPONDER_RECEIVED_BAM:
        case RESPONDER_RECEIVED_DT:
        case RESPONDER_SENT_CTS:
        {
            const uint8_t seq = p.sequence_number();
            const uint8_t expected_seq = responder().seq() + 1;

            if(seq == expected_seq)
            {
                state_ = RESPONDER_RECEIVING_DT;
                responder().last_dt_ = p.payload();
                ++responder().current_packet_per_cts_;

#if FEATURE_EMBR_J1939_TP_FUTURE
                // If we hit this timeout with RECEIVING_DT, that's a kind of overflow on our side
                // since we didn't empty out payload
                next_event_ = ctx.current + timeouts::T1;
#endif
                // Not 'more' since we expect a pause for consumer to pick up payload
                return result::ok();
            }
            // No out-of-sequence flow control when in BAM mode
            else if(!responder().bam())
            {
                state_ = RESPONDER_SENDING_CTS;
            }

            return result::ok();
        }

        default: break;
    }

    return result::ignore();
}
#endif

template <class TimePoint, class Policy>
template <class Transport>
auto transport_protocol<TimePoint, Policy>::process_outgoing(
    Transport& t,
    const context& ctx) -> result
{
    using traits = transport_traits<Transport>;

    switch(state_)
    {
#if FEATURE_EMBR_J1939_TP_ORIGINATOR
        case ORIGINATOR_SENDING_BAM:
        {
            pdu<pgns::tp_cm> cm{null_t{}};
            const uint16_t& sz = originator().total_size_;

            cm.total_packets((sz + 7) / 7);
            cm.total_size(sz);
            cm.control(pdu<pgns::tp_cm>::bam);
            cm.destination_address(originator().responder_address_);
            cm.source_address(ctx.self_address);
            cm.payload().pgn(originator().pgn_);

            traits::send(t, cm);

            state_ = ORIGINATOR_SENT_BAM;
#if FEATURE_EMBR_J1939_TP_FUTURE
            // A bit of lazy-ish init, could have done this at initiate_originator
            next_event_ = ctx.current + timeouts::bam;
#else
            last_event_ = ctx.current;
#endif
            return true;
        }

#if FEATURE_EMBR_J1939_TP_AUTO_PAYLOAD
        case ORIGINATOR_SENT_BAM:
            if(originator().auto_payload_)
            {
                state_ = ORIGINATOR_SENDING_DT;
                // DEBT: Fallthrough would be more elegant
                //process_outgoing(t, ctx);
                return result::more();
            }
            // else, underflow error
            break;
#endif

        case ORIGINATOR_SENDING_DT:
        {
            pdu<pgns::tp_dt> dt{null_t{}};
            const bool bam = originator().bam();

            // BAM emissions all delay for 50ms
            if(bam && !elapsed(ctx, timeouts::bam))  return false;

            uint8_t& seq = originator().last_sequence_;

            // NOTE: Doing this before increasing seq so that payload size calculates
            // correctly
            estd::copy_n(originator().payload_,
                originator().payload_size(),
                dt.packetized_data());

            // DEBT: Only actually increment this if transport level send succeeds
            ++seq;

            dt.sequence_number(seq);
            dt.source_address(ctx.self_address);
            dt.destination_address(originator().responder_address_);

            ++originator().current_packet_per_cts_;

            traits::send(t, dt);

            state_ = ORIGINATOR_SENT_DT;

#if FEATURE_EMBR_J1939_TP_FUTURE
            // There's a minimum time between DT transmissions
            // DEBT: 25 is arbitrary lower limit below timeout::Tr - needs improvement
            next_event_ = ctx.current + (bam ? timeouts::bam : timeouts::mst{25});
#else
            last_event_ = ctx.current;
#endif
            return result::ok();
        }

        case ORIGINATOR_SENT_DT:
            // For BAM, this means we're done
            // Otherwise, it means wait for EOM ACK
            if(originator().sent_everything())
                state_ = ORIGINATOR_SENT_ALL_DT;
            else if(originator().current_packet_per_cts_ == originator().max_packets_per_cts_)
            {
                state_ = ORIGINATOR_WAITING_CTS;
#if FEATURE_EMBR_J1939_TP_FUTURE
                next_event_ = ctx.current + timeouts::T3;
#endif
            }
#if FEATURE_EMBR_J1939_TP_AUTO_PAYLOAD
            else if(originator().auto_payload_)
            {
                originator().payload_ += 7;
                state_ = ORIGINATOR_SENDING_DT;
                return result::more();
            }
#endif

            // DEBT: This is likely an underflow, reaching here by Tr.  Should we abort?

            return true;

        case ORIGINATOR_SENT_ALL_DT:
            if(originator().bam())
            {
#if FEATURE_EMBR_J1939_TP_FUTURE
                next_event_ = {};
#endif
                state_ = IDLE;
            }
            return true;

        case ORIGINATOR_SENDING_RTS:
        {
            pdu<pgns::tp_cm> cm{null_t{}};
            const uint16_t& sz = originator().total_size_;

            cm.total_packets((sz + 7) / 7);
            cm.total_size(sz);
            cm.control(pdu<pgns::tp_cm>::rts);
            cm.destination_address(originator().responder_address_);
            cm.source_address(ctx.self_address);
            cm.payload().pgn(originator().pgn_);

            traits::send(t, cm);

            state_ = ORIGINATOR_SENT_RTS;
#if FEATURE_EMBR_J1939_TP_FUTURE
            // A bit of lazy-ish init, could have done this at initiate_originator
            next_event_ = ctx.current + timeouts::T3;
#else
            last_event_ = ctx.current;
#endif
            return true;
        }

        // FIX: Need to combine this with ORIGINATOR_WAITING_CTS
        case ORIGINATOR_SENT_RTS:
            // [1] Section 5.12.3
            if(elapsed(ctx, timeouts::T3))
            {
                state_ = ORIGINATOR_TIMEOUT;

                traits::send(t, originator().build_abort(ctx, abort_reasons::timeout));

#if FEATURE_EMBR_J1939_TP_FUTURE
                // No further event processing expected
                next_event_ = time_point{};
#endif
            }
            break;

        case ORIGINATOR_RECEIVED_CTS:
            // If responder asked for a hold, they will need to re-send a CTS
            // to keep us alive.  Otherwise, timeout
            // "a lack of a CTS for more than (T4) seconds after a CTS (0) message to “hold the
            //  connection open” will all cause a connection closure to occur" [1] Section 5.10.2.4
            if(originator().hold_requested() &&
                elapsed(ctx, timeouts::T4))
            {
                state_ = ORIGINATOR_TIMEOUT;

                traits::send(t, originator().build_abort(ctx, abort_reasons::timeout));
                return true;
            }
#if FEATURE_EMBR_J1939_TP_AUTO_PAYLOAD
            else if(originator().auto_payload_)
            {
                state_ = ORIGINATOR_SENDING_DT;
#if FEATURE_EMBR_J1939_TP_FUTURE
                // DEBT: 25 is arbitrary lower limit below timeout::Tr - needs improvement
                next_event_ = ctx.current;
#endif
                return result::ok();
            }
#endif

            break;
#endif
#if FEATURE_EMBR_J1939_TP_RESPONDER
        case RESPONDER_RECEIVED_BAM:
            if(elapsed(ctx, timeouts::T1))
            {
                state_ = RESPONDER_SENDING_ABORT;
                return true;
            }
            break;

        // Got RTS, send CTS
        case RESPONDER_RECEIVED_RTS:
        case RESPONDER_SENDING_CTS:
        {
            pdu<pgns::tp_cm> p{null_t{}};

            responder().prep_cts(p, ctx.self_address);

            p.can_send(responder().max_packets());

            traits::send(t, p);
            state_ = RESPONDER_SENT_CTS;
#if FEATURE_EMBR_J1939_TP_FUTURE
            next_event_ = ctx.current + timeouts::T2;
#endif
            return result::ok();
        }

        case RESPONDER_SENDING_CTS_HOLD:
        {
            pdu<pgns::tp_cm> p{null_t{}};

            responder().prep_cts(p, ctx.self_address);

            p.can_send(0);

            traits::send(t, p);
            state_ = RESPONDER_SENT_CTS_HOLD;
#if FEATURE_EMBR_J1939_TP_FUTURE
            next_event_ = ctx.current + timeouts::Th;
#endif
            return true;
        }

        // Useful to place this here so that external party can pick up RESPONDER_RECEIVING_DT
        // payload.  They will transition to RESPONDER_RECEIVED_DT, then we land here to
        // possibly do further transitions
        case RESPONDER_RECEIVED_DT:
            if(responder().last_one())
            {
                if(responder().bam())
                {
                    state_ = IDLE;
                }
                else
                {
                    pdu<pgns::tp_cm> p = responder().originator_;

                    p.control(modes::ack);
                    p.destination_address(responder().originator_.source_address());
                    p.source_address(ctx.self_address);

                    traits::send(t, p);

                    // DEBT: Should we do a true SENDING_EOM_ACK?
                    state_ = RESPONDER_SENT_EOM_ACK;
                }

#if FEATURE_EMBR_J1939_TP_FUTURE
                next_event_ = {};
#endif
                return result::ok();
            }
            else if(responder().last_one_per_batch())
            {
                responder().retransmit_counter_ = 0;
                state_ = RESPONDER_SENDING_CTS;
                return true;
            }

            // +++ Timeout code
            if(elapsed(ctx, timeouts::T1))
            {
                // TODO: I think we may want to issue another CTS here?
                state_ = RESPONDER_TIMEOUT;

                traits::send(t, responder().build_abort(ctx, abort_reasons::timeout));
#if FEATURE_EMBR_J1939_TP_FUTURE
                next_event_ = {};
#endif
            }
            // ---
            break;

        case RESPONDER_SENT_EOM_ACK:
            state_ = IDLE;
#if FEATURE_EMBR_J1939_TP_FUTURE
            next_event_ = {};
#endif
            break;

        // +++ Timeouts & other time-based activity

        case RESPONDER_SENT_CTS_HOLD:
            if(elapsed(ctx, timeouts::Th))
            {
                // If nothing has happened and we reach Th time, send another hold
                // message (like a keep-alive)
                state_ = RESPONDER_SENDING_CTS_HOLD;
            }
            break;

        case RESPONDER_SENT_CTS:
            // [1] Section 5.12.3
            if(elapsed(ctx, timeouts::T2))
            {
                // NOTE: It is only implied in documentation that a resend of CTS is desired.
                // [1] Section 5.10.2.4 does indicate one MAY choose to abort connection

                // At least one DT is expected.  If we don't get one, try another CTS
                if(++responder().retransmit_counter_ == 3)
                {
                    state_ = ORIGINATOR_TIMEOUT;

                    traits::send(t, responder().build_abort(ctx, abort_reasons::timeout));
#if FEATURE_EMBR_J1939_TP_FUTURE
                    next_event_ = {};
#endif
                }
                else
                    state_ = RESPONDER_SENDING_CTS;

                return result::ok();
            }
            break;
#endif

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

template <class TimePoint, class Policy>
inline void transport_protocol<TimePoint, Policy>::initiate_originator(
    uint16_t sz,
    const context&,
    uint8_t dest_address,
    uint32_t pgn)
{
#if FEATURE_EMBR_J1939_STRICT_STATES
    assert(state_ == IDLE);
#endif

    if(dest_address == addresses::global)
    {
        state_ = ORIGINATOR_SENDING_BAM;
    }
    else
    {
        state_ = ORIGINATOR_SENDING_RTS;
    }

    storage_.template emplace<originator_state>(sz, dest_address, pgn);
}

#if FEATURE_EMBR_J1939_TP_AUTO_PAYLOAD
template <class TimePoint, class Policy>
inline void transport_protocol<TimePoint, Policy>::initiate_originator(
    uint8_t dest_address,
    uint32_t pgn,
    const void* payload,
    uint16_t sz
    )
{
    initiate_originator(dest_address, pgn, sz);
    originator().payload_ = (const uint8_t*)payload;
    originator().auto_payload_ = true;
}
#endif

template <class TimePoint, class Policy>
inline auto transport_protocol<TimePoint, Policy>::next_event() const -> time_point
{
#if FEATURE_EMBR_J1939_TP_FUTURE
    // DEBT: In this case, parent class will do.  Only doing this during transition
    return next_event_;
#else
    switch(state_)
    {
        case ORIGINATOR_SENT_RTS:
            return last_event_ + timeouts::T3;

        case ORIGINATOR_SENT_BAM:
            return last_event_ + timeouts::bam;

        // If not BAM, no waiting
        case ORIGINATOR_SENT_DT:
            return last_event_ + (originator().bam() ? timeouts::bam : timeouts::mst{0});

        case RESPONDER_SENT_CTS_HOLD:
            return last_event_ + timeouts::Th;

        case RESPONDER_SENT_CTS:
            return last_event_ + timeouts::T2;

        case RESPONDER_RECEIVED_DT:
            return last_event_ + timeouts::T1;

        default: return time_point{};
    }
#endif
}

template <class TimePoint, class Policy>
inline void transport_protocol<TimePoint, Policy>::initiate_responder(uint8_t originator_address)
{
#if FEATURE_EMBR_J1939_STRICT_STATES
    assert(state_ == IDLE);
#endif

    storage_.template get<idle_state>()->anticipated_address_ = originator_address;
    state_ = ANTICIPATING_RTS;
}


}}}}

namespace embr { namespace j1939 { namespace sm { namespace tp { inline namespace v0 {

inline void base::request_hold()
{
#if FEATURE_EMBR_J1939_STRICT_STATES
    assert(state_ == RESPONDER_RECEIVED_DT);
#endif

    state_ = RESPONDER_SENDING_CTS_HOLD;
}


// DEBT: A little clumsy.  Might be better to track role explicitly and rework state machine
// into a 2 way sending/receiving/sent, etc and ack, cts, rts, etc.
inline auto base::role() const -> roles
{
    return roles(state_ >> role_shift);
    /*
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
    }   */
}

}}}}}
