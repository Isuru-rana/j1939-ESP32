#pragma once

#include "../../pgn/fwd.h"

namespace embr { namespace j1939 {

template <pgns pgn, class Streambuf, class Base>
constexpr estd::detail::basic_ostream<Streambuf, Base>& operator<<(
    estd::detail::basic_ostream<Streambuf, Base>& out,
    const pdu<pgn>& p)
{
    return out << embr::put_pdu(p);
}

}

template <j1939::pgns pgn>
constexpr j1939::internal::pgn_put<pgn> put_pdu(const j1939::pdu<pgn>& pdu_)
{
    return { pdu_ };
}

}