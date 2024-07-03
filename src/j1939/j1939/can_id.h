/**
 * References:
 *
 * 1. https://www.kvaser.com/about-can/higher-layer-protocols/j1939-introduction/
 * 2. J1939-21 (2006)
 */
#pragma once

#include <estd/cstdint.h>

#include <embr/bits/word.hpp>

#include "addresses.h"
#include "pgn/enum.h"

namespace embr { namespace j1939 {

// as per [1]
class can_id
{
protected:
    using desc = bits::descriptor;
    using address_type = addresses::type;

    // EXPERIMENTAL
    struct traits
    {
        typedef bits::experimental::bit_traits<26, 3> priority;
    };

    struct d
    {
        static constexpr desc source_address()  { return desc{0, 8}; }
        static constexpr desc pdu_specific()    { return desc{8, 8}; }
        static constexpr desc pdu_format()      { return desc{16, 8}; }
        static constexpr desc priority()        { return desc{26, 3}; }

        static constexpr desc range_pdu1()      { return desc{16, 10}; }
        static constexpr desc range_pdu2()      { return desc{8, 18}; }
    };

    embr::bits::internal::word<29> value;

#if FEATURE_EMBR_J1939_DATAFIELD_AUTOINIT == 0
    can_id() = default;
#endif

public:
    constexpr explicit can_id(uint32_t v) : value{v} {}

    constexpr address_type source_address() const
    {
        return address_type(value.get(d::source_address()));
    }

    // pdu1 this is dest address
    // pdu2 this is group extension (part of pgn#)
    constexpr uint8_t pdu_specific() const { return value.get(d::pdu_specific()); }

    // [2] 5.2.4
    constexpr uint8_t pdu_format() const { return value.get(d::pdu_format()); }

    constexpr bool data_page() const
    {
        return (value & ((uint32_t)1 << 24)) != 0U;
    }

    constexpr bool reserved() const
    {
        return (value & ((uint32_t)1 << 25)) != 0U;
    }

    constexpr bool extended_data_page() const
    {
        return reserved();
    }

    constexpr traits::priority::word_type priority() const { return value.get(d::priority()); }
    //constexpr traits::priority::word_type priority() const { return value.get<traits::priority>(); }

    /// "A value of 0 has the highest priority." [1]
    // FIX: value.value() now a constexpr temporary - could be viable reworking bit traits
    // to take a value and/or reference --
    //constexpr traits::priority::word_type priority() const
    //{ return traits::priority::get(&value.value()); }

    void pdu_specific(uint8_t v) { value.set(d::pdu_specific(), v); }

    void source_address(uint8_t v) { value.set(d::source_address(), v); }

    void priority(uint8_t v) { value.set(d::priority(), v); }

    // [2] Section 5.2.5
    constexpr bool is_pdu1() const { return pdu_format() < 240; }

    constexpr operator uint32_t() const { return value.value(); }

    // DEBT: Probably could use a better name
    constexpr uint32_t raw() const { return value.value(); }

    constexpr uint16_t range_pdu1() const { return value.get(d::range_pdu1()); }
    constexpr uint32_t range_pdu2() const { return value.get(d::range_pdu2()); }
    void range_pdu1(uint16_t v) { value.set(d::range_pdu1(), v); }
    void range_pdu2(uint32_t v) { value.set(d::range_pdu2(), v); }
};


// DEBT: Move this out to pdu header area, just to avoid pgns enum inclusion
namespace internal {

constexpr pgns get_pgn(const can_id& id)
{
    return id.is_pdu1() ?
        pgns(id.range_pdu1()) :
        pgns(id.range_pdu2());
}


// DEBT: the whole pdu1 = 0x??00 range probably is gonna change this
constexpr bool is_pdu1(pgns pgn)
{
    return pgn < pgns::pdu2_boundary;
}


}

}}
