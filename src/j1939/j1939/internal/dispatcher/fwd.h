#pragma once

namespace embr { namespace j1939 {

namespace internal {

// DEBT: Naming perhaps should be more of a 'request_state'
template <class Transport, class Impl, class Context>
struct app_state;

}

template <class Transport, class Impl, class Context>
bool process_incoming(internal::app_state<Transport, Impl, Context> state, const typename Transport::frame& f);

}}
