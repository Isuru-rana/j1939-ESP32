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
#include "internal/dispatcher/incoming.hpp"
#include "internal/dispatcher/incoming.h"
#include "internal/dispatcher/incoming2.hpp"
#include "ca.h"

namespace embr { namespace j1939 {

template <class Transport, class Impl>
auto controller_application<Transport, Impl>::process_incoming(transport_type& t, const frame_type& f) -> result
{
    return j1939::v2::process_incoming(impl(), t, f);
}

}}
