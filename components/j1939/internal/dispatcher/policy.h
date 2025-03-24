#pragma once

#include <estd/internal/variadic.h>

#include "../../pgn/enum.h"

namespace embr { namespace j1939 { namespace internal {

enum policy_modes
{
    DISPATCH_POLICY_BLACKLIST,          // Execute all pgns except for ones in list
    DISPATCH_POLICY_WHITELIST,          // Execute only pgns in list
};


// TODO: Make a Policy concept
// Policy is based on idea that noop switch optimization is insufficient.  Indeed, on AVR
// noop/effectively empty case statements still seem to occupy ~40-70 bytes per.  This policy
// *seems* to mitigate that by forcing particular identified pgns down the default-unidentified
// path.  Strange though, because I thought both of those were emtpy constexpr functions.
struct dispatch_default_policy
{
    template <pgns ... Values>
    using pgn_list = estd::variadic::values<pgns, Values...>;

    // idea#1
    static constexpr policy_modes policy = DISPATCH_POLICY_BLACKLIST;

    using list = pgn_list<>;

    // alternate idea#2

    // empty whitelist = allow all (implicit *)
    using whitelist = pgn_list<>;
    // empty blacklist = deny none
    using blacklist = pgn_list<>;
};

}}}
