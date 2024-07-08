#include <Arduino.h>

#define FEATURE_EMBR_J1939_TP_RESPONDER 0   // ~1.5k ROM

#include <estd/string.h>

#include <j1939/pdu.h>
#include <j1939/pgn.h>

#include <j1939/cas/diagnostic.hpp>
#include <j1939/cas/network.hpp>
#include <j1939/cas/internal/prng_address_manager.h>
#include <j1939/ca.hpp>

#include <j1939/internal/dispatcher/incoming2.hpp>

#include <j1939/data_field/all.hpp>
#include <j1939/state-machines/transport_protocol.hpp>

#include <j1939/ostream.h>

#include "component_id.h"
#include "transport.h"
#include "scheduler.h"

using namespace estd;
using namespace embr::j1939;
using namespace embr::units::literals;

arduino_ostream cout(Serial);

#ifdef AUTOWP_LIB
static transport t(10);     // CS pin
#else
static transport t;
#endif

scheduler_type scheduler;

#define CONFIG_NCA_SCHEDULED 1  // flag not used yet, always on

// 08JUL24
#define FEATURE_DIAGNOSTIC  1       // ~8.5k ROM
#define FEATURE_NETWORK     1       // ~4k ROM
#define FEATURE_TP          1       // ~3.5k ROM w/ responder disabled

using dca_type = diagnostic_ca<transport, arduino_ostream>;

using proto_name = embr::j1939::layer0::NAME<true,
    industry_groups::process_control,
    vehicle_systems::ig5_not_available, // DEBT: Change to a better IG/Veh Sys,
    function_fields::ig5_not_available>;

// TODO: Change this to state machine only flavor, scheduler-based NCA
// falling out of favor
using nca_type = embr::j1939::impl::network_ca<transport,
    scheduler_type,
    embr::j1939::internal::prng_address_manager>;

#define FEATURE_AGGREGATED_CA 0
#if FEATURE_AGGREGATED_CA
embr::j1939::impl::controller_application_aggregator<dca_type, nca_type>
    app_ca(
        cout,
        nca_type::get_init(
            proto_name::sparse{0, 0, ecu_instance.value()},
            scheduler));
#endif

#if FEATURE_NETWORK
nca_type nca(proto_name::sparse{1, 0, 3}, scheduler);
#endif

#if FEATURE_DIAGNOSTIC
dca_type dca(cout);
#endif

#if FEATURE_TP
sm::transport_protocol<time_point> tp;

component_identification_ca cidca;
#endif



// DEBT: Not fully vetted if this is 100% proper way to emit software ID.  Close, though
static const char software_id[] =
    "\1Arduino cm_dt/network test firmware v0.0.0*";

#if FEATURE_TP
template <class Transport>
bool component_identification_ca::process_incoming(
    Transport&, const pdu<pgns::request>& p)
{
    uint32_t pgn = p.payload().pgn();

    switch((pgns)pgn)
    {
        case pgns::component_identification:
            tp.initiate_originator(p.source_address(), pgn,
                software_id, sizeof(software_id) - 1);
            return true;

        default: break;
    }

    return {};
}
#endif

void setup()
{
    Serial.begin(115200);

    while(!Serial);

    init_can(t);
}


void loop()
{
    transport::frame f;
    // DEBT: time_point overall needs more attention
#if FEATURE_TP
    sm::transport_protocol<time_point>::context ctx{
        time_point::clock::now(),
#if FEATURE_NETWORK
        nca.address().value()};
#else
        0x77};
#endif
#endif

    if(t.receive(&f))
    {
#if FEATURE_DIAGNOSTIC
        process_incoming(dca, t, f);
#endif
#if FEATURE_NETWORK
        embr::j1939::v2::process_incoming(nca, t, f);
#endif
#if FEATURE_TP
        embr::j1939::v2::process_incoming(tp, t, f, ctx);
        embr::j1939::v2::process_incoming(cidca, t, f);
#endif
    }

#if FEATURE_TP
    tp.process_outgoing(t, ctx);
#endif
#if FEATURE_NETWORK
    scheduler.process();
#endif
}