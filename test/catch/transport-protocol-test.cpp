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

        processed += process_incoming(tp_orig, t, f, ctx{current, orig_sa});
        processed += process_incoming(tp_recv, t, f, ctx{current, recv_sa});

        return processed;
    }

    template <class Transport>
    unsigned outgoing(Transport& t, time_point current = {})
    {
        unsigned processed = 0;

        processed += tp_orig.process_outgoing(t, ctx{current, orig_sa});
        processed += tp_recv.process_outgoing(t, ctx{current, recv_sa});

        return processed;
    }

    // Performs outgoing phase first, then expects one message present at transport,
    // then performs incoming phase
    // NOTE: Will need a diff version of this with frame* at some point
    template <class Transport>
    void cycle(Transport& t, time_point current = {})
    {
        typename Transport::frame f;

        CAPTURE(
            current, to_string(tp_recv.state()), tp_recv.state(),
            to_string(tp_orig.state()));

        REQUIRE(outgoing(t, current) >= 1);
        REQUIRE(t.receive(&f));

        CAPTURE(tp_recv.state(), tp_orig.state());

        REQUIRE(incoming(t, f, current) == 1);
    }

    void verify_incoming_payload(const uint8_t* expected, unsigned expected_sz)
    {
        REQUIRE(tp_recv.state() == transport_protocol::RESPONDER_RECEIVING_DT);

        auto span = tp_recv.payload();
        auto data = (char*)span.data();
        // FIX: It appears fixed-size string pointer doesn't work
        //estd::layer2::basic_string<char, 7, false> s{data};
        //REQUIRE(s == "abcdefg");
        REQUIRE(span.size() == expected_sz);
        REQUIRE(memcmp(data, expected, expected_sz) == 0);

        REQUIRE(tp_recv.state() == transport_protocol::RESPONDER_RECEIVED_DT);
    }
};

// Feeds outgoing (originator) state machine
class feeder
{
    const uint8_t* data_;
    transport_protocol& tp_;

public:
    feeder(transport_protocol& tp, const uint8_t* data) :
        data_{data},
        tp_{tp}
    {}

    bool process()
    {
        if(tp_.ready_for_payload())
        {
            const uint16_t pos = tp_.originator().last_position();
            tp_.payload(data_ + pos);
            return true;
        }

        return false;
    }
};


// Feeds incoming (responder) data from state machine
class recv_feeder
{
    uint8_t* data_;
    transport_protocol& tp_;

public:
    recv_feeder(transport_protocol& tp, uint8_t* data) :
        data_{data},
        tp_{tp}
    {}

    bool process()
    {
        if(tp_.payload_present())
        {
            const uint16_t pos = tp_.responder().receiving_bytes();
            auto data = tp_.payload();

            std::copy(data.begin(), data.end(), data_ + pos);
            return true;
        }

        return false;
    }
};


TEST_CASE("transport protocol (J1939-21 Section 5.10)")
{
    embr::can::loopback_transport t;
    embr::can::loopback_transport::frame frame;
    helper h;
    feeder feed(h.tp_orig, (uint8_t*)test::test_str2);
    constexpr unsigned sz = sizeof(test::test_str2) - 1;    // Zapping null terminator
    using ctx = transport_protocol::context;

    SECTION("core")
    {
        transport_protocol& tp_orig = h.tp_orig;
        transport_protocol& tp_recv = h.tp_recv;

        {
            tp_orig.initiate_originator(sz, {0, uint8_t(addresses::null_address)}, h.recv_sa,
                (uint32_t)pgns::software_identification);

            h.cycle(t, 0);      // Send RTS, receive RTS

            REQUIRE(tp_orig.originator().resequence_requested() == false);

            h.cycle(t, 50);     // Send CTS, receive CTS

            REQUIRE(tp_orig.state() == transport_protocol::ORIGINATOR_RECEIVED_CTS);
            REQUIRE(tp_orig.originator().last_sequence() == 0);
        }

        REQUIRE(t.peek() == nullptr);

        {
            tp_orig.payload((uint8_t*)test::test_str2);

            h.cycle(t, 100);    // Send DT, receive DT

            REQUIRE(tp_orig.originator().last_sequence() == 1);
            REQUIRE(tp_recv.responder().seq() == 1);
            REQUIRE(tp_recv.responder().received_bytes() == 7);

            h.verify_incoming_payload((const uint8_t *)"0123456", 7);
        }

        {
            tp_orig.payload((uint8_t*)test::test_str2 + 7);

            h.cycle(t, 150);    // Send DT, receive DT

            REQUIRE(tp_orig.originator().last_sequence() == 2);
            REQUIRE(tp_recv.responder().seq() == 2);
            REQUIRE(tp_recv.responder().received_bytes() == 14);

            h.verify_incoming_payload((const uint8_t *)"789ABCD", 7);
        }

        {
            REQUIRE(feed.process());
            //tp_orig.payload((uint8_t*)test::test_str2 + 14);

            h.cycle(t, 200);    // Send DT, receive DT

            REQUIRE(tp_orig.originator().last_sequence() == 3);
            REQUIRE(tp_recv.responder().seq() == 3);
            // NOTE: State machine doesn't need to precisely know it's 16 bytes, so
            // it always is on 7 byte boundaries
            REQUIRE(tp_recv.responder().received_bytes() == 21);

            h.verify_incoming_payload((const uint8_t *)"EF", 2);
        }

        // Reached end/ack area

        h.cycle(t, 250);    // Send ACK, receive ACK

        REQUIRE(h.tp_recv.state() == transport_protocol::RESPONDER_SENT_EOM_ACK);
        REQUIRE(h.tp_orig.state() == transport_protocol::ORIGINATOR_RECEIVED_EOM_ACK);
    }
    SECTION("retry (cts early)")
    {
        embr::can::loopback_transport black_hole;
        h.tp_orig.initiate_originator(h.recv_sa,
            (uint32_t)pgns::software_identification,
            sz);

        h.cycle(t, 0);      // Send RTS, receive RTS
        h.cycle(t, 50);      // Send CTS, receive CTS

        REQUIRE(t.peek() == nullptr);

        h.tp_orig.payload((uint8_t*)test::test_str2);                   // mark payload as ready to send
        h.tp_orig.process_outgoing(black_hole, { 100, h.orig_sa });     // lose the DT
        h.tp_recv.process_outgoing(t, { 100, h.recv_sa });

        REQUIRE(t.peek() == nullptr);

        // DEBT: Minor debt only, two consecutive process_outgoing are needed since one
        // detects the timeout and the next actually emits the resend
        h.tp_recv.process_outgoing(t, { transport_protocol::timeouts::T2, h.recv_sa });
        h.tp_recv.process_outgoing(t, { transport_protocol::timeouts::T2, h.recv_sa });

        REQUIRE(h.tp_recv.responder().retransmit_counter_ == 1);

        REQUIRE(t.receive(&frame));

        process_incoming(h.tp_orig, t, frame, ctx{transport_protocol::timeouts::T2 + 50, h.orig_sa});

        REQUIRE(h.tp_orig.originator().resequence_requested());
    }
    SECTION("broadcast (bam)")
    {
        h.tp_orig.initiate_originator(0xFF,
            (uint32_t)pgns::software_identification,
            sz);

        h.cycle(t, 0);      // Send BAM, receive BAM

        h.tp_orig.payload((uint8_t*)test::test_str2);
        h.tp_orig.process_outgoing(t, {25, h.orig_sa}); // Too early

        REQUIRE(t.peek() == nullptr);

        h.tp_orig.process_outgoing(t, {50, h.orig_sa});

        REQUIRE(t.receive(&frame));

        process_incoming(h.tp_recv, t, frame, ctx{51, h.recv_sa});
    }
    SECTION("unfinished test")
    {
        char s[32] {};
        recv_feeder recv_feed(h.tp_recv, (uint8_t*)s);

        h.tp_orig.initiate_originator(h.recv_sa,
            (uint32_t)pgns::software_identification,
            sz);

        h.cycle(t, 0);      // Send RTS, receive RTS
        h.cycle(t, 0);      // Send CTS, receive CTS

        REQUIRE(feed.process());

        h.cycle(t, 100);    // Send DT, receive DT

        REQUIRE(recv_feed.process());

        REQUIRE(feed.process());

        h.cycle(t, 150);    // Send DT, receive DT

        REQUIRE(recv_feed.process());

        REQUIRE(memcmp(s, test::test_str2, 14) == 0);
    }
    SECTION("auto payoad")
    {
        char s[32] {};
        recv_feeder recv_feed(h.tp_recv, (uint8_t*)s);

        h.tp_orig.initiate_originator(h.recv_sa,
            (uint32_t)pgns::software_identification,
            test::test_str2,
            sz);

        h.outgoing(t, 0);   // Send RTS

        REQUIRE(t.receive(&frame));

        // Receive RTS... ?
        int r = h.incoming(t, frame, 0) == 1;
        REQUIRE(r == 1);

        h.cycle(t, 0);      // Send CTS, receive CTS

        r = h.outgoing(t, 0);   // Stoke auto-send logic

        REQUIRE(r == 1);

        h.cycle(t, 100);    // Send DT, receive DT

        REQUIRE(recv_feed.process());

        r = h.outgoing(t, 150);   // Stoke auto-send logic

        h.cycle(t, 150);    // Send DT, receive DT

        REQUIRE(recv_feed.process());

        // Not quite...
        REQUIRE(memcmp(s, test::test_str2, 14) == 0);
    }
}
