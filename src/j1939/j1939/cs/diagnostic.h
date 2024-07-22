#pragma once

#include "base.h"

namespace embr { namespace j1939 { namespace cs { inline namespace v0 {

class diagnostic : base
{
public:
    template <class Transport>
    result process_incoming_default(Transport&, const typename Transport::frame& f) const;    // NOLINT

    template <class Transport, pgns pgn>
    result process_incoming(Transport&, const pdu<pgn>& p);

};

}}}}