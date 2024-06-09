#include <iostream>
#include <map>

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

template <j1939::spns s>
bool helper3()
{
    using namespace j1939;

    using t1 = spn::traits<s>;
    constexpr spn::descriptor d = spn::get_descriptor<s>();

    INFO("name" << t1::name())

    return {};
}

template <j1939::spns ...spns>
void helper(estd::variadic::values<j1939::spns, spns...>)
{
    (... && helper3<spns>());
}

struct Helper1
{
    template <class Container>
    using dfb = const j1939::internal::data_field_base<Container>;

    std::map<std::string, std::string> properties_;

    template <j1939::spns s, class Container>
    bool decompose(dfb<Container>& d)
    {
        using traits = j1939::spn::traits<s>;
        std::string value;

        auto v = d.template get<s>();
        value = std::to_string(int(v));

        properties_[traits::name()] = value;

        return true;
    }

    template <j1939::spns ...spns, class Container>
    void decompose(estd::variadic::values<j1939::spns, spns...>, dfb<Container>& d)
    {
        (... && decompose<spns>(d));
    }


    template <j1939::pgns pgn, class Container>
    void decompose(const j1939::data_field<pgn, Container>& d)
    {
        using traits = j1939::pgn::traits<pgn>;
        using spns = typename traits::spns;

        decompose(spns{}, d);
    }
};

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
        SECTION("constexpr stuff")
        {
            using traits = j1939::pgn::traits<j1939::pgns::oel>;
            using spns = traits::spns;
            std::string s;

            helper(spns{});
            helper2<spns>::dostuff(s);
            //std::cout << s <<std::endl;
            REQUIRE(s == "turn_signal_switch, high_low_beam_switch, work_light_switch, "
                         "main_light_switch, hazard_light_switch, operators_desired_delay_lamp_off_time, ");
        }
        SECTION("pdu")
        {
            Helper1 h;
            j1939::pdu<j1939::pgns::oel> pdu{j1939::null_t{}};

            h.decompose(pdu);

            std::string v = h.properties_["turn_signal_switch"];

            REQUIRE(v == "15");
        }
#endif
    }
}
