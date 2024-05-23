#include <catch2/catch.hpp>

#include <j1939/state-machines/transport_protocol.hpp>
#include <j1939/ca.hpp>

#include <can/loopback.h>

using namespace embr::j1939;
using namespace embr::j1939::sm::v0;

TEST_CASE("transport protocol (J1939-21 Section 5.10)")
{
    embr::can::loopback_transport t;
    embr::can::loopback_transport::frame frame;

    SECTION("core")
    {
        transport_protocol tp_orig, tp_recv;
        constexpr unsigned sz = 64;

        {
            pdu<pgns::tp_cm> cm;

            tp_orig.initiate_originator();

            cm.total_packets((sz + 7) / 7);
            cm.total_size(sz);
            cm.control(pdu<pgns::tp_cm>::rts);
            // DEBT: Consider doing the namespace, non-class enum trick -- though for
            // addresses, it's a minor edge case to explicitly say null_address like this
            cm.destination_address((uint8_t)addresses::null_address);
            cm.source_address((uint8_t)addresses::null_address);

            tp_recv.process_incoming(t, cm);
            tp_recv.process_outgoing(t, 0);

            REQUIRE(t.receive(&frame));

            process_incoming(tp_orig, t, frame);

            REQUIRE(tp_orig.state() == transport_protocol::ORIGINATOR_RECEIVED_CTS);
        }
    }
}