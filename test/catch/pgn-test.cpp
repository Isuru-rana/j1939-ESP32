#include <j1939/data_field/disp1.hpp>
#include <j1939/data_field/bjm1.hpp>
#include <j1939/data_field/oel.hpp>
#include <j1939/data_field/lighting_command.hpp>
#include <j1939/data_field/vep1.hpp>
#include <j1939/data_field/bt1.hpp>
#include <j1939/data_field/network.hpp>
#include <j1939/data_field/transport_protocol.hpp>
#include <j1939/data_field/time.hpp>


#include <j1939/pgn.h>
#include <j1939/spn.h>
#include <j1939/dispatcher.hpp>

#include <j1939/spn/descriptors.h>

#include "test-data.h"

using namespace embr::j1939;
using namespace embr::units::literals;

#include "macro/push.h"

TEST_CASE("pgn")
{
    SECTION("data_field")
    {
        SECTION("bjm1")
        {
            data_field<pgns::basic_joystick_message_1> data{null_t{}};
            const auto& raw = data.data_;

            // 35 = 0x23
            data.joystick1_x_axis_position(35);
            data.joystick1_y_axis_position(995);
            data.button1_pressed(spn::measured::on);

            REQUIRE(raw[2] == 0xFF);
            REQUIRE(raw[4] == 0xFF);

            auto v = data.joystick1_x_axis_position();

            REQUIRE(v == 3.5_pct);

            v = data.joystick1_y_axis_position();

            REQUIRE(v == 99.5_pct);

            REQUIRE(data.button1_pressed() == spn::measured::on);
            REQUIRE(data.button2_pressed() == spn::measured::not_available);
        }
        SECTION("disp1")
        {
            data_field<pgns::disp1> data{null_t{}};
        }
        SECTION("oel")
        {
            data_field<pgns::oel> data{null_t{}};
            data.turn_signal_switch();
            data.high_low_beam_switch(enum_type<spns::high_low_beam_switch>::high_beam_selected);
            //data.();
        }
        SECTION("lighting_command")
        {
            data_field<pgns::lighting_command> data{null_t{}};

            uint16_t val = data.get_raw<spns::left_turn_signal_lights_cmd>();

            REQUIRE(val == 0b11);

            data.set<spns::left_turn_signal_lights_cmd>(0);
            val = data.get_raw<spns::left_turn_signal_lights_cmd>();

            REQUIRE(val == 0);
        }
        SECTION("vehicle_electrical_power_1")
        {
            data_field<pgns::vehicle_electrical_power_1> data{null_t{}};
            typedef spn::traits<spns::battery_potential> traits_type;
            constexpr embr::units::millivolts<uint16_t> v1{25000};
            constexpr unit_type<spns::battery_potential> v2{v1};

            data.battery_potential(500);
            auto v = data.battery_potential();

            REQUIRE(v.count() == 500);

            data.battery_potential(v1);

            REQUIRE(v.count() == 500);
            REQUIRE(v.count() == v2.count());

            embr::units::volts<int> _v{v};

            REQUIRE(_v.count() == 25);
        }
        SECTION("transport_protocol")
        {
            data_field<pgns::tp_cm> data{null_t{}};

            // FIX: Something is wrong with underlying constexpr's
            data.control(data.cts);

            auto c = data.control();

            auto ts = data.total_size();

            data.pgn(20);
            data.max_packets(5);

            uint32_t _pgn = data.pgn();

            // FIX: Failing likely because we aren't passing '24' to 'width'
            REQUIRE(_pgn == 20);

            REQUIRE(data.max_packets() == 5);
        }
        SECTION("NMEA 2000")
        {
            SECTION("switch_bank_status")
            {
                data_field<pgns::switch_bank_status> data{null_t{}};

                bool v = data[4];
            }
        }
        SECTION("time_date")
        {
            data_field<pgns::time_date> data{null_t{}};
            embr::units::days<int> d(10);

            data.seconds(3);
            data.minutes(3);
            data.hours(11);

            // NOTE: For some reason, they elected to go to 1/4 resolution with days, so this represents
            // one day
            data.day(4);

            REQUIRE(data.seconds().count() == 3);
            REQUIRE(data.day().count() == 4);

            embr::units::hours<int> h(data.day());

            REQUIRE(h.count() == 24);

            data.day(d);

            h = data.day();

            // 10 days = 240 hours
            REQUIRE(h.count() == 240);

            data.month(5);

            using local_hour_offset_traits = spn::type_traits<spns::local_hour_offset>;

            data.local_hour_offset(local_hour_offset_traits::raw::min());

            auto _h = data.local_hour_offset();
            h = _h;

            REQUIRE(h.count() == local_hour_offset_traits::cooked::min());
        }
    }
    SECTION("layer2 data_field")
    {
        uint8_t buf[32];
        //layer2::data_field<pgns::lighting_command> data(buf);

    }
    SECTION("traits")
    {
        std::string s;
        const char* s_;

        SECTION("joystick")
        {
            // No bueno, due to
            // https://stackoverflow.com/questions/8016780/undefined-reference-to-static-constexpr-char
            /*
            using traits = pgn::traits<pgns::basic_joystick_message_1>;

            const char* const s2_ = traits::abbrev;
            s = s_;

            REQUIRE(s == "BJM1"); */
        }
    }
}

#include "macro/pop.h"
