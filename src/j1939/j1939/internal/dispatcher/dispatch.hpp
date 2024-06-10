#pragma once

namespace embr { namespace j1939 {

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
using in_place_pgn = estd::integral_constant<pgns, pgn>;

#define J1939_DISPATCH_TARGET(n)    \
    case pgns::n:   return f(in_place_pgn<pgns::n>{});


template <class F>
auto dispatch(F&& f, pgns pgn_) -> decltype(f())
{
    // NOTE: Would be interesting to do this with estd::variadic and/or a fold expression, but I am concerned that it would
    // destroy the optimizer
#if __cpp_fold_expressions
    //using s = estd::make_integer_sequence<unsigned, int(pgns::mf3_end)>;

    // Nifty idea, but constexpr recursion depth kills this
    //dispatch_assist(s{}, std::forward<F>(f), pgn_);
#endif


    switch((pgns)pgn_)
    {
        J1939_DISPATCH_TARGET(address_claimed)
        J1939_DISPATCH_TARGET(oel)
        J1939_DISPATCH_TARGET(lighting_command)

        default:    return f();
    }
}

#undef J1939_DISPATCH_TARGET

template <class F>
auto dispatch(F&& f, can_id id) -> decltype(f())
{
    const uint16_t pgn_ = id.is_pdu1() ?
        pdu1_header(id).range() :
        pdu2_header(id).range();
    return dispatch(std::forward<F>(f), pgns(pgn_));
}


}

}}