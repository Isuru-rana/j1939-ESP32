#pragma once

#include "../fwd.h"
#include "../../pgn/enum.h"

namespace embr { namespace j1939 {

namespace internal {

template <pgns pgn, class Enabled = void>
struct pgn_put;

}


}

template <j1939::pgns pgn>
constexpr j1939::internal::pgn_put<pgn> put_pdu(const j1939::pdu<pgn>& pdu_);

}