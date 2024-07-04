#pragma once

// DEBT: Consider putting this into embr proper.  Look into embr::coap state machine support for consolidation
// ideas

// entire file is
// EXPERIMENTAL

namespace embr { namespace j1939 { namespace sm { inline namespace v0 {

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


class base
{
public:
    template <class ...Args>
    static constexpr result process(Args&&...)
    {
        return result::ignore();
    }
};

}}}}
