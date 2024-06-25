#pragma once

#include <estd/internal/utility.h>   // DEBT: Needed for complete certainty that we have std::forward.  Should be in variadic.h itself
#include <estd/internal/variadic.h>

#include "fwd.h"
#include "enum.h"

#include "../spn/enum.h"

#include "../macros/progmem.h"

namespace embr { namespace j1939 { namespace internal {

#if FEATURE_EMBR_J1939_NO_TRAITS_WRAPPER
#else
template <embr::j1939::pgns, typename = void>
struct traits_wrapper
{
    static constexpr const char specialized = false;

    static constexpr const char* name() { return "N/A"; }
    static constexpr const char* abbrev() { return "NA"; }
};

// DEBT: Move this out to an .hpp which has included a ton of stuff already
// to better check for specializations
template <embr::j1939::pgns pgn>
struct traits_wrapper<pgn, estd::enable_if_t<
    (sizeof(embr::j1939::pgn::traits<pgn>) > 0)> > :
    embr::j1939::pgn::traits<pgn>
{
    static constexpr const char specialized = true;
};
#endif


}}}

namespace embr { namespace j1939 { namespace pgn {

#if __cpp_concepts
#endif

namespace internal {

template <spns ...Values>
using spns_list = estd::variadic::values<spns, Values...>;

struct traits_base
{
    using s = embr::j1939::spns;

    static constexpr unsigned length = 8;
    // "The default for all other informational, proprietary, request, and
    // ACK messages is 6" [1] 5.2.1
    // DEBT: Decide whether we like "default" explicit name, probably leave it out
    static constexpr unsigned default_priority = 6;
    static constexpr unsigned priority = 6;

    // Designate fixed (typical, 8 byte payload) or variable (tp.cm & tp.dt)
    // NOTE: Occasionally some = true variants have fixed modes
    static constexpr bool variable = false;

    // This relies on a specialized child class inheriting it
    static constexpr bool is_specialized = true;
};

}

#if FEATURE_EMBR_J1939_NO_TRAITS_WRAPPER
template <pgns>
struct traits : internal::traits_base
{
    static constexpr bool is_specialized = false;
};
#endif

}}}
