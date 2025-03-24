#pragma once

#include "base.hpp"

namespace embr { namespace j1939 {

template<class Container>
struct data_field<pgns::disp1, Container> :
    internal::data_field_base<Container>
{
    using base_type = internal::data_field_base<Container>;

    ESTD_CPP_FORWARDING_CTOR(data_field)
};

}}
