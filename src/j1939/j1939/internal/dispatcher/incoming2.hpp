#pragma once

#include "dispatch.hpp"

namespace embr { namespace j1939 {

namespace internal {

namespace v2 {

template <class Transport>
class process_incoming_functor
{
    using frame = typename Transport::frame;
public:
    template <pgns pgn>
    void operator()(j1939::internal::in_place_pgn<pgn>, Transport&& t, const frame& f) const
    {
        using traits = frame_traits<frame>;
        using pdu_type = pdu<pgn>;

        // DEBT: Ensure payload size is correct
        pdu_type p(
            traits::id(f),
            traits::payload());
    }

    void operator()(pgns) { }
};

template <class Transport, class Impl, class ...Args>
bool process_incoming(Transport&& transport, const typename Transport::frame& f, Args&&...args)
{
    using frame = typename Transport::frame;
    using traits = frame_traits<frame>;
    can_id id(traits::id(f));
    dispatch(process_incoming_functor<Transport>{}, id,
        std::forward<Transport>(transport),
        f,
        std::forward<Args>(args)...);
    return {};
}

}

}

}}
