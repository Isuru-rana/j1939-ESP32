#include <estd/iosfwd.h>


#include "../fwd.h"

namespace embr { namespace j1939 {

template <class Streambuf, class Base>
estd::detail::basic_ostream<Streambuf, Base>& operator <<(
    estd::detail::basic_ostream<Streambuf, Base>& out,
    const pdu1_header& ph)
{
    out << "SA:" << uint8_t(ph.source_address()) << ' ';
    out << "DA:" << uint8_t(ph.destination_address());
    return out;
}


template <class Streambuf, class Base>
estd::detail::basic_ostream<Streambuf, Base>& operator <<(
    estd::detail::basic_ostream<Streambuf, Base>& out,
    const pdu2_header& ph)
{
    out << "SA:" << uint8_t(ph.source_address());
    return out;
}

}}