#pragma once

namespace embr { namespace j1939 {

namespace internal {

template <class Transport, class Impl, class Context>
struct app_state
{
    Transport& t;
    Impl& impl;
    Context& context;

    template <pgns pgn>
    constexpr bool invoker(uint32_t id, const uint8_t* payload) const
    {
        return impl.process_incoming(t, pdu<pgn>(id, payload), context);
    }
};


template <class Transport, class Impl>
struct app_state<Transport, Impl, estd::monostate>
{
    Transport& t;
    Impl& impl;

    template <pgns pgn>
    constexpr bool invoker(uint32_t id, const uint8_t* payload) const
    {
        return impl.process_incoming(t, pdu<pgn>(id, payload));
    }
};


}

}}
