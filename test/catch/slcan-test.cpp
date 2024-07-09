#include <catch2/catch.hpp>

#include <estd/sstream.h>

#include <can/loopback.h>
#include <can/internal/slcan/parser.hpp>

#include <j1939/data_field/time.hpp>
#include <j1939/pdu.h>

#include "macro/push.h"


TEST_CASE("slcan")
{
    using parser_type = embr::can::slcan::parser<>;
    parser_type p;
    parser_type::frame_type frame;
    estd::layer1::stringstream<64> ss;
    const auto& s = ss.rdbuf()->str();

    SECTION("status")
    {
        p.status(ss);

        REQUIRE(s == "fC---0\r");
    }
    SECTION("parse")
    {
        SECTION("open")
        {

        }
        SECTION("close")
        {
            const char* cmd = "C";
            p.parse(cmd, ss);

            REQUIRE(s[0] == '\7');
            REQUIRE(s.length() == 1);
        }
        SECTION("version")
        {
            p.parse("V", ss);

            REQUIRE(s == "V0001\r");
        }
    }
    SECTION("deserialize")
    {
        estd::errc ec = p.deserialize("0000000A412345678", &frame, true);

        REQUIRE(ec == 0);

        REQUIRE(frame.id == 10);
        REQUIRE(frame.dlc == 4);
        REQUIRE(frame.payload[0] == 0x12);
        REQUIRE(frame.payload[3] == 0x78);

        // same as esp32/slcan/test-emit
        ec = p.deserialize("1BFFFF8080123456789ABCDEF", &frame, true);

        REQUIRE(ec == 0);

        REQUIRE(frame.id == 0x1BFFFF80);
        REQUIRE(frame.dlc == 8);
        REQUIRE(frame.payload[0] == 0x01);
        REQUIRE(frame.payload[3] == 0x67);
        REQUIRE(frame.payload[7] == 0xEF);
    }
    SECTION("serialize")
    {
        SECTION("general")
        {
            frame.id = 0x12345;
            frame.extended = true;
            frame.dlc = 3;
            frame.payload[0] = 0x12;
            frame.payload[1] = 0x34;
            frame.payload[2] = 0x56;
            //char s[64];
            p.serialize(frame, ss);
            REQUIRE(s == "T000123453123456\r");
        }
        SECTION("j1939 time/date")
        {
            using namespace embr;
            j1939::pdu<j1939::pgns::time_date> pdu(0xB5, j1939::null_t{});
            using frame_traits = j1939::frame_traits<parser_type::frame_type>;

            pdu.hours(0);
            pdu.minutes(0);
            pdu.seconds(10);

            frame = frame_traits::create(pdu);

            // DEBT: Loopback create doesn't seem to work out extended/normal frame yet
            frame.extended = true;

            p.serialize(frame, ss);

            // FIX: Doesn't emit right amount of characters.  Smells like a padding problem
            REQUIRE(s == "T18FEE6B580A00FFFFFFFFFF\r");
        }
    }
}

#include "macro/pop.h"
