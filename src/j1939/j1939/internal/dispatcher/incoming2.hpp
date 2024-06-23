#pragma once

#include <can/fwd.h>

#include "dispatch.hpp"

namespace embr { namespace j1939 {

namespace internal {

namespace v2 {

template <class Transport>
class process_incoming_functor
{
    using frame = typename Transport::frame;
public:
    template <pgns pgn, class Impl>
    void operator()(j1939::internal::in_place_pgn<pgn>, Impl&, Transport&& t, const frame& f) const
    {
        using traits = can::frame_traits<frame>;
        using pdu_type = pdu<pgn>;

        // DEBT: Ensure payload size is correct
        pdu_type p(
            traits::id(f),
            traits::payload());
    }

    void operator()(pgns) { }
};

template <class Transport, class Impl, class ...Args>
bool process_incoming(Impl& impl,
    Transport&& transport,
    const typename estd::remove_cvref_t<Transport>::frame& f,
    Args&&...args)
{
    using transport_type = const typename estd::remove_cvref_t<Transport>;
    using frame = typename transport_type::frame;
    // DEBT: Be careful, j1939::frame_traits is different than can::frame_traits.  Could
    // we derive j1939 flavor from can flavor?
    using traits = embr::can::frame_traits<frame>;
    can_id id(traits::id(f));
    dispatch(process_incoming_functor<transport_type>{}, id,
        impl,
        std::forward<Transport>(transport),
        f,
        std::forward<Args>(args)...);
    return {};
}

}

}

}}
