#pragma once

#include <embr/units/meters.h>

#include "slots.h"
#include "../spn/ranges.h"

#include "macro/push.h"

namespace embr { namespace j1939 {


template <>
struct slot_traits<slots::SAEvl02>
{
    using h = slot_traits_helper<uint16_t, 0, estd::ratio<1, 256>>;
    using type = embr::units::meters_per_second<uint16_t,
        estd::ratio_multiply<h::period, estd::ratio<1000, 3600>>::type, h::offset>;
        //h::period, h::offset>;
};

}}

#include "macro/pop.h"
