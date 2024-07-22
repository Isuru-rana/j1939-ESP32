#pragma once

#include "../../state-machines/result.h"

namespace embr { namespace j1939 {

namespace internal {

// DEBT: Naming perhaps should be more of a 'request_state'
template <class Transport, class Impl, class Context>
struct app_state;

}

inline namespace v1 {

template <class Transport, class Impl, class Context>
sm::v1::result process_incoming(internal::app_state<Transport, Impl, Context> state, const typename Transport::frame& f);

}

/*
namespace v2 {

template <class Transport, class Impl, class ...Args>
bool process_incoming(Impl&,
    Transport&&,
    const typename estd::remove_cvref_t<Transport>::frame& f,
    Args&&...);

}
*/

}}
