#pragma once

#include <j1939/cs/base.h>
#include <j1939/pgn/traits.h>  // For traits_wrapper

#include "generic.h"
#include "../pdu.h"

namespace embr::j1939::qt::cs { inline namespace v1 {

// DEBT: This probably can be moved to .cpp file since the only consumer of it lives there
// DEBT: Emit messages even if they aren't specialized.  Wait for final throws of NO_TRAITS_WRAPPER
// to settle down (it's nearly there)
template <pgns pgn>
auto Generic::process_incoming(can::qt_transport&, const pdu<pgn>& p) -> result
{
    using traits = j1939::pgn::traits<pgn>;

    if constexpr(traits::is_specialized == false)
        return false;

    auto p2 = new Pdu(p.can_id(), this);

    if constexpr(
        pgn == pgns::address_claimed ||
        pgn == pgns::commanded_address)
    {
        // DEBT: A bit clumsy since NAME is parent of j1939::data_field,
        p2->data_field().populate_name(p);
    }
    else
    {
        p2->data_field().populate(p);
        //DataField df(this);

        //df.populate(p);

    }

    emit pduReceived(p2);

    return result::ok();
}

}}
