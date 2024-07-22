#include <catch2/catch.hpp>

#include <chrono>

#include <j1939/state-machines/transport_protocol.hpp>

#include <j1939/internal/dispatcher/incoming2.hpp>

#include <can/loopback.h>

#include "test-data.h"

using namespace embr::j1939;
using namespace embr::j1939::sm::v0;

struct fake_clock { };

using duration = estd::chrono::milliseconds;
using time_point = estd::chrono::time_point<fake_clock, duration>;
using ms_type = estd::chrono::milliseconds;

// Mainly useful for testing, not so much production though bears some resemblance
// to aggregated CA handler
template <class TimePoint>
struct helper
{
    const uint8_t orig_sa = 1, recv_sa = 2;
    using time_point = TimePoint;
    transport_protocol<time_point> tp_orig, tp_recv;
    using states = sm::tp::v0::base::states;
    using result = cs::v1::base::result;

    // Theory being CA/state machine should not get confused by its own traffic,
    // plus we auto aggregate to both for convenience

    using ctx = typename transport_protocol<time_point>::context;

    template <class Transport>
    unsigned incoming(Transport& t, const typename Transport::frame& f, unsigned current_ms = {})
    {
        time_point c{ms_type{current_ms}};
        unsigned processed = 0;
        result r = result::ignore();

        r = v2::process_incoming(tp_orig, t, f, ctx{c, orig_sa});
        processed += r.processed;

        r = v2::process_incoming(tp_recv, t, f, ctx{c, recv_sa});
        processed += r.processed;

        return processed;
    }

    // DEBT: prefer to pass in a time_point
    template <class Transport>
    unsigned outgoing(Transport& t, unsigned current_ms = 0)
    {
        time_point c{ms_type(current_ms)};

        unsigned processed = 0;
        result r = result::more();

        while(r.immediate)
        {
            r = tp_orig.process_outgoing(t, ctx{c, orig_sa});
            processed += r.processed;
        }

        r = result::more();

        while(r.immediate)
        {
            r = tp_recv.process_outgoing(t, ctx{c, orig_sa});
            processed += r.processed;
        }

        return processed;
    }

    // Performs outgoing phase first, then expects one message present at transport,
    // then performs incoming phase
    // NOTE: Will need a diff version of this with frame* at some point
    template <class Transport>
    void cycle(Transport& t, unsigned current_ms = {})
    {
        time_point c{ms_type(current_ms)};
        typename Transport::frame f;

        CAPTURE(
            c, to_string(tp_recv.state()), tp_recv.state(),
            to_string(tp_orig.state()));

        REQUIRE(outgoing(t, current_ms) >= 1);
        REQUIRE(t.receive(&f));

        CAPTURE(tp_recv.state(), tp_orig.state());

        REQUIRE(incoming(t, f, current_ms) == 1);
    }

    void verify_incoming_payload(const uint8_t* expected, unsigned expected_sz)
    {
        REQUIRE(tp_recv.state() == states::RESPONDER_RECEIVING_DT);

        auto span = tp_recv.payload();
        auto data = (char*)span.data();
        // FIX: It appears fixed-size string pointer doesn't work
        //estd::layer2::basic_string<char, 7, false> s{data};
        //REQUIRE(s == "abcdefg");
        REQUIRE(span.size() == expected_sz);
        REQUIRE(memcmp(data, expected, expected_sz) == 0);

        REQUIRE(tp_recv.state() == states::RESPONDER_RECEIVED_DT);
    }
};

// Feeds outgoing (originator) state machine
class feeder
{
    const uint8_t* data_;
    transport_protocol<time_point>& tp_;

public:
    feeder(transport_protocol<time_point>& tp, const uint8_t* data) :
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
    transport_protocol<time_point>& tp_;

public:
    recv_feeder(transport_protocol<time_point>& tp, uint8_t* data) :
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
    helper<time_point> h;
    feeder feed(h.tp_orig, (uint8_t*)test::test_str2);
    constexpr unsigned sz = sizeof(test::test_str2) - 1;    // Zapping null terminator
    using states = sm::tp::v0::base::states;
    using tp_type = transport_protocol<time_point>;
    using ctx = tp_type::context;

    SECTION("core")
    {
        tp_type& tp_orig = h.tp_orig;
        tp_type& tp_recv = h.tp_recv;

        {
            tp_orig.initiate_originator(sz, {ms_type{0}, addresses::null}, h.recv_sa,
                (uint32_t)pgns::software_identification);

            h.cycle(t, 0);      // Send RTS, receive RTS

            REQUIRE(tp_orig.originator().resequence_requested() == false);

            h.cycle(t, 50);     // Send CTS, receive CTS

            REQUIRE(tp_orig.state() == states::ORIGINATOR_RECEIVED_CTS);
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

        REQUIRE(h.tp_recv.state() == states::RESPONDER_SENT_EOM_ACK);
        REQUIRE(h.tp_orig.state() == states::ORIGINATOR_RECEIVED_EOM_ACK);
    }
    SECTION("retry (cts early)")
    {
        embr::can::loopback_transport black_hole;
        h.tp_orig.initiate_originator(h.recv_sa,
            (uint32_t)pgns::software_identification,
            sz);

        h.cycle(t, 0);      // Send RTS, receive RTS
        h.cycle(t, 50);      // Send CTS, receive CTS

        // NOTE: Above 50 demarcates beginning of CTS T2 timeout, only computed exactly right
        // with 'next_event' flavor

        REQUIRE(t.peek() == nullptr);

        time_point current(ms_type(50));

        h.tp_orig.payload((uint8_t*)test::test_str2);                   // mark payload as ready to send
        h.tp_orig.process_outgoing(black_hole, { current, h.orig_sa });     // lose the DT
        h.tp_recv.process_outgoing(t, { current, h.recv_sa });     // ensure state machine just sits there

        REQUIRE(t.peek() == nullptr);

        // Wait long enough for CTS retry to kick in
        current += tp_type::timeouts::T2;

#if FEATURE_EMBR_J1939_TP_FUTURE
        // DEBT: Put together to_string overloads
        REQUIRE(current.time_since_epoch().count() ==
            h.tp_recv.next_event().time_since_epoch().count());
#endif

        // DEBT: Minor debt only, two consecutive process_outgoing are needed since one
        // detects the timeout and the next actually emits the resend
        h.tp_recv.process_outgoing(t, { current, h.recv_sa });
        h.tp_recv.process_outgoing(t, { current, h.recv_sa });

        REQUIRE(h.tp_recv.responder().retransmit_counter_ == 1);

        REQUIRE(t.receive(&frame));

        current += ms_type{50};

        embr::j1939::v2::
            process_incoming(
                h.tp_orig,
                t, frame,
                ctx{current, h.orig_sa});

        REQUIRE(h.tp_orig.originator().resequence_requested());
    }
    SECTION("broadcast (bam)")
    {
        SECTION("normal")
        {
            h.tp_orig.initiate_originator(
                0xFF,
                (uint32_t)pgns::software_identification,
                sz);

            h.cycle(t, 0);      // Send BAM, receive BAM

            h.tp_orig.payload((uint8_t*)test::test_str2);
            h.tp_orig.process_outgoing(t, {ms_type{25}, h.orig_sa}); // Too early

            REQUIRE(t.peek() == nullptr);

            h.tp_orig.process_outgoing(t, {ms_type{50}, h.orig_sa});

            REQUIRE(t.receive(&frame));

            v2::process_incoming(h.tp_recv, t, frame, ctx{ms_type{51}, h.recv_sa});
        }
        SECTION("auto-payload")
        {
            h.tp_orig.initiate_originator(
                0xFF,
                (uint32_t)pgns::software_identification,
                test::test_str2,
                sz);

            h.cycle(t, 0);      // Send BAM, receive BAM

            h.tp_orig.process_outgoing(t, {ms_type{25}, h.orig_sa}); // Too early

            REQUIRE(t.peek() == nullptr);

            h.tp_orig.process_outgoing(t, {ms_type{50}, h.orig_sa});

            REQUIRE(h.tp_recv.state() == states::RESPONDER_RECEIVED_BAM);
            REQUIRE(h.tp_orig.state() == states::ORIGINATOR_SENT_DT);

            REQUIRE(t.receive(&frame));

            embr::j1939::v2::process_incoming(h.tp_recv, t, frame, ctx{ms_type{51}, h.recv_sa});

            // Remember, no auto-payload on receive, just on send
            estd::span<const uint8_t> payload(h.tp_recv.payload());

            REQUIRE(payload[0] == '0');
        }
    }
    SECTION("experimental feeder test")
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
    SECTION("auto send payoad")
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

        REQUIRE(memcmp(s, test::test_str2, 14) == 0);
    }
    SECTION("timeouts/aborts")
    {
        h.tp_orig.initiate_originator(h.recv_sa,
            (uint32_t)pgns::software_identification,
            test::test_str2,
            sz);

        h.outgoing(t, 0);   // Send RTS

        REQUIRE(t.receive(&frame));

        SECTION("On initial handshake")
        {
            h.incoming(t, frame, 0);    // Receive RTS

            // tp_orig process outgoing (too late) results in an abort
            // tp_recv Send CTS (too late)
            // DEBT: tp_recv is able to detect it's too late, but does nothing
            // about it - maybe it should emit an abort message too
            h.outgoing(t, tp_type::timeouts::T2.count() + 1);

            // Here we have two messages now, a CTS and abort
            REQUIRE(t.queue.size() == 2);
            REQUIRE(t.receive(&frame));

            h.incoming(t, frame, tp_type::timeouts::T2.count() + 2);
        }
    }
}
