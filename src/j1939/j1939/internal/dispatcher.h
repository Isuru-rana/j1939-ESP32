#pragma once

// NOTE: Not yet active

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
    internal::app_state<Transport, Impl, const Context> state{t, impl, context};

    return process_incoming(state, f);
}

}}
