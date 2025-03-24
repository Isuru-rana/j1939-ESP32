#pragma once

#include <estd/iomanip.h>

#include "../../data_field/fwd.h"
#include "fwd.h"

namespace embr { namespace j1939 {

namespace internal {

// DEBT: Consolidate this with diagnostic payload output
// No fancy PDU specializations, just raw bytes
template <class Streambuf, class Base, class Container>
void write_raw_payload(estd::detail::basic_ostream<Streambuf, Base>& out,
    const internal::data_field_base<Container>& payload)
{
    out.fill('0');

    for(unsigned v : payload) out << estd::setw(2) << v << ' ';
}

template <class Container>
struct payload_put_base : estd::internal::ostream_functor_tag
{
    using payload_type = internal::data_field_base<Container>;
    const payload_type& payload;

    constexpr explicit payload_put_base(const payload_type& payload) : payload{payload} {}

    template <class Streambuf, class Base>
    void operator()(estd::detail::basic_ostream<Streambuf, Base>& out) const
    {
        write_raw_payload(out, payload);
    }
};

template <pgns pgn, class Container>
struct payload_put : payload_put_base<Container>
{
    constexpr explicit payload_put(const data_field<pgn>& payload) :
        payload_put_base<Container>{payload} {}
};

}

}}
