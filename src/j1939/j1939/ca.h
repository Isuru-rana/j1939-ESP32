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

#include <estd/tuple.h>
#include <estd/internal/variadic.h>

#include <can/transport.h>

#include "data_field/fwd.h"
#include "pdu.h"
#include "pgn.h"

#include "cs/base.h"
#include "cs/aggregate.h"

#include "internal/dispatcher/dispatch.h"    // EXPERIMENTAL, just for policy

namespace embr { namespace j1939 {

namespace impl {

// NOTE: Was going to Dogfood improved cs::v1::base back into original ca_base,
// however ambiguous base issues make that a pain.  Rather, phase out use
// of this base unless really needed - and if so, multiple inhereit this *and*
// cs::v1::base
class controller_application_base
{
public:
    // EXPERIMENTAL, inactive for process_incoming assisted filtering of incoming DA
    enum address_filter
    {
        FILTER_NONE,        // We desire all traffic
        FILTER_BAM,         // We desire broadcast and directed (to our DA)
        FILTER_DA,          // We desire only traffic to this DA specifically
    };
};

/// This is a noop reference type.  Only useful for testing and learning
/// how to make a different impl
/// @tparam TTransport
template <class TTransport>
class controller_application : public controller_application_base
{
protected:
    typedef TTransport transport_type;
    using frame_type = typename transport_type::frame;
    //using endpoint_type = typename transport_type::endpoint_type;
    //using message_type = typename transport_type::message_type;

public:

    //template <pgns pgn>
    //constexpr bool process_incoming(transport_type&, pdu<pgn>) const { return false; }
};

// DEBT: Move aggregator to its own .h/.hpp file
template <class ...TCAs>
class controller_application_aggregator
{
    using tuple = estd::tuple<TCAs...>;

// DEBT
//#if UNIT_TESTING
public:
//#endif
    tuple child_cas;

    template <class Transport>
    using visitor = cs::internal::v1::incoming_visitor<Transport>;

    template <class TTransport, class ...TArgs>
    bool apply(TTransport& transport, TArgs&&...args)
    {
        // DEBT: Pretty sure there's a tuple specific overload of all this
        // and if there isn't, make one.  Specifically, (e)std::apply
        // https://en.cppreference.com/w/cpp/utility/apply.
        // Note though that stock apply doesn't allow for additional
        // parameters - think we might want to make estd::apply do so
        // Note also that existing 'apply' uses a std::move but should
        // use a std::forward
        estd::variadic::type_visitor<TCAs...>::visit(
            visitor<TTransport>{transport}, child_cas,
            std::forward<TArgs>(args)...);

        // DEBT: Need to |= results together, though not 100% sure
        // I like that overall paradigm either
        return false;
    }

public:
    template <class TTransport, pgns pgn>
    bool process_incoming(TTransport& transport, const pdu<pgn>& p)
    {
        return apply(transport, p);
    }

    // Effectively undefined/unhandled CAN frame.  Otherwise, you'll want to add to the switch/data_field mapper
    template <class TTransport>
    bool process_incoming_default(TTransport& transport,
        const typename TTransport::frame& frame)
    {
        return apply(transport, frame);
    }

    controller_application_aggregator() = default;

    template <class ...TArgs>
    constexpr explicit controller_application_aggregator(TArgs&&...args) :
        child_cas(std::forward<TArgs>(args)...)
    {

    }
};

}

// DEBT: Strongly consider grabbing transport_type from TImpl rather than
// specifying here
template <class TTransport, class TImpl = impl::controller_application<TTransport> >
class controller_application : public TImpl
{
    typedef TImpl impl_type;

    impl_type& impl() { return *this; }
    const impl_type& impl() const { return *this; }

public:
    typedef TTransport transport_type;
    using frame_type = typename transport_type::frame;

    using frame_traits = can::frame_traits<frame_type>;

    bool process_incoming(transport_type& t, const frame_type& f);

    template <class ...TArgs>
    controller_application(TArgs&&...args) : TImpl(std::forward<TArgs>(args)...)
    {

    }
};

namespace experimental {

template <class ...TArgs>
using tuple_init = estd::tuple<TArgs...>;

}


}}
