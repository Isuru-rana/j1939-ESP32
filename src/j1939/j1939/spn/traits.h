/**
 * References
 *
 * 1. J1939-71 (REV. DEV2003) section 5.1.4
 * 2. sae_j_1939_spreadsheet_supported_by_TwidoExtreme.xls
 */
#pragma once

#include "fwd.h"
#include "ranges.h"
#include "../slots/traits.h"

#include "../slots/fwd.h"
#include "../units/fwd.h"
#include "../pdu/header.h"

#include <estd/cstdint.h>

#include <embr/units/fwd.h>

#include <estd/internal/macro/push.h>

// Arduino compensators
// DEBT: Consolidate with estd/embr variety
#ifdef FEATURE_PRAGMA_PUSH_MACRO
#pragma push_macro("word")
#undef word
#endif


// All code here is in support of [1]
namespace embr { namespace j1939 { namespace spn {

template <typename TInt>
struct range_traits
{
    typedef ranges::valid_signal<TInt> valid_signal_type;
    typedef ranges::error_indicator<TInt> error_indicator_type;
    typedef ranges::parameter_specific_indicator<TInt> parameter_specific_type;

    static bool valid_signal(TInt v)
    {
        return v >= valid_signal_type::min() && v <= valid_signal_type::max();
    }

    static bool error(TInt v)
    {
        return v >= error_indicator_type::min() && v <= error_indicator_type::max();
    }

    static bool parameter_specific(TInt v)
    {
        return v >= parameter_specific_type::min() && v <= parameter_specific_type::max();
    }
};


namespace internal {


// EXPERIMENTAL
enum traits_enum
{
    TRAITS_NONE     =   0x0000,
    TRAITS_ENUM     =   0x0001,
    TRAITS_ASCII    =   0x0002,
    TRAITS_NAME     =   0x0003
};

// DEBT: Might be better off deducing these traits, and also we need to cite where in
// documentation these general behaviors reside

template <>
struct numeric_traits<2>
{
    static constexpr uint8_t off = 0;
    static constexpr uint8_t err = 2;
    static constexpr uint8_t noop = 3;
};


template <>
struct numeric_traits<4>
{
    static constexpr uint8_t off = 0;
    static constexpr uint8_t err = 0b1110;
    static constexpr uint8_t noop = 15;
};


template <>
struct numeric_traits<8>
{
    static constexpr uint8_t off = 0;
    static constexpr uint8_t err = 0xFE;
    static constexpr uint8_t noop = 0xFF;
};

template <>
struct numeric_traits<10>
{
    static constexpr uint16_t err = 0x3FE;
    static constexpr uint16_t noop = 0x3FF;
};


template <>
struct numeric_traits<16>
{
    static constexpr uint16_t off = 0;
    static constexpr uint16_t noop = 0xFFFF;
};

// DEBT: Document why we like the option of an unshifted compare
template <unsigned N, typename Int>
constexpr bool noop(Int v, unsigned bitpos)
{
    return (v ^= numeric_traits<N>::noop << (bitpos - 1)) == 0;
    //return v == numeric_traits<N>::noop << (bitpos - 1);
}

// Yields matching int_type and value_type
template <class Int>
struct type_traits_base
{
    typedef Int int_type;
    typedef int_type value_type;

    // EXPERIMENTAL, name of spn - can deviate slightly as long as it is uniquely
    // identifiable within context of associated pdu/pgn
    static constexpr const char* name() { return nullptr; }
    static constexpr const char* description() { return nullptr; }

    // Very short version of name for impacted UI use.  Rules:
    // 1. SHOULD be under 8 characters
    // 2. MUST NOT be same as name() in:
    //    a. PGN-adjacent spns (prefer leave as nullptr if needed)
    //    b. Current spn
    static constexpr const char* short_name() { return nullptr; }

    // EXPERIMENTAL -
    // true here so that specializers can easily derive from this
    static constexpr bool is_specialized = true;
    static constexpr traits_enum features = TRAITS_NONE;
};

// Overrides value_type with enum_type
template <class Enum, class Int = uint8_t>
struct enum_traits_base : type_traits_base<Int>
{
    typedef Enum enum_type;
    typedef Enum value_type;

    // EXPERIMENTAL
    static constexpr traits_enum features =
        //type_traits_base<Int>::features |
        TRAITS_ENUM;
};

// helper for status command types
using status_type_traits = enum_traits_base<control_commands>;

// helper for measured command types
using measured_type_traits = enum_traits_base<discrete_parameters>;

// TODO: Does a whole lotta nothing right now, but still makes a good placeholder
template <unsigned N, char delimiter_ = '*'>
struct ascii_type_traits
{
    static constexpr unsigned max_len = N;
    static constexpr char delimiter = delimiter_;

    // EXPERIMENTAL, name of spn
    static constexpr const char* name() { return nullptr; }
    static constexpr const char* description() { return nullptr; }

    // EXPERIMENTAL
    static constexpr traits_enum features = TRAITS_ASCII;
};


// Maybe relates to SAEsa01
// See J1939-81 (2003) 4.1.2
struct address_type_traits_base :
    internal::type_traits_base<uint8_t>,
    j1939::internal::address_type_traits_base
{
    // EXPERIMENTAL
    static constexpr traits_enum features = TRAITS_NAME;
};


}

// TODO: Put a constexpr spn member var in here
template <spns spn>
struct traits :
    type_traits<spn>,
    range_traits<typename type_traits<spn>::int_type>
{
    static constexpr spn::descriptor d = spn::get_descriptor<spn>();
    static constexpr spn::descriptor descriptor() { return spn::get_descriptor<spn>(); }

    /// Indicate whether specified value is the "no action" value, which is always
    /// all-bits-set.  Default behavior = pass in value *unshifted* from data stream
    /// @param v
    /// @param autoshift if you have shifted the data to comfortable 0-position yourself, set this to false.
    /// @return
    constexpr static bool noop(typename type_traits<spn>::int_type v, bool autoshift = true)
    {
        return internal::noop<d.length>(v, autoshift ? d.bitpos : 1);
    }

    // EXPERIMENTAL
    template <typename Rep, class Period, class Tag,
        ESTD_CPP_CONCEPT(estd::internal::units::Adder<Rep>) F>
    //constexpr
    static bool noop(embr::units::detail::unit<Rep, Period, Tag, F> v)
    {
        using helper = ranges::not_available<Rep>;
        // Presumes we're dealing with an embr::units type
        using unit_type = typename type_traits<spn>::value_type;

        // DEBT: The way we filter here is clumsy.  We'd prefer to do this up at
        // the function header
        static_assert(estd::is_same<decltype(v), unit_type>::value, "Queried type must match slot/unit type");

        Rep rc = v.root_count();

        return helper::is_not_available(rc);
    }
};


/// Generic catch-all - reports 8-bit every time
/// @remarks particular pdu .hpp files are expected to specialize this
template <spns spn>
struct type_traits :
    internal::type_traits_base<uint8_t>,
    intrinsic_tag
{
    // EXPERIMENTAL
    static constexpr bool is_specialized = false;
};


}}}

#include <estd/internal/macro/pop.h>

#ifdef FEATURE_PRAGMA_PUSH_MACRO
#pragma pop_macro("word")
#endif
