#pragma once

#include "../../pgn/enum.h"
#include "../../spn/enum.h"
#include "../../pgn/traits.h"

#include "fwd.h"

namespace embr { namespace j1939 {

namespace internal {

// DEBT: Eventually I want this to be just traits
template <pgns pgn>
#if FEATURE_EMBR_J1939_NO_TRAITS_WRAPPER
// Requires too much specialization knowledge up front
//using in_place_pgn = j1939::pgn::traits<pgn>;
// This guy auto-converts to pgns value, which we don't want
//using in_place_pgn = estd::integral_constant<pgns, pgn>;
using in_place_pgn = j1939::internal::pdu_traits<pgn>;
#else
using in_place_pgn = j1939::internal::traits_wrapper<pgn>;
#endif

enum policy_modes
{
    DISPATCH_POLICY_BLACKLIST,          // Execute all pgns except for ones in list
    DISPATCH_POLICY_WHITELIST,          // Execute only pgns in list
};

// TODO: Need variadic value_selector built out just a bit more for this to work
struct dispatch_default_policy
{
    // idea#1
    static constexpr policy_modes policy = DISPATCH_POLICY_BLACKLIST;

    using list = estd::variadic::values<pgns>;

    // alternate idea#2

    // empty whitelist = allow all (implicit *)
    using whitelist = estd::variadic::values<pgns>;
    // empty blacklist = deny none
    using blacklist = estd::variadic::values<pgns>;
};

template <class Policy, pgns pgn, class Enabled = void>
struct should_execute_pgn : estd::bool_constant<false> {};

template <class Policy, pgns pgn>
struct should_execute_pgn<
    Policy,
    pgn,
    estd::enable_if_t<
        ((Policy::whitelist::size() == 0) || Policy::whitelist::template contains<pgn>()) &&
        (Policy::blacklist::template contains<pgn>() == false)
        > > :
        estd::bool_constant<true>
{

};


template <class Policy, pgns pgn, class Enabled = void>
struct exec_dispatch;

// DEBT: Can probably do this with a regular bool specialization not enable_if, just
// hedging our bets for now
template <class Policy, pgns pgn>
struct exec_dispatch<
    Policy,
    pgn,
    estd::enable_if_t<should_execute_pgn<Policy, pgn>::value> >
{
    template <class F, class ...Args>
    constexpr auto operator()(F&& f, Args&&...args) -> decltype(f(pgns{}, args...))
    {
        return f(in_place_pgn<pgn>{}, std::forward<Args>(args)...);
    }
};


template <class Policy, pgns pgn>
struct exec_dispatch<
    Policy,
    pgn,
    estd::enable_if_t<!should_execute_pgn<Policy, pgn>::value> >
{
    template <class F, class ...Args>
    constexpr auto operator()(F&& f, Args&&...args) -> decltype(f(pgns{}, args...))
    {
        return f(pgn, std::forward<Args>(args)...);
    }
};


}

}}
