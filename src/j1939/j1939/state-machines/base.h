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

    // EXPERIMENTAL, not yet used
    // indicates state machine has reached the end of its cycle and will
    // return to IDLE (or equivelant).  Note that multiples of these may
    // appear if 'immediate' is set, signaling a potentially elongated shutdown
    bool end : 1;

    enum codes
    {
        CODE_OK = 0,
        // State machine has not received expected data from caller (data missing, not so much invalid format)
        CODE_UNDERFLOW = 1,
        // State machine accumulated data and caller did not retrieve it in time
        CODE_OVERFLOW = 2
    };

    // EXPERIMENTAL
    // These are intended as internal system errors (think exceptions).  Higher level state machine issues (i.e. validation errors)
    // are not expected to present here (still will be CODE_OK)
    codes code : 3;

    result(const result&) = default;

    constexpr result(bool processed, bool immediate = false, codes code = CODE_OK) :
        processed{processed},
        immediate{immediate},
        end{false},
        code{code}
    {

    }

    result& operator=(const result&) = default;

    static constexpr result more() { return result{true, true}; }
    static constexpr result ok() { return result{true, false}; }
    static constexpr result ignore() { return result{false, false}; }
    static constexpr result underflow() { return {false, false, CODE_UNDERFLOW}; }
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
