#pragma once

#include "../pdu/fwd.h"
#include "../pgn/enum.h"
#include "../spn/enum.h"

namespace embr { namespace j1939 { namespace cs { inline namespace v1 {

// Adapted from old controller_application_base
class base
{
protected:
    using pgns = embr::j1939::pgns;
    using spns = embr::j1939::spns;

    template <pgns p>
    using pdu = embr::j1939::pdu<p>;

public:
    // DEBT: Need a Context variety of this one too
    // Undefined/unhandled CAN frame
    template <class Transport, class Frame>
    static constexpr bool process_incoming_default(const Transport&, const Frame&)
    {
        return false;
    }

    template <class Transport, pgns pgn>
    constexpr bool process_incoming(const Transport&, pdu<pgn>) const { return false; }

    template <class Transport, pgns pgn, class Context>
    constexpr bool process_incoming(Transport&, pdu<pgn>, Context) const { return false; }

    template <class Transport, class Context>
    constexpr bool process_outgoing(Transport&, Context = {}) const { return false; }
};

}}}}
