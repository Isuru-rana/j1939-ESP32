/**
 *
 * References:
 *
 * 1. J1939-81 (draft MAY2003)
 * 2. J1939-21 (DEC2006)
 * 3. AddressResolution.md v0.1
 */
#pragma once

#ifdef ESP_PLATFORM
#include <esp_log.h>
#endif

#include "network.h"

#include "../data_field/request.hpp"
#include "../state-machines/network.hpp"

namespace embr { namespace j1939 {

namespace impl {


template <class TTransport, class TScheduler, class TAddressManager>
void network_ca<TTransport, TScheduler, TAddressManager>::scheduled_claiming(
    time_point* wake, time_point current)
{
    // Currently just a NOOP
    using type = typename nca_base_type::template context<time_point>;
    const type context(current, *address_);
    nca_base_type::process_outgoing_internal(*t, context);

    nca_base_type::scheduled_claiming(*t, wake, current);
}

template <class TTransport, class TScheduler, class TAddressManager>
bool network_ca<TTransport, TScheduler, TAddressManager>::process_incoming(
    transport_type& t,
    const pdu<pgns::address_claimed>& p)
{
    pdu<pgns::address_claimed> p_resp{null_t{}};

    const uint8_t sa = p.can_id().source_address();
    // we expect all address_claimed messages to be BAM
    //uint8_t da = p.can_id().destination_address();

#ifdef ESP_PLATFORM
    // Don't go too bananas, since we already have a diagnostic_ca
    ESP_LOGV(TAG, "processing AC with SA:%X", sa);
#endif

    bool result = nca_base_type::evaluate_contenders(t, p);

    if(result == false) return false;

    // Is our address in contest? [1] 4.4.3.3
    if(sa == address_)
    {
        const embr::j1939::layer1::NAME& incoming_name = p.payload();

        // DEBT: a C#-style compare returning an int would be useful here,
        // or the c++ two way compare <=>
        if(name_ < incoming_name)
        {
            // we have the higher priority name
            // transmit our own address, basically re-announce our claim
            nca_base_type::send_claim(t);

            // TODO:
            // If we're currently claiming, this extends the 250ms next_event_
            // If we're fully claimed, this has no followup scheduled
            if(state_ == states::claiming)  {}

            // DEBT: Do we need to schedule a followup here?
        }
        else if(name_ > incoming_name)
        {
            // we have the lower priority name, we're contended.  Give up our SA

            // Incoming address remembered so that we avoid RNG attempts at it
            address_manager().encountered(sa);

            // Dormant
            nca_base_type::contended();

            // [1] 4.4.4
            // emit a 'cannot claim' or attempt to claim a new address
            address_type new_address = find_new_address();

            if(new_address.has_value())
            {
                // DEBT: Account for 'requesting' state in which case we probably
                // shouldn't respond right away but probably should still find_new_address
                // DEBT: Do this output with a state machine.  Arguably cleaner to service in process_outgoing,
                // and also gives us chance to decouple this whole function from scheduler
                resend_claim_and_reschedule(t, *new_address);

                // DEBT: Account for other states here also
                if(state_ == states::claimed)
                {
                    // DEBT: Assigning state & substate at once is reasonable but clumsy
                    // and easy to get wrong
                    state_ = states::claiming;
                    substate_ = substates::waiting;
                }

                // We optimistically assign ourselves this new address, expecting someone
                // will contend us necessary
                address_ = new_address;
            }
            else
            {
                nca_base_type::send_cannot_claim(t);
                state_ = states::claim_failed;
            }
        }
        else
        {
            // equals, which is not a covered scenario that I know of
            // See [3] 1.1.1 and 1.1.1.2
            // That said, we MAY encounter this when responding to our own request for address
            // as per [3] 1.2.1.1
        }
    }
    else
    {
        // Address not matching our own encountered, take note
        // NOTE: Strongly consider filtering this so that only incoming NAMEs which don't
        // contend us get registered
        address_manager().encountered(sa);
    }

    return false;
}

template <class TTransport, class TScheduler, class TAddressManager>
bool network_ca<TTransport, TScheduler, TAddressManager>::process_incoming(transport_type& t, const pdu<pgns::request>& p)
{
    switch((pgns)p.payload().pgn())
    {
        // [1] 4.2.1, 4.4.3 - request for address claimed
        case pgns::address_claimed:
            nca_base_type::process_request_for_address_claimed(t, p);
            return false;

        default:
            return false;
    }
}

template <class TTransport, class TScheduler, class TAddressManager>
void network_ca<TTransport, TScheduler, TAddressManager>::start(transport_type& t)
{
    this->t = &t;

    const time_point current = scheduler.impl().now();

    nca_base_type::start(t, current);

    scheduler.schedule(next_event_, &wake_model);
}

template <class TTransport, class TScheduler, class TAddressManager>
void network_ca<TTransport, TScheduler, TAddressManager>::resend_claim_and_reschedule(
    transport_type& t, uint8_t sa)
{
    switch(state_)
    {
        // Use case here is we've comfortably sat on an address past negotiation phase,
        // and a newcomer has arrived contending us.  Scheduler has spooled out
        case states::claimed:
            nca_base_type::send_claim(t, sa);
            next_event_ = scheduler.impl().now() + nca_base_type::address_claim_timeout();
            scheduler.schedule(next_event_, &wake_model);
            break;

        // Use case here is we're underway performing a claim and someone has contended.
        // In this case our scheduled item is still active
        case states::claiming:
            nca_base_type::send_claim(t, sa);
            next_event_ = scheduler.impl().now() + nca_base_type::address_claim_timeout();
            // this implicitly reschedules by virtue of adjusting 'next_event_'
            break;

        default:
            // unstarted = no actions
            // cannot_claim = no point in attempting
            // requesting = undefined behavior (address claim WITHOUT followup appropriate here [3] 1.2.1.1. )
            // claim_send_error, reclaim_waiting = no action because whole different process emitting its own claims
            break;
    }
}


}

}}
