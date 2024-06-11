#pragma once

#include "../../pgn/enum.h"
#include "../../spn/enum.h"
#include "../../pgn/traits.h"

namespace embr { namespace j1939 {

#ifdef __cpp_concepts
namespace concepts {
template <class F>
concept Functor = requires(F f)
{
    { f(pgns{}) };
};

}
#endif

namespace internal {

template <class Key, Key key, class F>
bool dispatch_one(F&& f, Key compare_to)
{
    if(compare_to == key) return false;

    f();

    return true;
}

#if __cpp_fold_expressions
template <class Key, Key ...keys, class F>
void dispatch_assist(estd::integer_sequence<Key, keys...>, F&& f, Key key)
{
    (... || dispatch_one<Key, keys>(std::forward<F>(f), key));
}
#endif

template <pgns pgn>
using in_place_pgn = j1939::internal::traits_wrapper<pgn>;

#define J1939_DISPATCH_TARGET(n)    \
    case pgns::n:   return f(in_place_pgn<pgns::n>{});


template <ESTD_CPP_CONCEPT(concepts::Functor) F>
auto dispatch(F&& f, pgns pgn_) -> decltype(f(pgns{}))
{
    // NOTE: Would be interesting to do this with estd::variadic and/or a fold expression, but I am concerned that it would
    // destroy the optimizer
#if __cpp_fold_expressions
    //using s = estd::make_integer_sequence<unsigned, int(pgns::mf3_end)>;

    // Nifty idea, but constexpr recursion depth kills this
    //dispatch_assist(s{}, std::forward<F>(f), pgn_);
#endif


    switch(pgn_)
    {
        J1939_DISPATCH_TARGET(acknowledgement)
        J1939_DISPATCH_TARGET(address_claimed)
        J1939_DISPATCH_TARGET(cm1)
        J1939_DISPATCH_TARGET(cm3)
        J1939_DISPATCH_TARGET(bjm1)
        J1939_DISPATCH_TARGET(bjm2)
        J1939_DISPATCH_TARGET(oel)
        J1939_DISPATCH_TARGET(lighting_command)
        J1939_DISPATCH_TARGET(lighting_data)
        J1939_DISPATCH_TARGET(sensor_electrical_power_1)
        J1939_DISPATCH_TARGET(sensor_electrical_power_2)
        J1939_DISPATCH_TARGET(vep1)
        J1939_DISPATCH_TARGET(vep2)
        J1939_DISPATCH_TARGET(vep3)

        default:    return f(pgn_);
    }
}

#undef J1939_DISPATCH_TARGET

template <class F>
auto dispatch(F&& f, can_id id) -> decltype(f(pgns{}))
{
    const uint16_t pgn_ = id.is_pdu1() ?
        pdu1_header(id).range() :
        pdu2_header(id).range();
    return dispatch(std::forward<F>(f), pgns(pgn_));
}


}

}}
