#pragma once

#include "base.h"

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
                    context.next(estd::chrono::milliseconds(500));
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
void network_base::send_claim(Transport& t, pdu<pgns::address_claimed>& p, uint8_t sa)
{
    using traits = transport_traits<Transport>;
    // DEBT: Not sure if claim ALWAYS is a BAM but I think so
    p.can_id().destination_address(address_traits::global);
    p.payload() = name_;
    p.can_id().source_address(sa);

    // Turn off CAN transport auto retry as per [1] 4.4.4.3
#if FEATURE_EMBR_J1939_AC_COLLISION_MANAGEMENT
    t.one_shot(true);
#endif
    bool send_result = traits::send(t, p);
#if FEATURE_EMBR_J1939_AC_COLLISION_MANAGEMENT
    t.one_shot(false);
#endif

#if FEATURE_EMBR_J1939_AC_COLLISION_MANAGEMENT
    // DEBT: Do this pseudo asynchronously, since 'send' may not register an error
    // right away
    if(!t.good() || !send_result)
    {
        // If a bus error, schedule our own retry after "idle" 250ms
        // as per [1] 4.4.4.3 and [3] 1.1.4.
        // As per [3] 1.1.4.1 - "idle" MIGHT mean CAN idle - that will require a code change
        substate_ = substates::claim_send_error;
    }
#endif
}


// DEBT: No test seems to exercise this yet
template <class Transport>
void network_base::send_request_for_address_claimed(Transport& t, uint8_t da)
{
    using traits = transport_traits<Transport>;

    pdu<pgns::request> p(address_traits::null, da);

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

    if (!(da == address_traits::global ||
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

}}}}
