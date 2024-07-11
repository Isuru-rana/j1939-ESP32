#pragma once

#include <estd/type_traits.h>

#include "../ca.h"
#include "../pgn/traits.h"
#include "../cs/diagnostic.h"

#include "fwd.h"

namespace embr { namespace j1939 { inline namespace v1 {

// Just to see how much ROM 'process_incoming_default' really uses
#define EXP_DIAGNOSTIC_OPT1 0

// DEBT: He has now become a 'cs'.  Make a 'v2' which doesn't take TTransport
template <class TTransport, class OStream, class Policy = internal::dispatch_default_policy>
class diagnostic_ca : public cs::v1::base
{
    using base_type = embr::j1939::impl::controller_application<TTransport>;

    using transport_type = TTransport;
    typedef typename TTransport::frame frame_type;
    using frame_traits = can::frame_traits<frame_type>;

    // DEBT: Consider passing this in as process_incoming param
    // DEBT: Really wants to be estd::detail::basic_ostream<Streambuf, Base>
    OStream& out_;

public:
    using policy_type = Policy;

    OStream& out() { return out_; }

    // TODO: Bring in CTAD
    explicit constexpr diagnostic_ca(OStream& out) : out_(out) {}

    // DEBT: inline instead of constexpr seems to hels compiler not favor this
    // one.  However, that is obnoxious
    //template <class TPDU>
    //inline bool process_incoming(transport_type&, TPDU) { return false; }

#if EXP_DIAGNOSTIC_OPT1
    constexpr
#endif
    result process_incoming_default(transport_type& t, const frame_type& f) const;    // NOLINT

    template <pgns pgn>
    result process_incoming(transport_type& t, const pdu<pgn>& p);
};

}}}
