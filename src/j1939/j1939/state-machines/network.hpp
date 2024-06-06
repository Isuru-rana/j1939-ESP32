#pragma once

#include "network.h"

namespace embr { namespace j1939 { namespace sm { inline namespace v1 {

// true = another call to process_outgoing is requested OR we performed a meaningful task
// false = no further calls to outgoing are requested
template <class Transport, class TimePoint>
bool network_base::process_outgoing(Transport& t, const context<TimePoint>& context)
{
    if(substate != substates::sending) return false;

    switch(state)
    {
        case states::requesting:
            return true;

        case states::claiming:
            switch(substate)
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
        substate = substates::claim_send_error;
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


template <ESTD_CPP_CONCEPT(internal::concepts::AddressManager) AddressManager, class TimePoint>
inline estd::chrono::milliseconds network<AddressManager, TimePoint>::get_send_claim_defer()
{
    uint8_t v = address_manager().rng().get() % 256;
    estd::chrono::duration<uint8_t, estd::ratio<6, 10000>> d(v);
    return d;
}


template <ESTD_CPP_CONCEPT(internal::concepts::AddressManager) AddressManager, class TimePoint>
template <class Transport>
bool network<AddressManager, TimePoint>::evaluate_contenders(Transport& t, const pdu<pgns::address_claimed>& p)
{
    switch(state)
    {
        // Incoming address claim after we've settled on our SA.  Evaluate whether
        // we can/should give it up
        case states::claimed:
            if(is_contender(p))
                evaluate_contender(t, p);
            break;

            // Incoming address claim while we're trying to claim SA.  Could be someone
            // specifically contending with our claim
        case states::claiming:
            if(is_contender(p))
                evaluate_contender(t, p);
            break;

            // Incoming address claims after we do a request for address claim is expected.
            // Contention possibility is still present.
        case states::requesting:
            if(is_contender(p))
                evaluate_contender(t, p);

            track(p);

            break;

            // If unstarted, we must ignore things until we DO start/init
            // If claim_failed, we've given up trying to get on network
        case states::unstarted:
        case states::claim_failed:
            return false;
    }

    return true;
}



}}}}