#pragma once

// DEBT: Hey you, stop that!
#include "../data_field/all.hpp"

#include "diagnostic.h"

#include "../ostream.h"

namespace embr { namespace j1939 {

// Adapted from https://devblogs.microsoft.com/oldnewthing/20190710-00/?p=102678
// https://stackoverflow.com/questions/39816779/check-if-type-is-defined

// NOTE: They are correct that there are some limitations/problems with this
// is_type_complete

/*
template<typename, typename = void>
struct is_type_complete : estd::false_type {};

template<typename T>
struct is_type_complete<T, estd::enable_if_t<(sizeof(T) > 0)> > : estd::true_type {};
 */

template <class TTransport, class TOStream, class Policy>
template <embr::j1939::pgns pgn>
auto diagnostic_ca<TTransport, TOStream, Policy>::process_incoming(transport_type& t, const pdu<pgn>& p) -> result
{
    out << p << estd::endl;

    return result::ok();
}

template <class TTransport, class TOStream, class Policy>
#if EXP_DIAGNOSTIC_OPT1
constexpr
#endif
auto diagnostic_ca<TTransport, TOStream, Policy>::process_incoming_default(
    transport_type& t, const frame_type& f) const -> result
{
#if EXP_DIAGNOSTIC_OPT1
    return false;
#else
    //internal::pdu_header id2(frame_traits::id(f));

    pdu1_header id{frame_traits::id(f)};
    pdu2_header _id{frame_traits::id(f)};

    //out << estd::hex;
    //out << "range1: " << id.range() << ", range2: " << _id.range() << estd::endl;

    //auto pgn = (long) (id.is_pdu1() ? id.range() : _id.range());

    out << "PDU: " << estd::hex;

    if(id.is_pdu1())
        out << id.range() << ' ' << id;
    else
        out << _id.range() << ' ' << _id;

    const uint8_t* payload = frame_traits::payload(f);

    for(unsigned i = 0; i < frame_traits::length(f); i++)
        out << ' ' << estd::setw(2) << payload[i];

    out << estd::endl;

    return result::ignore();
#endif
}


}}
