#pragma once

#include "enum.h"
#include "feature.h"
#include "context.h"

namespace embr { namespace j1939 { namespace sm { namespace tp { inline namespace v0 {

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
