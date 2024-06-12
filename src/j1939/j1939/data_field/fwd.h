#pragma once

#include <estd/array.h>
#include <estd/span.h>

#include <embr/bits/bits.h>

#include "../pgn/fwd.h"

namespace embr { namespace j1939 {

// 28MAY24 DEBT: Belongs in proper FEATURE area, sort of
// 28MAY24 DEBT: Temporary legacy feature flag to retain old auto null init behavior.
// Phase out by 01JUL24
// NOTE: pdu header is still initialized, since primary use cases are:
// 1. copy initialize from a raw data frame
// 2. constructing new frame for output, necessitating pgn and somewhat priority
#ifndef FEATURE_EMBR_J1939_DATAFIELD_AUTOINIT
#define FEATURE_EMBR_J1939_DATAFIELD_AUTOINIT 1
#endif


template<pgns pgn, class Container = estd::array<uint8_t,
    pgn::get_descriptor<pgn>().length> >
struct data_field;


namespace layer1 {

template<pgns pgn>
using data_field = embr::j1939::data_field<pgn>;

}

namespace layer2 {

template<pgns pgn>
using data_field = embr::j1939::data_field<pgn,
    estd::span<uint8_t, pgn::get_descriptor<pgn>().length> >;

}

namespace experimental {

struct get_helper_tag {};

template <class TEnum>
struct get_helper;

}

namespace internal {

template <pgns pgn, class Container = typename data_field<pgn>::container_type>
struct payload_put;

template <class Container, bits::endianness e = bits::little_endian>
struct data_field_base;

}

}}
