#pragma once

#include "base.h"

#include "../slots.hpp"

#include "../slots/macro/push.h"

namespace embr { namespace j1939 {

namespace spn {

template <>
struct type_traits<spns::auxiliary_temperature_1> : internal::slot_type_traits<slots::SAEtp01>
{
};

}

}}

#include "../slots/macro/pop.h"
