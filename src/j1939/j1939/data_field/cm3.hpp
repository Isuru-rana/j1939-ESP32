#pragma once

#include "base.h"

namespace embr { namespace j1939 {

template<class TContainer>
struct data_field<pgns::cab_message_3, TContainer> :
    internal::data_field_base<TContainer>
{
    typedef internal::data_field_base<TContainer> base_type;

    data_field() = default;

    data_field(const uint8_t* copy_from) : base_type(copy_from) {}
};

namespace pgn {

template <>
struct traits<pgns::cab_message_3> : internal::traits_base
{
    static constexpr const char* name()
    {
        return "Cab Message 3";
    }

    static constexpr const char* description()
    {
        return "Provides information from Cab mounted operator inputs.";
    }

    static constexpr const char* abbrev() { return "CM3"; }
};


}

}}
