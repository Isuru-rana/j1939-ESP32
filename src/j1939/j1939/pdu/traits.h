#pragma once

#include <estd/cstdint.h>

#include <can/transport.h>


#include "fwd.h"

namespace embr { namespace j1939 {

namespace internal {

// DEBT: Fix up naming
struct address_type_traits_base
{
    static constexpr uint8_t global = 255;
    static constexpr uint8_t null = 254;

    // EXPERIMENTAL
    static constexpr bool assigned(uint8_t v) { return v < 254; }
};

}


// EXPERIMENTAL
template <class TFrame>
struct frame_traits
{
    typedef TFrame frame_type;

    using can_frame_traits = embr::can::frame_traits<frame_type>;

    // Create "plain old" CAN frame from pdu
    template <pgns pgn>
    static constexpr frame_type create(const pdu<pgn>& p)
    {
        return can_frame_traits::create(
            p.can_id(),
            p.data(),
            p.size());
    }
};



// EXPERIMENTAL
template <class Transport>
struct transport_traits
{
    using transport_type = Transport;
    using frame_type = typename transport_type::frame;

    // TODO: In theory this should resolve down to pdu1 and pdu2 variety.  In practice,
    // optimizer might not be that clever.  Consider a .raw() method
    // DEBT: Consider a translated/specialized/more informative return code
    template <pgns pgn>
    inline static bool send(transport_type& t, const pdu<pgn>& p)
    {
        return t.send(frame_traits<frame_type>::create(p));
    }
};



}}
