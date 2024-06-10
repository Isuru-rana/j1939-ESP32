/**
 *
 * References:
 *
 * 1. J1939-21 (REV. DEC2006)
 * 2. - 9. Reserved
 * 10. https://grayhill.github.io/comm_protocols/sae_j1939.html
 * 11. J1939-71 (REV. DEC2003)
 */
#pragma once

#include <estd/algorithm.h>

#include <estd/cstdint.h>

#include <embr/bits/bits.hpp>

#include "pgn/enum.h"
#include "spn/enum.h"

#include "data_field/base.h"

#include "fwd.h"

namespace embr { namespace j1939 {


template<pgns pgn, class Container>
struct data_field :
    internal::data_field_base<Container>
{
    typedef internal::data_field_base<Container> base_type;

    ESTD_CPP_FORWARDING_CTOR(data_field)
};




}}