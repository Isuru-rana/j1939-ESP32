#pragma once

#include <estd/variant.h>   // DEBT: Just for monostate

#include "fwd.h"

namespace embr { namespace j1939 {

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
    internal::app_state<Transport, Impl, const Context> state{t, impl, std::forward<Context>(context)};

    return process_incoming(state, f);
}


template <class Transport, class Impl>
inline bool process_incoming(Impl& impl, Transport& t, const typename Transport::frame& f)
{
    internal::app_state<Transport, Impl, estd::monostate> state{t, impl};

    return process_incoming(state, f);
}


}}
