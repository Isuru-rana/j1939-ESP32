#pragma once

#include "../pdu/fwd.h"
#include "../pgn/enum.h"
#include "../spn/enum.h"

#include "../internal/dispatcher/policy.h"
#include "../data_field/fwd.h"

// Experimenting with more robust state machine/process return value (ala embr::coap)
#define FEATURE_EMBR_J1939_CS_ADV_RESULT 1

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

#if FEATURE_EMBR_J1939_CS_ADV_RESULT == 1
    // EXPERIMENTAL
    struct result
    {
        // indicates an internal state change or transport interaction
        // in other words, a rough measurement of whether an action was taken
        bool processed : 1;
        // indicates an immediate additional call is requested
        // (often used to give caller chance to notice interesting states)
        // when used with process_incoming, indicates a process_outgoing is requested
        bool immediate : 1;
        // indicates state machine has reached the end of its cycle and will
        // return to IDLE (or equivelant).  Note that multiples of these may
        // appear if 'immediate' is set, signaling a potentially elongated shutdown
        bool end : 1;

        result(const result&) = default;

        constexpr result(bool processed, bool immediate = false) :
            processed{processed},
            immediate{immediate},
            end{false}
        {

        }

        // DEBT: Only for legacy compatibility, eliminate or rework this once we fully
        // transition to 'result' awareness
        constexpr operator bool() const { return processed; }

        static constexpr result more() { return result{true, true}; }
        static constexpr result ok() { return result{true, false}; }
        static constexpr result ignore() { return result{false, false}; }
    };
#else
    using result = bool;
#endif

    using policy_type = j1939::internal::dispatch_default_policy;

    // Undefined/unhandled CAN frame
    template <class Transport, class Frame, class ...Args>
    static constexpr result process_incoming_default(const Transport&, const Frame&, Args&&...)
    {
        return false;
    }

    // DEBT: Would like ...Args treatment, but compiler gets ornery about overload ambiguities
    template <class Transport, pgns pgn>
    constexpr result process_incoming(const Transport&, pdu<pgn>) const { return false; }

    template <class Transport, pgns pgn, class Context>
    constexpr result process_incoming(Transport&, pdu<pgn>, Context) const { return false; }

    template <class Transport, class Context>
    constexpr result process_outgoing(Transport&, Context = {}) const { return false; }
};

}}}}
