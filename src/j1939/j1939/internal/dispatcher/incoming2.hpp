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
    constexpr auto operator()(j1939::internal::in_place_pgn<pgn>, F&& f, const Frame& frame, Args&&...args) const ->
        decltype(f(std::declval<pdu<pgn>>(), args...))
    {
        using traits = can::frame_traits<Frame>;

        return f(
            pdu<pgn>(traits::id(frame), traits::payload(frame)),
            std::forward<Args>(args)...);
    }

    template <class F, class Frame, class ...Args>
    constexpr auto operator()(pgns pgn, F&& f, const Frame&, Args&&...args) const ->
        decltype(f(pgns{}, args...))
    {
        return f(pgn, std::forward<Args>(args)...);
    }
};


template <class Transport>
class process_incoming_functor
{
    using frame = typename Transport::frame;
public:
    template <pgns pgn, class Impl, class ...Args>
    constexpr bool operator()(j1939::internal::in_place_pgn<pgn>, Impl& impl, Transport& t, const frame& f, Args&&...args) const
    {
        //using traits = j1939::frame_traits<frame>;
        using traits = can::frame_traits<frame>;
        using pdu_type = pdu<pgn>;

        // DEBT: Ensure payload size is correct

        return impl.process_incoming(t,
            pdu_type(traits::id(f), traits::payload(f)),
            std::forward<Args>(args)...);
    }

    template <class Impl>
    constexpr bool operator()(pgns, Impl&, Transport&, const frame&) const { return{}; }
};


class test_rcv_specialized_functor
{
public:
    template <pgns pgn>
    bool operator()(const pdu<pgn>& p) const
    {
        return true;
    }

    bool operator()(pgns) const
    {
        return false;
    }
};

template <class Transport, class Impl, class ...Args>
constexpr bool process_incoming(Impl& impl,
    Transport& transport,
    const typename estd::remove_cvref_t<Transport>::frame& f,
    Args&&...args)
{
    using transport_type = typename estd::remove_cvref_t<Transport>;
    using frame = typename transport_type::frame;
    // DEBT: Be careful, j1939::frame_traits is different than can::frame_traits.  Could
    // we derive j1939 flavor from can flavor?
    using traits = embr::can::frame_traits<frame>;

    return internal::dispatch<dispatch_default_policy>(
        process_incoming_functor<transport_type>{},
        //id,
        get_pgn(traits::id(f)),
        impl,
        transport,
        f,
        std::forward<Args>(args)...);
}

}

}

}}
