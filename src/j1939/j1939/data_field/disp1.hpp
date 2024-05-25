#pragma once

#include "base.hpp"

namespace embr { namespace j1939 {

template<class Container>
struct data_field<pgns::disp1, Container> :
    internal::data_field_base<Container>
{
    typedef internal::data_field_base<Container> base_type;

    data_field() = default;

    data_field(const uint8_t* copy_from) : base_type(copy_from) {}
};

}}
