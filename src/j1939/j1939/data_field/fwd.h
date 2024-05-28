#pragma once

#include <estd/array.h>
#include <estd/span.h>

#include "../pgn/fwd.h"

namespace embr { namespace j1939 {

// Tag to indicate in place null initialization, kind of a relative of in_place_t
struct null_t {};


// 28MAY24 DEBT: Belongs in proper FEATURE area, sort of
// 28MAY24 DEBT: Temporary legacy feature flag to retain old auto null init behavior.
// Phase out by 01JUL24
#ifndef FEATURE_EMBR_J1939_DATAFIELD_AUTOINIT
#define FEATURE_EMBR_J1939_DATAFIELD_AUTOINIT 1
#endif


template<pgns pgn, class TContainer = estd::array<uint8_t,
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

template <pgns pgn, class TContainer = typename data_field<pgn>::container_type>
struct payload_put;

}

}}
