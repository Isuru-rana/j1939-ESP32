#include <map>

#include <estd/sstream.h>

#include <embr/observer.h>

// DEBT: Be very careful, if this guy doesn't appear before "implementors" such as dispatch.hpp,
// OEL & associated specializations somehow are ignored.  I think it's related to 'Container'
#include <j1939/data_field/oel.hpp>
#include <j1939/data_field/all.hpp>

#include <j1939/data_field/base.h>

// 11JUN24 Such an early take on this, I forgot all about this guy
#include <j1939/dispatcher.hpp>

// 11JUN24 New flavor
#include <j1939/internal/dispatcher/dispatch.hpp>

#include <j1939/pgn.h>

#include <can/loopback.h>

#include "test-data.h"

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

        int specialized = j1939::internal::dispatch(dispatch_functor{}, id);

        REQUIRE(specialized == 1);
    }
}
