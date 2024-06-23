#pragma once

#include <can/fwd.h>

#include "dispatch.hpp"

namespace embr { namespace j1939 {

namespace internal {

namespace v2 {


class specialize_frame_functor
{
public:
    template <pgns pgn, class F, class Frame, class ...Args>
    bool operator()(j1939::internal::in_place_pgn<pgn>, F&& f, const Frame& frame, Args&&...args) const
    {
        using traits = can::frame_traits<Frame>;

        f(pdu<pgn>(traits::id(f), traits::payload(f)), std::forward<Args>(args)...);

        return true;
    }

    template <class F, class Frame, class ...Args>
    bool operator()(pgns, F&&, const Frame&, Args&&...args)
    {
        return false;
    }
};


template <class Transport>
class process_incoming_functor
{
    using frame = typename Transport::frame;
public:
    template <pgns pgn, class Impl, class ...Args>
    bool operator()(j1939::internal::in_place_pgn<pgn>, Impl& impl, Transport& t, const frame& f, Args&&...args) const
    {
        //using traits = j1939::frame_traits<frame>;
        using traits = can::frame_traits<frame>;
        using pdu_type = pdu<pgn>;

        // DEBT: Ensure payload size is correct
        pdu_type p(
            traits::id(f),
            traits::payload(f));

        impl.process_incoming(t, p, std::forward<Args>(args)...);

        return {};
    }

    template <class Impl>
    bool operator()(pgns, Impl& impl, Transport&, const frame&) { return{}; }
};

template <class Transport, class Impl, class ...Args>
bool process_incoming(Impl& impl,
    Transport& transport,
    const typename estd::remove_cvref_t<Transport>::frame& f,
    Args&&...args)
{
    using transport_type = typename estd::remove_cvref_t<Transport>;
    using frame = typename transport_type::frame;
    // DEBT: Be careful, j1939::frame_traits is different than can::frame_traits.  Could
    // we derive j1939 flavor from can flavor?
    using traits = embr::can::frame_traits<frame>;
    can_id id(traits::id(f));
    const uint16_t pgn_ = id.is_pdu1() ?
        pdu1_header(id).range() :
        pdu2_header(id).range();

    internal::dispatch(process_incoming_functor<transport_type>{},
        //id,
        pgns(pgn_),
        impl,
        transport,
        f,
        std::forward<Args>(args)...);
    return {};
}

}

}

}}
