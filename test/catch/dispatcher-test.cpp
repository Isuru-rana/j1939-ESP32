#include <map>

#include <estd/sstream.h>

#include <embr/observer.h>

// FIX: This guy *must* appear beforce 'dispatch.hpp' otherwise test-cs flips out
#include <j1939/data_field/oel.hpp>

// 11JUN24 New flavor
#include <j1939/internal/dispatcher/dispatch.hpp>


// 11JUN24 Such an early take on this, I forgot all about this guy
#include <j1939/internal/dispatcher/subject.h>

#include <can/loopback.h>

#include <j1939/internal/dispatcher/incoming2.hpp>


#include "test-data.h"
#include "test-cs.h"

using namespace embr;
using namespace embr::j1939;

struct dispatch_functor
{
    template <pgns pgn>
    int operator()(j1939::internal::in_place_pgn<pgn>) const
    {
#if FEATURE_EMBR_J1939_NO_TRAITS_WRAPPER
        using traits = j1939::pgn::traits<pgn>;

        return traits::is_specialized;
#else
        using traits = j1939::internal::traits_wrapper<pgn>;

        return traits::specialized;
#endif

    }

    int operator()(pgns) { return 0; }
};

struct SyntheticObserver
{
    int marker_ = 7;
    std::map<pgns, int> counters;
    int marker2_ = 77;

    void on_notify(events::received<pgns::oel> e)
    {
        ++counters[pgns::oel];
    }

    void on_notify(events::received<pgns::lighting_command> e)
    {
        ++counters[pgns::lighting_command];
    }
};


TEST_CASE("dispatcher")
{
    SECTION("early version")
    {
        pdu2_header id{0};

        id.range((uint32_t)pgns::oel);

        auto s = embr::layer1::make_subject(SyntheticObserver());

        dispatch(id, blinker_on, s);

        auto& o = estd::get<0>(s.observers());

        int counter = o.counters[pgns::oel];

        REQUIRE(counter == 1);

        try
        {
            o.counters.at(pgns::lighting_command);
            FAIL("Should throw an exception");
        }
        catch(const std::out_of_range&)
        {

        }
    }
    SECTION("functor flavor")
    {
        pdu2_header id{0};

        id.range((uint32_t)pgns::oel);

        int specialized = j1939::v1::dispatch(dispatch_functor{}, id);

        REQUIRE(specialized == 1);
    }
    SECTION("process_incoming (v2)")
    {
        using namespace j1939::internal;
        using namespace j1939::internal::v2;
        using transport = can::loopback_transport;
        transport t;
        using frame_type = transport::frame;
        using frame_traits = frame_traits<frame_type>;
        test::SyntheticCA<transport> ca;

        pdu<pgns::oel> p(0);

        p.turn_signal_switch(enum_type<spns::turn_signal_switch>::left_turn_to_be_flashing);

        frame_type f = frame_traits::create(p);

        j1939::v2::process_incoming(ca, t, f);

        REQUIRE(ca.oel_counter == 1);

        j1939::v1::dispatch<j1939::internal::dispatch_default_policy>(
            j1939::internal::v2::specialize_frame_functor{},
            pgns::oel,
            j1939::internal::v2::test_rcv_specialized_functor{},
            f);

        specialize_frame_functor{}(in_place_pgn<pgns::oel>{}, test_rcv_specialized_functor{}, f);
    }
}
