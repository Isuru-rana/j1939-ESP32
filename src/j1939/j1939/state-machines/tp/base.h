#pragma once

#include "enum.h"
#include "feature.h"

namespace embr { namespace j1939 { namespace sm { namespace tp { inline namespace v0 {

// Consider a specialization ala "include_next" rather than a feature flag
template <class TimePoint>
struct context
{
    using time_point = TimePoint;

    const time_point current;
    const uint8_t self_address = addresses::null;
#if FEATURE_EMBR_J1939_TP_CONTEXT_NEXT
    time_point* const next_;

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


template <class TimePoint>
inline void prep_abort(
    pdu<pgns::tp_cm>& cm,
    const context<TimePoint>& ctx,
    enum_base::abort_reasons r)
{
    cm.source_address(ctx.self_address);
    cm.control(enum_base::modes::abort);
    cm.abort_reason(r);
}

}}}}}