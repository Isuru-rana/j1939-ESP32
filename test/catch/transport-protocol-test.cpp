#include <catch2/catch.hpp>

#include <j1939/state-machines/transport_protocol.hpp>
#include <j1939/ca.hpp>

#include <can/loopback.h>

#include "test-data.h"

using namespace embr::j1939;
using namespace embr::j1939::sm::v0;

TEST_CASE("transport protocol (J1939-21 Section 5.10)")
{
    embr::can::loopback_transport t;
    embr::can::loopback_transport::frame frame;

    SECTION("core")
    {
        transport_protocol::context ctx{0, uint8_t(addresses::null_address)};
        transport_protocol tp_orig, tp_recv;
        constexpr unsigned sz = sizeof(test::test_str2) - 1;    // Zapping null terminator

        {
            tp_orig.initiate_originator(sz, ctx);
            tp_orig.process_outgoing(t, ctx);

            REQUIRE(t.receive(&frame));

            process_incoming(tp_recv, t, frame, ctx);

            tp_recv.process_outgoing(t, ctx);

            REQUIRE(t.receive(&frame));

            process_incoming(tp_orig, t, frame, ctx);

            REQUIRE(tp_orig.state() == transport_protocol::ORIGINATOR_RECEIVED_CTS);
        }

        REQUIRE(t.peek() == nullptr);

        {
            tp_orig.payload((uint8_t*)test::test_str2);
            tp_orig.process_outgoing(t, ctx);

            REQUIRE(t.receive(&frame));

            process_incoming(tp_recv, t, frame, ctx);

            REQUIRE(tp_recv.state() == transport_protocol::RESPONDER_RECEIVING_DT);

            REQUIRE(tp_recv.established().remaining_bytes() == sz - 7);

            auto data = (char*)tp_recv.payload().data();
            // FIX: It appears fixed-size string pointer doesn't work
            //estd::layer2::basic_string<char, 7, false> s{data};
            //REQUIRE(s == "abcdefg");
            REQUIRE(memcmp(data, "abcdefg", 7) == 0);

            REQUIRE(tp_recv.state() == transport_protocol::RESPONDER_RECEIVED_DT);
        }

        {
            tp_orig.payload((uint8_t*)test::test_str2 + 7);
            tp_orig.process_outgoing(t, ctx);

            REQUIRE(t.receive(&frame));

            process_incoming(tp_recv, t, frame, ctx);

            REQUIRE(tp_recv.state() == transport_protocol::RESPONDER_RECEIVING_DT);

            REQUIRE(tp_recv.established().remaining_bytes() == sz - 14);

            auto data = (char*)tp_recv.payload().data();

            REQUIRE(memcmp(data, "hijklmn", 7) == 0);

            REQUIRE(tp_recv.state() == transport_protocol::RESPONDER_RECEIVED_DT);
        }
    }
}
