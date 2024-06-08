/**
 *
 * Controller Application (CA)
 * No specific CAs are implemented here.  Rather, this is a kind of base class to build them from
 *
 * References:
 *
 * 1. j1939-81 (MAY2003)
 */
#pragma once

#include <estd/variant.h>

// Have to do this here, because otherwise definition (not necessarily instantiation) will lock
// down pdu<> to generic rather than expected specialized varieties
#include "data_field/all.hpp"
#include "cas/internal/dispatcher.hpp"
#include "ca.h"

namespace embr { namespace j1939 {

// DEBT: Although CA is an obvious category for these process_incoming helpers, they have
// a bigger scope than that.  Probably move these out to 'dispatcher' area, and move dispatcher
// itself out of 'ca' area

// NOTE: Impl::context trick is EXPERIMENTAL to help with initializer-list style trivial init
template <class Transport, class Impl, class Context = typename Impl::context>
inline bool process_incoming(Impl& impl, Transport& t, const typename Transport::frame& f, Context& context)
{
    internal::app_state<Transport, Impl, Context> state{t, impl, context};

    return process_incoming(state, f);
}

template <class Transport, class Impl, class Context = typename Impl::context>
inline bool process_incoming(Impl& impl, Transport& t, const typename Transport::frame& f, Context&& context)
{
    internal::app_state<Transport, Impl, const Context> state{t, impl, context};

    return process_incoming(state, f);
}


template <class Transport, class Impl>
inline bool process_incoming(Impl& impl, Transport& t, const typename Transport::frame& f)
{
    internal::app_state<Transport, Impl, estd::monostate> state{t, impl};

    return process_incoming(state, f);
}

template <class Transport, class Impl>
bool controller_application<Transport, Impl>::process_incoming(transport_type& t, const frame_type& f)
{
    return j1939::process_incoming(impl(), t, f);
}

}}
