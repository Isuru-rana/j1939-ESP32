#pragma once

#include <estd/internal/deduce_fixed_size.h>    // For 'Range'

#include "../pgn/enum.h"

namespace embr { namespace j1939 { namespace internal {

// DEBT: Use estd flavor directly out in code
template <bool v>
using Range = estd::internal::Range<v>;

// TODO: Add in 100ms indicator
// See J1939-21 (2006) Section 5.3.2

// This particular traits one SHOULD NOT specialize generally
// Its length member is low level transport reference (think of it as
// minimum length)
template <pgns pgn, class = Range<true> >
struct pdu_traits;

template <pgns pgn_>
struct pdu_traits<pgn_, Range<(pgn_ < pgns::pdu2_boundary)> >
{
    static constexpr bool is_pdu1 = true;
    static constexpr bool is_pdu2 = false;

    static constexpr unsigned length = 8;

    static constexpr pgns pgn = pgn_;
};

template <pgns pgn_>
struct pdu_traits<pgn_, Range<(pgn_ >= pgns::pdu2_boundary)> >
{
    static constexpr bool is_pdu1 = false;
    static constexpr bool is_pdu2 = true;

    static constexpr unsigned length = 8;

    static constexpr pgns pgn = pgn_;
};

template <>
struct pdu_traits<pgns::request, Range<true> >
{
    static constexpr bool is_pdu1 = true;
    static constexpr bool is_pdu2 = false;

    static constexpr unsigned length = 3;

    static constexpr pgns pgn = pgns::request;
};


}}}