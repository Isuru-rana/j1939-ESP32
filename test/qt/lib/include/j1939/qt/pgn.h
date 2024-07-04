#pragma once

#include <j1939/pgn/enum.h>

#include "data_field.h"

// DEBT: This should live in non-qt portion of j1939 lib

namespace embr::j1939 {

template <bool abbrev>
struct pgn_to_string_functor
{
    template <pgns pgn>
    constexpr const char* operator()(internal::in_place_pgn<pgn>)
    {
#if FEATURE_EMBR_J1939_NO_TRAITS_WRAPPER
        using traits = j1939::pgn::traits<pgn>;

        if constexpr(traits::is_specialized)
            return abbrev ? traits::abbrev() : traits::name();
        else
            return "N/A";
#else
        return internal::traits_wrapper<pgn>::name();
#endif
    }

    constexpr const char* operator()(pgns) const { return nullptr; }
};

// NOTE: Beware!  This commits to whatever specialized pdus are available
//constexpr
inline
    const char* to_string(pgns pgn)
{
    return j1939::v1::dispatch<internal::dispatch_default_policy>(pgn_to_string_functor<false>{}, pgn);
}

}

