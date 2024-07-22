#pragma once

#include "traits.h"
#include "../can_id.h"
#include "../pgn/fwd.h"

namespace embr { namespace j1939 {

#if UNUSED
// As per [1] Section 5.2
// NOTE: This is PDU minus the 'Data Field' portion, thus the term 'header' whose name does not
// appear in the spec specifically
struct pdu_header : bits::material<bits::little_endian, bits::lsb_to_msb>
{
    typedef bits::material<bits::little_endian, bits::lsb_to_msb> base_type;

    struct d
    {
        static constexpr bits::descriptor P = bits::descriptor{2, 3};
        static constexpr bits::descriptor EDP = bits::descriptor{1, 1};
    };

    uint8_t priority() const { return get<uint8_t>(0, d::P); }
    void priority(uint8_t v) { set<uint8_t>(0, d::P, v); }

    bool extended_data_page() const { return get<bool>(0, d::EDP); }
    void extended_data_page(bool v) { set(0, d::EDP, v); }
};
#endif

struct pdu1_header : can_id
{
    friend class internal::pdu_header;

    constexpr explicit pdu1_header(uint32_t v) : can_id(v) {}

    // EXPERIMENTAL
    constexpr pdu1_header(uint8_t priority, pgns pgn) :
        can_id{((uint32_t)priority << d::priority().bitpos) | (((uint32_t)pgn) << d::range_pdu1().bitpos)}
    {}

    constexpr uint8_t destination_address() const { return pdu_specific(); }
    void destination_address(uint8_t v) { pdu_specific(v); }

    // Limited 10-bit pdu1 command range
    constexpr uint16_t range() const { return range_pdu1(); }

    void range(uint16_t v) { value.set(d::range_pdu1(), v); }

#if FEATURE_EMBR_J1939_DATAFIELD_AUTOINIT == 0
protected:
    pdu1_header() = default;
#endif
};

struct pdu2_header : can_id
{
    friend class internal::pdu_header;

    constexpr explicit pdu2_header(uint32_t v) : can_id(v) {}

    constexpr pdu2_header(uint8_t priority, pgns pgn) :
        can_id{((uint32_t)priority << d::priority().bitpos) | (((uint32_t)pgn) << d::range_pdu2().bitpos)}
    {}

    template <pgns pgn>
    static constexpr pdu2_header create()
    {
        return pdu2_header(pgn::traits<pgn>::default_priority, pgn);
    }

    // Full 18-bit PGN range
    constexpr uint32_t range() const { return range_pdu2(); }

    void range(uint32_t v) { value.set(d::range_pdu2(), v); }

#if FEATURE_EMBR_J1939_DATAFIELD_AUTOINIT == 0
protected:
    pdu2_header() = default;
#endif
};

constexpr bool is_bam(const pdu1_header& id)
{
    return id.destination_address() == addresses::global;
}

namespace internal {

#if FEATURE_EMBR_J1939_DATAFIELD_AUTOINIT == 0
class pdu_header
{
public:
    // DEBT: Depends on type-punning which is not gaurunteed by c++ spec
    // word on the street is GCC does gauruntee it (cite reference)
    union
    {
        pdu1_header pdu1;
        pdu2_header pdu2;
        can_id id;
    };

    pdu_header() = default;
    constexpr explicit pdu_header(can_id id) : id{id}   {}
};
#endif

}

}}
