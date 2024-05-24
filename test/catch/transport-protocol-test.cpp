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

    using ctx = transport_protocol::context;
    using time_point = transport_protocol::time_point;

    template <class Transport>
    unsigned incoming(Transport& t, const typename Transport::frame& f, time_point current = {})
    {
        unsigned processed = 0;

        processed += process_incoming(tp_orig, t, f, ctx{0, orig_sa});
        processed += process_incoming(tp_recv, t, f, ctx{0, recv_sa});

        return processed;
    }

    template <class Transport>
    unsigned outgoing(Transport& t, time_point current = {})
    {
        unsigned processed = 0;

        processed += tp_orig.process_outgoing(t, ctx{0, orig_sa});
        processed += tp_recv.process_outgoing(t, ctx{0, recv_sa});

        return processed;
    }

    // Performs outgoing phase first, then expects one message present at transport,
    // then performs incoming phase
    // NOTE: Will need a diff version of this with frame* at some point
    template <class Transport>
    void cycle(Transport& t, time_point current = {})
    {
        typename Transport::frame f;

        CAPTURE(current, tp_recv.state(), tp_orig.state());

        REQUIRE(outgoing(t, current) == 1);
        REQUIRE(t.receive(&f));

        CAPTURE(tp_recv.state(), tp_orig.state());

        REQUIRE(incoming(t, f, current) == 1);
    }

    void verify_incoming_payload(const uint8_t* expected, unsigned expected_sz)
    {
        REQUIRE(tp_recv.state() == transport_protocol::RESPONDER_RECEIVING_DT);

        auto data = (char*)tp_recv.payload().data();
        // FIX: It appears fixed-size string pointer doesn't work
        //estd::layer2::basic_string<char, 7, false> s{data};
        //REQUIRE(s == "abcdefg");
        REQUIRE(memcmp(data, expected, expected_sz) == 0);

        REQUIRE(tp_recv.state() == transport_protocol::RESPONDER_RECEIVED_DT);
    }
};

TEST_CASE("transport protocol (J1939-21 Section 5.10)")
{
    embr::can::loopback_transport t;
    //embr::can::loopback_transport::frame frame;

    SECTION("core")
    {
        //using ctx = transport_protocol::context;
        helper h;
        transport_protocol& tp_orig = h.tp_orig;
        transport_protocol& tp_recv = h.tp_recv;

        constexpr unsigned sz = sizeof(test::test_str2) - 1;    // Zapping null terminator

        //INFO("phase 1")

        {
            tp_orig.initiate_originator(sz, {0, uint8_t(addresses::null_address)}, h.recv_sa);

            h.cycle(t, 0);

            REQUIRE(tp_orig.originator().resequence_requested() == false);

            h.cycle(t, 50);

            REQUIRE(tp_orig.state() == transport_protocol::ORIGINATOR_RECEIVED_CTS);
        }

        REQUIRE(t.peek() == nullptr);

        {
            tp_orig.payload((uint8_t*)test::test_str2);

            h.cycle(t, 100);

            REQUIRE(tp_recv.established().remaining_bytes() == sz - 7);

            h.verify_incoming_payload((const uint8_t *)"0123456", 7);
        }

        {
            tp_orig.payload((uint8_t*)test::test_str2 + 7);

            h.cycle(t, 150);

            REQUIRE(tp_recv.established().remaining_bytes() == sz - 14);

            h.verify_incoming_payload((const uint8_t *)"789ABCD", 7);
        }

        {
            tp_orig.payload((uint8_t*)test::test_str2 + 14);

            h.cycle(t, 200);

            REQUIRE(tp_recv.established().remaining_bytes() == 2);

            h.verify_incoming_payload((const uint8_t *)"EF", 2);
        }

        // Reached end/ack area

        h.cycle(t, 250);

        REQUIRE(h.tp_recv.state() == transport_protocol::RESPONDER_SENT_EOM_ACK);
        REQUIRE(h.tp_orig.state() == transport_protocol::ORIGINATOR_RECEIVED_EOM_ACK);
    }
}
