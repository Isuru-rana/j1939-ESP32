#include <estd/sstream.h>

#include <embr/scheduler.hpp>

#include <can/loopback.h>

#include <j1939/ca.hpp>

#include <j1939/cas/lighting_command.hpp>
#include <j1939/cas/network.hpp>
#include <j1939/cas/transport_protocol.hpp>
#include <j1939/cas/diagnostic.hpp>
#include <j1939/cas/oel_source.h>

#include <j1939/cas/internal/prng_address_manager.h>

#include <j1939/internal/dispatcher/incoming2.hpp>

#include "test-data.h"
#include "test-cs.h"

#include "macro/push.h"

using namespace estd::chrono_literals;
using namespace embr;
using namespace embr::j1939;


namespace embr { namespace  j1939 { namespace  impl { namespace  experimental {

template <>
struct ca_time_helper<FunctorImpl, unsigned>
{
    static constexpr unsigned milliseconds(unsigned ms) { return ms; }
};

}}}}


struct BlinkCAImpl
{
    // number of total blinks
    int left_count = 0;
    int right_count = 0;

    void left_signal(bool on)
    {
        ++left_count;
    }

    void right_signal(bool on)
    {
        ++right_count;
    }
};


using ostringstream = estd::detail::basic_ostream<estd::layer1::stringbuf<128>>;


TEST_CASE("Controller Applications")
{
    using namespace j1939;
    using frame = can::loopback_transport::frame;
    using frame_type = frame;
    using frame_traits = j1939::frame_traits<frame>;

    ostringstream out;
    const auto& out_s = out.rdbuf()->str();
    uint32_t can_id;

    can::loopback_transport t;

    const pdu<pgns::fms_identity> fmsi{null_t{}};   // Specifically, not a dispatched flavor

    SECTION("basics")
    {
        controller_application<decltype(t), test::SyntheticCA<decltype(t)> > ca;

        pdu<pgns::oel> oel1(null_t{});

        oel1.payload().turn_signal_switch(enum_type<spns::turn_signal_switch>::left_turn_to_be_flashing);

        frame f = frame_traits::create(oel1);

        ca.process_incoming(t, f);

        REQUIRE(t.receive(&f));

        REQUIRE(ca.switch_bank_control_counter == 0);

        REQUIRE(ca.process_incoming(t, f));

        REQUIRE(ca.switch_bank_control_counter == 1);
    }
    SECTION("transport protocol")
    {
        impl::transport_protocol_ca<decltype(t)> impl_;
        using controls = pdu<pgns::tp_cm>::modes;

        pdu<pgns::tp_cm> r{null_t{}};

        r.payload().control(controls::rts);
        r.payload().pgn((uint32_t)pgns::NAME_management_message);

        process_incoming(impl_, t, frame_traits::create(r));
    }
    SECTION("lighting command (ca)")
    {
        embr::internal::layer1::Scheduler<5, FunctorImpl> scheduler;
        impl::lighting_command_ca<decltype(t), decltype(scheduler)> ca(scheduler);
        can::loopback_transport::frame frame;

        pdu<pgns::oel> p{null_t{}};

        p.turn_signal_switch(enum_type<spns::turn_signal_switch>::left_turn_to_be_flashing);

        ca.process_incoming(t, p);

        REQUIRE(t.queue.size() == 0);   // no lighting commands are generated until scheduler runs

        scheduler.impl().now_ += estd::chrono::milliseconds(300);
        scheduler.process();
        REQUIRE(t.queue.size() == 1);
        REQUIRE(t.receive(&frame));
        REQUIRE(t.queue.size() == 0);
        scheduler.impl().now_ += estd::chrono::milliseconds(300);
        scheduler.process();
        REQUIRE(t.queue.size() == 1);
        REQUIRE(t.receive(&frame));
    }
    SECTION("diagnostic ca")
    {
        SECTION("regular")
        {
            diagnostic_ca<can::loopback_transport, ostringstream> dca(out);

            pdu<pgns::oel> p{null_t{}};

            frame f = frame_traits::create(p);

            process_incoming(dca, t, f);

            //REQUIRE(out_s == "OEL SA:0 ff ff ff ff ff ff ff ff \n");
            REQUIRE(out_s == "OEL SA:0 high beam=no change, turn signal=noop\n");
        }
        SECTION("filtered")
        {
            struct policy : embr::j1939::internal::dispatch_default_policy
            {
                using blacklist = pgn_list<pgns::oel>;
            };

            diagnostic_ca<can::loopback_transport, ostringstream, policy> dca(out);

            out.setf(estd::ios_base::uppercase);

            pdu<pgns::oel> p{null_t{}};

            frame f = frame_traits::create(p);

            j1939::internal::v2::process_incoming(dca, t, f);

            REQUIRE(out_s == "PDU: FDCC SA:0 FF FF FF FF FF FF FF FF\n");
        }
    }
    SECTION("aggregated")
    {
        SECTION("basic")
        {
            impl::controller_application_aggregator<
                test::SyntheticCA<decltype(t)>,
                test::SyntheticCA2<decltype(t)> > ca;

            //REQUIRE(sizeof(ca) == 12);

            auto& child1 = estd::get<0>(ca.child_cas);
            auto& child2 = estd::get<1>(ca.child_cas);

            pdu<pgns::oel> oel1{null_t{}};

            oel1.payload().turn_signal_switch(enum_type<spns::turn_signal_switch>::left_turn_to_be_flashing);

            process_incoming(ca, t, frame_traits::create(oel1));
            process_incoming(ca, t, frame_traits::create(fmsi));

            REQUIRE(child1.oel_counter == 1);
            REQUIRE(child2.unhandled_counter == 1);
        }
    }
    SECTION("experimental")
    {
        embr::internal::layer1::Scheduler<5, FunctorImpl> scheduler;
        using address_manager = j1939::internal::prng_address_manager;
        using nca_type = j1939::impl::network_ca<decltype(t), decltype(scheduler), address_manager>;
        using proto_name = j1939::layer0::NAME<true,
            industry_groups::process_control,
            vehicle_systems::ig5_not_available, // DEBT: Change to a better IG/Veh Sys,
            function_fields::ig5_not_available>;
        using nca_init_type = nca_type::init1<proto_name::sparse>;
        nca_init_type nca_init(proto_name::sparse(0, 0, 0), scheduler);
        auto nca_init2 = nca_type::get_init(proto_name::sparse(0, 0, 0), scheduler);
        embr::j1939::layer1::NAME name{embr::j1939::null_t{}};

        nca_type nca(nca_init2);

        estd::get<0>(nca_init).populate(name);

        REQUIRE(nca.name() == name);

        // TODO: Won't work yet because tuple doesn't yet have converting constructor
        //impl::controller_application_aggregator<nca_type> app_ca(nca_init2);
    }
    SECTION("sources")
    {
        // CA sources here are platform independent, which means they are largely a helper wrapper
        // around CAN Transport itself since the IO of the platform does most of the work

        SECTION("oel")
        {
            embr::j1939::ca::source::oel<can::loopback_transport> ca;
        }
    }
    SECTION("transport protocol")
    {
        using streambuf_type = out_tp_dt_streambuf<can::loopback_transport>;
        streambuf_type sb(t, 0xFF, 123);
        estd::detail::basic_ostream<streambuf_type&> tp_out(sb);

        REQUIRE(sb.pubsync() == -1);

        sb.sputn("hi2u", 4);
        tp_out << "567";

        REQUIRE(t.queue.empty());

        REQUIRE(sb.pubsync() == 0);

        REQUIRE(t.queue.size() == 1);

        tp_out << "12345677654321!";

        REQUIRE(t.queue.size() == 3);
    }
}

#include "macro/pop.h"

