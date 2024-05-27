#pragma once

#include "enum.h"
#include "feature.h"

namespace embr { namespace j1939 { namespace sm { namespace tp { inline namespace v0 {

template <class TimePoint>
struct context
{
    using time_point = TimePoint;

    const time_point current;
    const uint8_t self_address;
#if FEATURE_EMBR_J1939_TP_CONTEXT_NEXT
    time_point* const next;

        constexpr context(time_point current, uint8_t sa, time_point* next = nullptr) :
            current{current},
            self_address{sa},
            next{next}
        {}
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