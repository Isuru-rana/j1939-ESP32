#pragma once

namespace embr { namespace j1939 {

template<class Container>
struct data_field<pgns::request2, Container> :
    internal::data_field_base<Container>
{
    typedef internal::data_field_base<Container> base_type;

    struct d
    {
        static constexpr spn::descriptor pgn() { return {1, 1, 24}; };
        static constexpr spn::descriptor use_transfer() { return {4, 2, 2}; }
    };

    data_field() = default;

    explicit data_field(const uint8_t* copy_from) : base_type(copy_from) {}
};

}}
