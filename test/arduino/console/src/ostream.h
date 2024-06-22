#pragma once

#include <estd/ostream.h>

// This saves ~1.2k of ROM space on AVR.  Achieves this by:
// - assumes uppercase hex is always wanted
// - always pushes integer conversions through maximum base (36 theoretical, 16 practical)
//   vs. branching off for <= 10 vs > 10
#define USE_ALT_POLICY 1
template <class Streambuf, template <class, class> class Policy, class Locale = estd::internal::default_locale>
using basic_ostream_with_policy =
    estd::detail::basic_ostream<Streambuf,
        estd::internal::basic_ios<Streambuf, false, Policy<Streambuf, Locale> > >;

#if USE_ALT_POLICY
template <class Streambuf, class Locale>
struct alt_policy : estd::internal::ios_base_policy<Streambuf, Locale>
{
    using cbase_policies = estd::internal::cbase_policies;

    static constexpr cbase_policies cbase_policy =
        // TODO: Add system wide defaults overrides for these policies to augment
        // these per-ostream policy
        cbase_policies(
            cbase_policies::CBASE_POLICY_CASE_UPPER |
            // TODO: Add a CBASE_POLICY_HEX_NEVER
            cbase_policies::CBASE_POLICY_HEX_ALWAYS);
};

using our_arduino_ostream = basic_ostream_with_policy<estd::arduino_ostreambuf, alt_policy>;

#else
using our_arduino_ostream = estd::arduino_ostream;
#endif
