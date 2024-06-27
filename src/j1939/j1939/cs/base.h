#pragma once

#include "../pdu/fwd.h"
#include "../pgn/enum.h"
#include "../spn/enum.h"

#include "../internal/dispatcher/policy.h"
#include "../data_field/fwd.h"


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
    // DEBT: We actually want layer2::data_field here but that one needs work
    // to get the constructors online
    // DEBT: No longer liking 'const' here, remove that
    template <pgns pgn>
    using data_field = const embr::j1939::layer1::data_field<pgn>;

    using policy_type = j1939::internal::dispatch_default_policy;

    // Undefined/unhandled CAN frame
    template <class Transport, class Frame, class ...Args>
    static constexpr bool process_incoming_default(const Transport&, const Frame&, Args&&...)
    {
        return false;
    }

    // DEBT: Would like ...Args treatment, but compiler gets ornery about overload ambiguities
    template <class Transport, pgns pgn>
    constexpr bool process_incoming(const Transport&, pdu<pgn>) const { return false; }

    template <class Transport, pgns pgn, class Context>
    constexpr bool process_incoming(Transport&, pdu<pgn>, Context) const { return false; }

    template <class Transport, class Context>
    constexpr bool process_outgoing(Transport&, Context = {}) const { return false; }
};

}}}}
