#include "test-data.h"

#include <estd/tuple.h>
#include <estd/internal/variadic.h>

#include <can/loopback.h>

#include <j1939/cs/base.h>
#include <j1939/state-machines/lcmd.hpp>
#include <j1939/internal/dispatcher/incoming.hpp>

using namespace embr;

// DEBT: State machine catch-all at the moment

// EXPERIMENTAL
estd::tuple<j1939::cs::v1::base, j1939::cs::v1::base> t1;

TEST_CASE("controller subsystems")
{
    can::loopback_transport t;
    using clock = std::chrono::steady_clock;
    using time_point = clock::time_point;
    time_point now;

#if __cpp_fold_expressions
#endif
    SECTION("lcmd state machine")
    {
        using namespace j1939;

        sm::v0::lighting_command<time_point> lcmd;
        sm::v0::context<time_point> context{now};

        pdu<pgns::oel> oel;

        oel.turn_signal_switch(enum_type<spns::turn_signal_switch>::left_turn_to_be_flashing);

        lcmd.process_incoming(t, oel, context);

        REQUIRE(lcmd.state() == lcmd.STATE_FLASH_ON);
    }
}
