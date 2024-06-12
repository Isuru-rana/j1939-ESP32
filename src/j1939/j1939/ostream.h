#include "fwd.h"

#include "data_field/network.hpp"
#include "data_field/oel.hpp"

// DEBT: We really don't want these above, but without them we encounter partial specialization
// issues

// TODO: Consider a ostream_type_traits or similar which indicates behavior overrides (i.e. treat
// an enum as int always)

/*
 * Doesn't beat out inbuilt estd char handler surprisingly, since I would think match on enum would
 * beat match on its underlying type
template <class Streambuf, class Base>
estd::detail::basic_ostream<Streambuf, Base>&
operator << (estd::detail::basic_ostream<Streambuf, Base>& out, embr::j1939::addresses::type value)
{
    return estd::internal::out_int_helper(out, value);
}   */

#include "pgn/ostream.h"
#include "pdu/ostream.h"
#include "units/ostream.h"
#include "NAME/ostream.hpp"

