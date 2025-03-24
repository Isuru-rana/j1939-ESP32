#pragma once

#include <estd/internal/fwd/tuple.h>

#include "../pdu/fwd.h"
#include "../cs/base.h"

namespace embr { namespace j1939 { namespace cs { namespace internal { namespace v1 {

template <class Transport>
struct incoming_visitor
{
    Transport& transport;

    using result = cs::v1::base::result;

    // DEBT: Emit result
    // DEBT: Use c++20 concept on 'CS'

    // NOTE: Used by aggregate CA code, which uses type_visitor::visit directly.
    // That in turn only uses variadic::type, not variadic::instance thus our
    // usage of it here.
    // DEBT: Upgrade that to use the visit_instance or similar which is baked into
    // 'tuple' and indeed the tuple 'visit' itself probably should default to that,
    // which may be more DEBT

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

    // v2-ish
    template <size_t I, class CS, pgns pgn, class ...Args>
    bool operator()(
        estd::variadic::instance<I, CS> cs,
        const pdu<pgn>& p,
        Args&&...args) const
    {
        cs.value.process_incoming(transport, p, std::forward<Args>(args)...);

        return false;
    }

    // v2-ish
    template <size_t I, class CS, class ...Args>
    bool operator()(
        estd::variadic::instance<I, CS> cs,
        const typename Transport::frame& frame,
        Args&&...args) const
    {
        cs.value.process_incoming_default(transport, frame, std::forward<Args>(args)...);

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
