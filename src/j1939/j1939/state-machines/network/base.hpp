#pragma once

#include "base.h"
#include "../../addresses.h"

namespace embr { namespace j1939 { namespace sm { inline namespace v1 {

// DORMANT
// 'internal' flavor does not check next_event_ or substate == sending
// true = another call to process_outgoing is requested OR we performed a meaningful task
// false = no further calls to outgoing are requested
template <class Transport, class TimePoint>
bool network_base::process_outgoing_internal(Transport& t, const context<TimePoint>& context)
{
    switch(state_)
    {
        case states::requesting:
            return true;

        case states::claiming:
            switch(substate_)
            {
                case substates::bus_off:
                    // DEBT: Arbitrary delay here, need something way more specific
                    //context.next(estd::chrono::milliseconds(500));
                    return false;

                case substates::bus_off_recover:
                    //send_claim(t);
                    //substate = substates::waiting;
                    return true;

                default: return true;
            }
            //send_claim(t);

        default:
            return false;
    }
}


template <class Transport>
void network_base::send_claim(Transport& t, uint8_t sa)
{
    // DEBT: Not sure if claim ALWAYS is a BAM but I think so

    pdu<pgns::address_claimed> p(sa, addresses::global, name_);

    using traits = transport_traits<Transport>;
    //p.can_id().destination_address(address_traits::global);
    //p.payload() = name_;
    //p.can_id().source_address(sa);

    // Turn off CAN transport auto retry as per [1] 4.4.4.3
#if FEATURE_EMBR_J1939_AC_COLLISION_MANAGEMENT
    t.one_shot(true);
#endif
    bool send_result = traits::send(t, p);
#if FEATURE_EMBR_J1939_AC_COLLISION_MANAGEMENT
    t.one_shot(false);
#endif

    // TODO: Being this is a state machine, we're more tolerant of side effects.
    // Consider setting up waiting/elapsed substate here

#if FEATURE_EMBR_J1939_AC_COLLISION_MANAGEMENT
    // FIX: Most of the time this substate gets blown away with 'waiting' or 'elapsed'
    // DEBT: Do this pseudo asynchronously, since 'send' may not register an error
    // right away
    // FIX: Intentionally left glitchy since [1] is unclear, we need non-draft version
    if(!t.good() || !send_result)
    {
        // If a bus error, schedule our own retry after "idle" 250ms
        // as per [1] 4.4.4.3 and [3] 1.1.4.
        // As per [3] 1.1.4.1 - "idle" MIGHT mean CAN idle - that will require a code change
        state(states::claiming, substates::claim_send_error);
    }
#endif
}


// DEBT: No test seems to exercise this yet
template <class Transport>
void network_base::send_request_for_address_claimed(Transport& t, uint8_t da)
{
    using traits = transport_traits<Transport>;

    pdu<pgns::request> p(addresses::null, da);

    // DEBT: make this pgn param take enum
    p.payload().pgn((uint32_t)pgns::address_claimed);

    traits::send(t, p);
}

inline void network_base::start()
{

}


template <class Transport>
bool network_base::process_request_for_address_claimed(
    Transport& t, const pdu<pgns::request>& p)
{
    // TODO: Verify source address is null and log if it isn't
    //const address_type sa = p.can_id().source_address();

    const uint8_t da = p.can_id().destination_address();

    if (!(da == addresses::global ||
          da == address_))
        return false;

    switch(state_)
    {
        case states::claiming:
            // Ignore request if we haven't completed our claim address process yet
            //send_claim(t);
            break;

        case states::claimed:
            // Send as basically an ACK, so no followup scheduling for this particular
            // send_claim
            send_claim(t);
            break;

        case states::claim_failed:
            // [1] 4.4.3.1
            // emit a 'cannot claim'
            send_cannot_claim(t);
            break;

            // If we ourselves are waiting on a response to our own request for claim,
            // or we just haven't started/initialized yet, ignore the request for claimed address
        case states::requesting:
        case states::unstarted:
            return false;
    }

    return true;
}


template <class Transport>
bool network_base::process_incoming(Transport& t, const pdu<pgns::request>& p)
{
    switch((pgns)p.payload().pgn())
    {
        // [1] 4.2.1, 4.4.3 - request for address claimed
        case pgns::address_claimed:
            return process_request_for_address_claimed(t, p);

        default:
            return false;
    }
}


}}}}
