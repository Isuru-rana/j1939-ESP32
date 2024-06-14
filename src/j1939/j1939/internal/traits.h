#pragma once

#include <estd/internal/deduce_fixed_size.h>    // For 'Range'

#include "../pgn/enum.h"

namespace embr { namespace j1939 { namespace internal {

// DEBT: Use estd flavor directly out in code
template <bool v>
using Range = estd::internal::Range<v>;

// This particular traits one SHOULD NOT specialize generally
// Its length member is low level transport reference (think of it as
// minimum length)
template <pgns pgn>
struct pdu_traits;

template <pgns pgn_>
struct pdu_traits
{
    static constexpr bool is_pdu1 = pgn_ < pgns::pdu2_boundary;
    static constexpr bool is_pdu2 = !is_pdu1;

    static constexpr unsigned length = 8;
    static constexpr unsigned pf = is_pdu2 ? ((unsigned) pgn_ >> 8) : (unsigned)pgn_;

    //  0-15, 0 being fastest - UNTESTED  See J1939-21 (2006) Section 5.3.2
    static constexpr bool speed_rank = pf < 240 ?
        (pf / 16) :
        (pf - 240);
};

template <>
struct pdu_traits<pgns::request>
{
    static constexpr bool is_pdu1 = true;
    static constexpr bool is_pdu2 = false;

    static constexpr unsigned length = 3;

    static constexpr pgns pgn = pgns::request;
    static constexpr unsigned pf = (unsigned)pgns::request;
};


}}}