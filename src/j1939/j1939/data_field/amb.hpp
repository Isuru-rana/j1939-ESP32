#pragma once

#include "base.hpp"

#include "../slots/macro/push.h"

namespace embr { namespace j1939 {

namespace spn {

template <>
struct type_traits<spns::cab_interior_temperature> :
    internal::slot_type_traits<slots::SAEtp02> {};

}

}}

#include "../slots/macro/pop.h"
