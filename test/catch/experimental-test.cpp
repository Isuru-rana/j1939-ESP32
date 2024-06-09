#include <iostream>
#include <map>

#include <embr/units/feet.h>

#include <j1939/data_field/cm1.hpp>
#include <j1939/data_field/lighting_command.hpp>
#include <j1939/data_field/lighting_data.hpp>
#include <j1939/data_field/oel.hpp>

#include <j1939/internal/decompose.h>

#include <j1939/units/time.h>

#include <j1939/state-machines/string_gatherer.hpp>

#include <j1939/pdu.h>

#include <embr/bits/bits.hpp>

//#include <embr/services.hpp>

#include "test-data.h"

using namespace embr;

#if __cpp_fold_expressions

template <j1939::spns ...spns>
void helper(estd::variadic::values<j1939::spns, spns...>, std::string& s)
{
    (s += ... += (std::string(j1939::spn::type_traits<spns>::name()) + ", "));
}

struct Helper1
{
    std::map<std::string, std::string> properties_;

    template <class T, j1939::spns spn>
    void operator()(j1939::spn::traits<spn>, const T& v)
    {
        using traits = j1939::spn::traits<spn>;
        std::string value = std::to_string(int(v));
        constexpr const char* name = traits::name();

        if constexpr(name == nullptr)
        {
            estd::layer1::string<32> key("spn");

            key += estd::to_string(int(spn));

            //std::string key = "spn";
            //key += std::to_string(int(spn));

            // Actually works really well.  I find myself preferring the more runtime-y one
            //constexpr const char* key = j1939::internal::v1::to_string<int(spn)>;

            properties_[key.data()] = value;
        }
        else
            properties_[name] = value;
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
        Helper1 h;

        SECTION("constexpr stuff")
        {
            using traits = j1939::pgn::traits<j1939::pgns::oel>;
            using spns = traits::spns;
            std::string s;

            helper(spns{}, s);
            //std::cout << s <<std::endl;
            REQUIRE(s == "turn_signal_switch, high_low_beam_switch, work_light_switch, "
                         "main_light_switch, hazard_light_switch, operators_desired_delay_lamp_off_time, ");
        }
        SECTION("pdu: oel")
        {
            j1939::pdu<j1939::pgns::oel> pdu{j1939::null_t{}};

            decompose(pdu, h);

            std::string v = h.properties_["turn_signal_switch"];

            REQUIRE(v == "15");
        }
        SECTION("pdu: lighting command")
        {
            j1939::pdu<j1939::pgns::lighting_command> pdu{j1939::null_t{}};

            pdu.left_turn_signal(j1939::spn::control_commands::enable);

            decompose(pdu, h);

            std::string v = h.properties_["left_turn_signal"];

            REQUIRE(v == "1");
        }
#endif
    }
}
