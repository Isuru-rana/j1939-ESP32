#include <catch2/catch.hpp>

#include <j1939/state-machines/transport_protocol.hpp>
#include <j1939/ca.hpp>

#include <can/loopback.h>

#include "test-data.h"

using namespace embr::j1939;
using namespace embr::j1939::sm::v0;

// Mainly useful for testing, not so much production though bears some resemblance
// to aggregated CA handler
struct helper
{
    const uint8_t orig_sa = 1, recv_sa = 2;
    transport_protocol tp_orig, tp_recv;

    // Theory being CA/state machine should not get confused by its own traffic,
    // plus we auto aggregate to both for convenience

    template <class Transport>
    unsigned incoming(Transport& t, const typename Transport::frame& f)
    {
        // Almost there, && context makes it mad
        using ctx = transport_protocol::context;
        unsigned processed = 0;

        processed += process_incoming(tp_orig, t, f, ctx{0, orig_sa});
        processed += process_incoming(tp_recv, t, f, ctx{0, recv_sa});

        return processed;
    }

    template <class Transport>
    void outgoing(Transport&)
    {

    }
};

TEST_CASE("transport protocol (J1939-21 Section 5.10)")
{
    embr::can::loopback_transport t;
    embr::can::loopback_transport::frame frame;

    SECTION("core")
    {
        const uint8_t orig_sa = 1, recv_sa = 2;
        transport_protocol::context ctx{0, uint8_t(addresses::null_address)};
        helper h;
        transport_protocol& tp_orig = h.tp_orig;
        transport_protocol& tp_recv = h.tp_recv;

        constexpr unsigned sz = sizeof(test::test_str2) - 1;    // Zapping null terminator

        {
            tp_orig.initiate_originator(sz, {0, uint8_t(addresses::null_address)});
            tp_orig.process_outgoing(t, ctx);

            REQUIRE(t.receive(&frame));

            // FIX: invoker doesn't run as expected
            //REQUIRE(h.incoming(t, frame) == 1);

            REQUIRE(process_incoming(tp_orig, t, frame, ctx) == false); // A formality.  orig should noop here
            process_incoming(tp_recv, t, frame, ctx);
            REQUIRE(tp_orig.originator().resequence_requested() == false);

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

            REQUIRE(process_incoming(tp_orig, t, frame, ctx) == false); // A formality.  orig should noop here
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

            REQUIRE(process_incoming(tp_orig, t, frame, ctx) == false); // A formality.  orig should noop here
            process_incoming(tp_recv, t, frame, ctx);

            REQUIRE(tp_recv.state() == transport_protocol::RESPONDER_RECEIVING_DT);

            REQUIRE(tp_recv.established().remaining_bytes() == sz - 14);

            auto data = (char*)tp_recv.payload().data();

            REQUIRE(memcmp(data, "hijklmn", 7) == 0);

            REQUIRE(tp_recv.state() == transport_protocol::RESPONDER_RECEIVED_DT);
        }
    }
}
