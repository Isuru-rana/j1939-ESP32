#include "test-data.h"

#include <estd/tuple.h>
#include <estd/internal/variadic.h>

#include <can/loopback.h>

#include <j1939/cs/aggregate.h>
#include <j1939/cs/base.h>
#include <j1939/state-machines/lcmd.hpp>
#include <j1939/internal/dispatcher/dispatch.hpp>

using namespace embr;
using namespace j1939;

// DEBT: State machine catch-all at the moment

// EXPERIMENTAL
estd::tuple<j1939::cs::v1::base, j1939::cs::v1::base> t1;

TEST_CASE("controller subsystems")
{
    using transport_type = can::loopback_transport;
    using frame_traits = j1939::frame_traits<transport_type::frame>;
    transport_type t;
    transport_type::frame frame;
    using clock = std::chrono::steady_clock;
    using time_point = clock::time_point;
    using milliseconds = std::chrono::milliseconds;
    time_point now;

#if __cpp_fold_expressions
#endif
    SECTION("lcmd state machine")
    {
        using ctx = sm::v0::context<time_point>;

        sm::v0::lighting_command<time_point> lcmd;
        ctx context{now};

        SECTION("oel input")
        {
            pdu<pgns::oel> oel{null_t{}};

            SECTION("blinkers")
            {
                oel.turn_signal_switch(enum_type<spns::turn_signal_switch>::left_turn_to_be_flashing);

                lcmd.process_incoming(t, oel, context);

                REQUIRE(lcmd.state() == lcmd.STATE_FLASH_ON);
            }
            SECTION("hazards")
            {
                oel.hazard_light_switch(spn::measured::on);

                lcmd.process_incoming(t, oel, context);

                REQUIRE(lcmd.state() == lcmd.STATE_FLASH_ON);

                REQUIRE(t.receive(&frame));

                // DEBT: No overload yet for std::chrono - estd::chrono
                now += milliseconds{2};

                REQUIRE(t.peek() == nullptr);

                lcmd.process_outgoing(t, ctx{now});

                REQUIRE(t.peek() == nullptr);

                now += lcmd.flash_delay();

                lcmd.process_outgoing(t, ctx{now});

                REQUIRE(t.receive(&frame));
            }
        }
        SECTION("ccvs input")
        {
            pdu<pgns::ccvs> ccvs;

            ccvs.brake_switch(spn::measured::on);

            lcmd.process_incoming(t, ccvs, context);
        }
    }
    SECTION("aggregate")
    {
        estd::tuple<sm::v0::lighting_command<time_point> > aggregate;
        pdu<pgns::oel> oel{null_t{}};
        auto f = frame_traits::create(oel);

        // DEBT: May be better suited in dispatcher area
        //using functor = j1939::cs::internal::v1::incoming_visitor<transport_type>;

        //aggregate.visit(functor{t}, f);
    }
}
