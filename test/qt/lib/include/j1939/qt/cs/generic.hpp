#pragma once

#include <j1939/cs/base.h>
#include <j1939/pgn/traits.h>  // For traits_wrapper

#include "generic.h"

namespace embr::j1939::qt::cs { inline namespace v1 {

template <pgns pgn>
bool Generic::process_incoming(can::qt_transport&, const pdu<pgn>& p)
{
    using traits = j1939::internal::traits_wrapper<pgn>;

    if constexpr(traits::specialized == false)
        return false;

    DataField df(this);

    df.populate(p);

    emit pduReceived(df);

    return true;
}

}}
