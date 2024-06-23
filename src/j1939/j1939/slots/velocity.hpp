#pragma once

#include <embr/units/meters.h>

#include "slots.h"
#include "../spn/ranges.h"

#include "macro/push.h"

namespace embr { namespace j1939 {


template <>
struct slot_traits<slots::SAEvl02> :
    slot::v1::internal::slot_presentation_tag
{
    using h = slot_traits_helper<uint16_t, 0, estd::ratio<1, 256>>;
    using type = embr::units::meters_per_second<uint16_t,
        estd::ratio_multiply<h::period, estd::ratio<1000, 3600>>::type, h::offset>;
        //h::period, h::offset>;

    // EXPERIMENTAL
    template <class Rep>
    using presentation_type = embr::units::kilometers_per_hour<Rep>;

    static constexpr const char* name() { return "SAEv102"; }
};

}}

#include "macro/pop.h"
