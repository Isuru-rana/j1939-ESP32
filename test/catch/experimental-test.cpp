#include <iostream>

#include <embr/units/feet.h>

#include <j1939/data_field/cm1.hpp>
#include <j1939/data_field/oel.hpp>

#include <j1939/units/time.h>

#include <j1939/state-machines/string_gatherer.hpp>

#include <j1939/pdu.h>

#include <embr/bits/bits.hpp>

//#include <embr/services.hpp>

#include "test-data.h"

using namespace embr;

#if __cpp_fold_expressions
template <j1939::spns ...spns>
void helper(estd::variadic::values<j1939::spns, spns...>)
{

}

template <class T>
struct helper2;

template <j1939::spns ...spns>
struct helper2<estd::variadic::values<j1939::spns, spns...>>
{
    static void dostuff(std::string& s)
    {
        (s += ... += (std::string(j1939::spn::type_traits<spns>::name()) + ", "));
    }
};
#endif

TEST_CASE("experimental")
{
    SECTION("A")
    {
        // TODO: move this out to embr
        using unit = units::miles<int>::per<units::hours<int> >;

        unit mph{0};
    }
    SECTION("state machines")
    {
        j1939::data_field<j1939::pgns::tp_dt>
            v1{j1939::null_t{}},
            v2{j1939::null_t{}},
            v3{j1939::null_t{}};

        SECTION("string gatherer")
        {
            estd::copy_n("hi2u*", 5, v1.packetized_data());

            j1939::string_gatherer<estd::layer1::string<32> > sg;

            sg.process_incoming(v1);

            REQUIRE(sg.s.size() == 4);
            REQUIRE(sg.s == "hi2u");
        }
    }
    /*
     * obsolete
    SECTION("services")
    {
        const int id = 0;
        services::manager.get(id);
    }   */
    SECTION("names from spns")
    {
#if __cpp_fold_expressions
        using traits = j1939::pgn::traits<j1939::pgns::oel>;
        using spns = traits::spns;
        std::string s;

        // bug in estd prohibits this
        //helper<spns>(spns{});
        helper2<spns>::dostuff(s);
        std::cout << s <<std::endl;
        REQUIRE(s == "turn_signal_switch, high_low_beam_switch, work_light_switch, "
                     "main_light_switch, hazard_light_switch, operators_desired_delay_lamp_off_time, ");
#endif
    }
}
