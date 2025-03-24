#pragma once

#include "../pgn/enum.h"
#include "../internal/traits.h"

// DEBT: Move these FEATURE_* to a feature area

// When outputting a PGN/PDU code, this will auto emit its actual abbrevation,
// if available
#ifndef FEATURE_EMBR_J1939_OSTREAM_PGN_ABBREV
#define FEATURE_EMBR_J1939_OSTREAM_PGN_ABBREV 1
#endif

// Payload specialized output is nice, but does consume extra ROM.  For
// very constrained devices, turning this off results in generic hex dump
// when out << payload occurs.
#ifndef FEATURE_EMBR_J1939_OSTREAM_FULL_PAYLOAD
#define FEATURE_EMBR_J1939_OSTREAM_FULL_PAYLOAD 1
#endif

// Whether to use floating point on output of ostream/diagnostic
#ifndef FEATURE_EMBR_J1939_OSTREAM_FLOAT
#define FEATURE_EMBR_J1939_OSTREAM_FLOAT 1
#endif


#ifndef FEATURE_EMBR_J1939_OSTREAM_FULL_CM1
#define FEATURE_EMBR_J1939_OSTREAM_FULL_CM1 FEATURE_EMBR_J1939_OSTREAM_FULL_PAYLOAD
#endif

namespace embr { namespace j1939 {

// TODO: Consider further specializing these based on transport, so that we can use native
// types to avoid copying and unnecessary allocating.  If so, it makes sense to do that by policy

///
/// @tparam pgn
/// @tparam Policy - place where perhaps we can specify underlying storage class preferences, etc
template <pgns pgn, class Policy = void, typename = internal::Range<true> >
class pdu;

struct pdu1_header;
struct pdu2_header;

// Tag to indicate in place null initialization, kind of a relative of in_place_t
// DEBT: Somewhat misleading name because for pdu this also initializes pgn and priority into can_id
struct null_t {};


}}
