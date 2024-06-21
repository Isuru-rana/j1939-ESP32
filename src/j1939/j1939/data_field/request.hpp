#pragma once

#include "base.h"

// NOTE: Very unusual in that
// "The CAN frame for this PG shall set the DLC to 3."
// j1939-21 (2006) Section 5.4.1 Figure 8

// 20JUN24 FIX: RQST has some issues:
// 1.  It appears ostream treatment isn't picking up bytes correctly.  PGN 0x00EE00 received,
//     but 0xCA 00 00 is rendered.
// 2.  Our enum trick of using the 8-bit/ps portion only of addressable PDU/PGN works well, but in
//     this case translating back from pgn() field  means we'll need to treat it as 18-bit all
//     the time

namespace embr { namespace j1939 {

namespace spn {

template <>
struct type_traits<spns::parameter_group_number_rqst> : internal::type_traits_base<uint32_t>
{

};

template<>
constexpr descriptor get_descriptor<spns::parameter_group_number_rqst>()
{
    return descriptor{1, 1, 24};
}

}

namespace pgn {

template <>
struct traits<pgns::request> : internal::traits_base
{
    using spns = internal::spns_list<s::parameter_group_number_rqst>;

    static constexpr const char* name()
    {
        return "Request";
    }

    // DEBT: J1939-21 (REV2006) Table D1 implies this, but not 100% sure
    static constexpr const char* abbrev() { return "RQST"; }

    // [1] Section 5.4.1
    static constexpr unsigned length = 3;
};


}

template <class Container>
struct data_field<pgns::request, Container> :
    internal::data_field_base<Container>
{
    struct d
    {
        //static constexpr spn::descriptor pgn()
        //{ return spn::descriptor{}}
    };

    typedef internal::data_field_base<Container> base_type;

    ESTD_CPP_FORWARDING_CTOR(data_field)

    uint32_t pgn() const
    {
        return base_type::template get<spns::parameter_group_number_rqst>();
    }

    void pgn(uint32_t v)
    {
        return base_type::template set<spns::parameter_group_number_rqst>(v);
    }

    // EXPERIMENTAL
    explicit data_field(uint32_t pgn)
    {
        this->pgn(pgn);
    }
};

}}
