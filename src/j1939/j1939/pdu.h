/**
 * References:
 *
 * 1. J1939-21 REV. DEC2006
 * 2. https://www.csselectronics.com/pages/j1939-explained-simple-intro-tutorial
 */
#pragma once

#include "fwd.h"

//#include "bits.hpp"
#include "can_id.h"
#include "pdu/header.h"
#include "pgn/enum.h"
#include "data_field.h"

#include "internal/traits.h"

// Deviates from [1] 5.3 in that we do not include data field in pdu1 or pdu2.
// priority field is included.

namespace embr { namespace j1939 {



template <pgns pgn_>
class pdu1 : public pdu1_header,
    public data_field<pgn_>
{
    typedef pdu1_header id;
    typedef data_field<pgn_> data_field_type;

public:
    static constexpr pgns pgn = pgn_;
#if FEATURE_EMBR_J1939_NO_TRAITS_WRAPPER
    using traits = pgn::traits<pgn_>;

    static constexpr pgn::descriptor descriptor()
    { return { traits::length, traits::default_priority }; }
#else
    static constexpr pgn::descriptor descriptor() { return pgn::get_descriptor<pgn>(); }
#endif

#if FEATURE_EMBR_J1939_DATAFIELD_AUTOINIT
    constexpr explicit pdu1(null_t = {}) :
#else
    //pdu1() = default;
    constexpr explicit pdu1() :
        id{descriptor().default_priority, pgn}
    {}

    constexpr explicit pdu1(null_t) :
#endif
        id{descriptor().default_priority, pgn},
        data_field_type{null_t{}}
    {}

    ///
    /// @param _id - undefined if 'range' does not match template pgn
    /// @param data
    constexpr explicit pdu1(can_id _id, const uint8_t* data) :
        id{_id},
        data_field_type{data}
    {}

    // EXPERIMENTAL
    template <class ...Args>
    explicit pdu1(uint8_t sa, uint8_t da, Args&&...args) :
        id{descriptor().default_priority, pgn},
        data_field_type{std::forward<Args>(args)...}
    {
        source_address(sa);
        destination_address(da);
    }


    const pdu1_header& can_id() const { return *this; }
    pdu1_header& can_id() { return *this; }
    const data_field_type& payload() const { return *this; }
    data_field_type& payload() { return *this; }
};

template <pgns pgn_>
class pdu2 : public pdu2_header,
    public data_field<pgn_>
{
    typedef pdu2_header id;
    typedef data_field<pgn_> data_field_type;

public:
    static constexpr pgns pgn = pgn_;
#if FEATURE_EMBR_J1939_NO_TRAITS_WRAPPER
    using traits = pgn::traits<pgn_>;

    static constexpr pgn::descriptor descriptor()
    { return { traits::length, traits::default_priority }; }
#else
    static constexpr pgn::descriptor descriptor() { return pgn::get_descriptor<pgn>(); }
#endif

#if FEATURE_EMBR_J1939_DATAFIELD_AUTOINIT
    constexpr explicit pdu2(null_t = {}) :
#else
    //pdu2() = default;
    constexpr explicit pdu2() :
        id{descriptor().default_priority, pgn}
    {}

    constexpr explicit pdu2(null_t) :
#endif
        id{descriptor().default_priority, pgn},
        data_field_type(null_t{})
    {}


    ///
    /// @param _id - undefined if 'range' does not match template pgn
    /// @param data
    pdu2(can_id _id, const uint8_t* data) :
        id{_id},
        data_field_type{data}
    {}

    // EXPERIMENTAL
    template <class ...Args>
    pdu2(uint8_t sa, Args&&...args) :
        id{descriptor().default_priority, pgn},
        data_field_type{std::forward<Args>(args)...}
    {
        pdu2_header::source_address(sa);
    }

    const pdu2_header& can_id() const { return *this; }
    pdu2_header& can_id() { return *this; }
    const data_field_type& payload() const { return *this; }
    data_field_type& payload() { return *this; }
};


template <pgns pgn>
class pdu<pgn, void, internal::Range<(pgn < pgns::pdu2_boundary)> > : public pdu1<pgn>
{
    typedef pdu1<pgn> base_type;

public:
    ESTD_CPP_FORWARDING_CTOR(pdu)
};

template <pgns pgn>
class pdu<pgn, void, internal::Range<(pgn >= pgns::pdu2_boundary)> > : public pdu2<pgn>
{
    typedef pdu2<pgn> base_type;

public:
    ESTD_CPP_FORWARDING_CTOR(pdu)
};

}}
