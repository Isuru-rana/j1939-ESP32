#pragma once

#include "network.h"
#include "base.hpp"

namespace embr { namespace j1939 { namespace sm { inline namespace v1 {

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
    switch(state_)
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


template <ESTD_CPP_CONCEPT(internal::concepts::AddressManager) AddressManager, class TimePoint>
template <class Transport>
bool network<AddressManager, TimePoint>::scheduled_claiming(Transport& t, time_point* wake, time_point current)
{
    switch(substate_)
    {
        case substates::bus_off:
            // TODO: "Delaying before Address Re-Claim"
            // this is a preceding addition to the regular 250ms next_event_
            substate_ = substates::bus_off_recover;
            // DEBT: Arbitrary delay here, need something way more specific
            *wake += estd::chrono::milliseconds(500);
            break;

        case substates::bus_off_recover:
            send_claim(t);

            if(skip_timeout())
            {
                state(states::claimed, substates::elapsed);
            }
            else
            {
                substate_ = substates::waiting;
                next_event_ = current + address_claim_timeout();
                *wake += address_claim_timeout();
                return true;
            }

            /*
            next_event_ = current + address_claim_timeout();

            substate_ = substates::waiting;
            if(!schedule_address_claim_timeout(wake)) // set up 250ms next_event_
                // don't wait for scheduling, immediately go to 'waiting' finish portion
                return scheduled_claiming(t, wake, current);    */
            break;

        case substates::contending:
            break;

        case substates::request_waiting:
            // got to 1.25s next_event_ for request for address claim
            // [1] Figure A5, A6, A7
            // 1.TODO: For A5, if noone contends, send out claim for X.  Then, wait 250ms for contention
            // 2.TODO: For A5, if someone contends, flow to A6
            // 3.TODO: For A6, use some algorithm to aggregate incoming claimed and
            //       select W and send out claim for W.  Then, wait 250ms for contention
            // 4.TODO: For A7, emit cannot claim address (single address CA)

            // Handles 1 and partial 3
            send_claim(t);
            if(skip_timeout())
            {
                state(states::claimed, substates::elapsed);
                return false;
            }

            next_event_ = current + address_claim_timeout();
            substate_ = substates::waiting;
            *wake += address_claim_timeout();

            // DEBT: Getting here we sorta presume we're arbitrary capable, meaning
            // we always do 250ms wait
            //schedule_address_claim_timeout(wake);
            return true;

        // Waiting to finish our own claim address phase
        case substates::waiting:
            if(current >= next_event_)
            {
                // got to timeout/next_event_ without contention means successful claim
                state(states::claimed, substates::elapsed);
            }
            else
            {
                // next_event_ can get adjusted when contenders come in, since
                // we sometimes need to re-emit and therefore re-start claim
                // process as per [3] 3.3.3.1
                // this effectively elongates the 'waiting' period so we reschedule
                *wake = next_event_;
                return true;
            }
            break;

        // While waiting 250ms after address claim, a bus/send error occurred.
        // We intentionally reach here at the 250ms expiry
        case substates::claim_send_error:
            next_event_ += get_send_claim_defer();
            *wake = next_event_;
            substate_ = substates::reclaim_waiting;
            return true;

        // Reach here after deferred waiting period for retransmission of claim
        // Manually retry as per [3] 1.1.4
        case substates::reclaim_waiting:
            // Optimistically go back to 'waiting', presuming a good bus awaits us
            substate_ = substates::waiting;
            send_claim(t);
            next_event_ = current + address_claim_timeout();
            *wake = next_event_;
            return true;


        case substates::cannot_claim_waiting:
            break;

        default:
            break;
    }

    return false;
}

template <ESTD_CPP_CONCEPT(internal::concepts::AddressManager) AddressManager, class TimePoint>
template <class Transport>
void network<AddressManager, TimePoint>::start(Transport& t, time_point current)
{
    address_ = address_manager().get_candidate();

    // DEBT: Not 100% right, more like we have an alleged address and
    // the send_claim is to ensure there's no contention
    if(has_address())
    {
        state_ = states::claiming;
        substate_ = substates::sending;      // Dormant substate at the moment
        send_claim(t);
        next_event_ = current + address_claim_timeout();
        substate_ = substates::waiting;
    }
    else
    {
        // TODO: Likely we need to instead do this during the "cannot claim" process
        state_ = states::requesting;
        substate_ = substates::request_waiting;
        send_request_for_address_claimed(t, address_traits::global);
        next_event_ = current + request_for_address_claim_timeout();
    }
}

template <ESTD_CPP_CONCEPT(internal::concepts::AddressManager) AddressManager, class TimePoint>
bool network<AddressManager, TimePoint>::contended()
{
    return {};
}

}}}}