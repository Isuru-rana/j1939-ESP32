#pragma once

#include <estd/internal/fwd/tuple.h>

#include "../pdu/fwd.h"

namespace embr { namespace j1939 { namespace cs { namespace internal { namespace v1 {

template <class Transport>
struct incoming_visitor
{
    Transport& transport;

    // DEBT: Add bool aggregated result
    // DEBT: Use c++20 concept on 'CS'
    // DEBT: Use estd::variadic::instance if we can, document why if we can't

    template <size_t I, class CS, pgns pgn, class ...CSs>
    bool operator()(estd::variadic::type<I, CS>, estd::tuple<CSs...>& ccas,
        const pdu<pgn>& p) const
    {
        CS& ca = estd::get<I>(ccas);

        ca.process_incoming(transport, p);

        return false;
    }

    template <size_t I, class CS, class ...CSs>
    bool operator()(estd::variadic::type<I, CS>, estd::tuple<CSs...>& ccas,
        const typename Transport::frame& frame) const
    {
        CS& ca = estd::get<I>(ccas);

        ca.process_incoming_default(transport, frame);

        return false;
    }
};

template <class Transport>
struct outgoing_visitor
{
    Transport& transport_;

    template <size_t I, class CS, pgns pgn, class Context>
    bool operator()(estd::variadic::instance<I, CS> cs, const pdu<pgn>& p,
        Context& context) const
    {
        cs.value.process_outgoing(transport_, context);
        
        return false;
    }
};



}}}}}