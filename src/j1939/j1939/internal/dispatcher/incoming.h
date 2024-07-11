#pragma once

#include <estd/variant.h>   // DEBT: Just for monostate

#include "fwd.h"

// NOTE: Deprecated, v2 process_incoming now preferred.  Be very careful,
// v1 will take precedence over v2 (I think) due to greedy Context consumption.
// What I know for sure is v1 and v2 do NOT collide, even though I expected them to

namespace embr { namespace j1939 { inline namespace v1 {

// NOTE: Impl::context trick is EXPERIMENTAL to help with initializer-list style trivial init
template <class Transport, class Impl, class Context = typename Impl::context>
inline sm::v1::result process_incoming(Impl& impl, Transport& t, const typename Transport::frame& f, Context& context)
{
    internal::app_state<Transport, Impl, Context> state{t, impl, context};

    return process_incoming(state, f);
}

template <class Transport, class Impl, class Context = typename Impl::context>
inline sm::v1::result process_incoming(Impl& impl, Transport& t, const typename Transport::frame& f, Context&& context)
{
    internal::app_state<Transport, Impl, const Context> state{t, impl, std::forward<Context>(context)};

    return process_incoming(state, f);
}


template <class Transport, class Impl>
inline sm::v1::result process_incoming(Impl& impl, Transport& t, const typename Transport::frame& f)
{
    internal::app_state<Transport, Impl, estd::monostate> state{t, impl};

    return process_incoming(state, f);
}

// FIX: Not ready yet.  Eventually all will be Transport&&
template <class Transport, class Impl>
inline sm::v1::result process_incoming(Impl& impl, Transport&& t, const typename Transport::frame& f)
{
    internal::app_state<Transport, Impl, estd::monostate> state{std::forward<Transport>(t), impl};

    return process_incoming(state, f);
}

}}}
