#pragma once

#include <estd/cstdint.h>

#include "../../addresses.h"

#include "feature.h"

namespace embr { namespace j1939 { namespace sm { inline namespace v0 {

// Consider a specialization ala "include_next" rather than a feature flag
template <class TimePoint>
struct context
{
    using time_point = TimePoint;
    using duration = typename time_point::duration;

    const time_point current;
    const uint8_t self_address = addresses::null;
#if FEATURE_EMBR_J1939_TP_CONTEXT_NEXT
    time_point* const next_;

#if UNIT_TESTING
    // NOTE: Avoid production use (time_point preferred)
    template <class Rep, class Period>
    constexpr context(estd::chrono::duration<Rep, Period> current, uint8_t sa = addresses::null, time_point* next = nullptr) :
        current{current},
        self_address{sa},
        next_{next}
    {}
#endif

    constexpr context(time_point current, uint8_t sa = addresses::null, time_point* next = nullptr) :
        current{current},
        self_address{sa},
        next_{next}
    {}

    // EXPERIMENTAL
    template <class Duration>
    void next(Duration next_delta) { *next_ += next_delta; }

#else
    template <class Duration>
    static constexpr bool next(Duration) { return {}; }
#endif
};



}}}}
