#pragma once

#include <can/fwd.h>    // for ATTR_NODISCARD

#include "fwd.h"
#include "result.h"

namespace embr { namespace j1939 { namespace sm { inline namespace v0 {

// EXPERIMENTAL
class base
{
public:
    template <class ...Args>
    static constexpr result process(Args&&...)
    {
        return result::ignore();
    }
};


// DEBT: Move this into sm::v1 once FEATURE_EMBR_J1939_TP_FUTURE is fully settled down
// (we're close)
// DEBT: c++20 concept feels appropriate here
template <class TimePoint>
class to_schedule
{
public:
    using time_point = TimePoint;
    using duration = typename time_point::duration;

protected:
    time_point next_event_;

public:
    ATTR_NODISCARD constexpr time_point next_event() const { return next_event_; }

    // DEBT: Experimenting, may want to change name.  May prefer old one with
    // explicit duration on it
    ATTR_NODISCARD constexpr bool elapsed(const sm::v0::context<TimePoint>& ctx) const
    {
        // time_point{} = zero, effectively

        return ctx.current >= next_event_ && next_event_ != time_point{};
    }
};


}}}}
