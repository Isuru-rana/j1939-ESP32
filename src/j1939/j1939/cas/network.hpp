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
#include "../state-machines/network/network.hpp"

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
    const time_point current = scheduler.impl().now();
    time_point wake;

    bool do_schedule = false;

    bool r = nca_base_type::process_incoming_internal(t, p, &wake, current, &do_schedule);

    if(do_schedule)
        scheduler.schedule(next_event_, &wake_model);

    return r;
}

template <class TTransport, class TScheduler, class TAddressManager>
void network_ca<TTransport, TScheduler, TAddressManager>::start(transport_type& t)
{
    this->t = &t;

    const time_point current = scheduler.impl().now();

    nca_base_type::start(t, current);

    scheduler.schedule(next_event_, &wake_model);
}

}

}}
