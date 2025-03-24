#include "diagnostic.h"

namespace embr { namespace j1939 { namespace cs { inline namespace v0 {

template <class Transport, embr::j1939::pgns pgn>
auto diagnostic::process_incoming(Transport&, const pdu<pgn>& p) -> result
{
    // Not ready yet
    //out_ << p << estd::endl;

    return result::ok();
}

template <class Transport>
auto diagnostic::process_incoming_default(
    Transport& t, const typename Transport::frame& f) const -> result
{
    // Not ready yet
    /*
    using frame_type = typename Transport::frame;
    using frame_traits = can::frame_traits<frame_type>;

    pdu1_header id{frame_traits::id(f)};
    pdu2_header _id{frame_traits::id(f)};

    out_ << "PDU: " << estd::hex;

    if(id.is_pdu1())
        out_ << id.range() << ' ' << id;
    else
        out_ << _id.range() << ' ' << _id;

    const uint8_t* payload = frame_traits::payload(f);

    for(unsigned i = 0; i < frame_traits::length(f); i++)
        out_ << ' ' << estd::setw(2) << payload[i];

    out_ << estd::endl; */

    return result::ignore();
}

}}}}