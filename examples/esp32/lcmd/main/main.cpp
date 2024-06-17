#include <estd/chrono.h>

#include <j1939/cas/internal/prng_address_manager.h>

#include <j1939/state-machines/lcmd.hpp>
#include <j1939/state-machines/network/network.hpp>
#include <j1939/NAME/name.h>

#include <j1939/internal/dispatcher/incoming.hpp>

#include "lcmd_sink.hpp"
#include "transport.h"

using clock_type = estd::chrono::freertos_clock;
using time_point = clock_type::time_point;

using namespace embr;

using NAME = j1939::layer0::NAME<
    true,
    j1939::industry_groups::on_highway,
    j1939::vehicle_systems::trailer,
    j1939::function_fields::brakes_system_controller,
    0, j1939::manufacturer_codes::not_applicable>;

using address_manager = j1939::internal::prng_address_manager;

static j1939::sm::v0::lighting_command<time_point> lcmd_source;
static app::lcmd_sink lcmd_sink;
static j1939::sm::v1::network<
    address_manager,
    time_point> nca(address_manager{}, NAME::sparse(j1939::null_t{}));

// NOTE: Not ready yet

extern "C" void app_main(void)
{
    transport_type primary;
    loopback_type loopback;

    using context = decltype(lcmd_source)::context;

    transport_type::frame frame;

    for(;;)
    {
        const time_point now = clock_type::now();

        while(loopback.receive(&frame))
        {
            // sink receives directly from loopback, which only carries lcmd generates from lcmd_source
            j1939::process_incoming(lcmd_sink, loopback, frame);
            primary.send(frame);
        }

        while(primary.receive(&frame))
        {
            // DEBT: Probably prefer a direct cascade psuedo-transport which has an aggregated list of
            // cs/ca/sm's attached.  That would obviate the need for a full loopback w/ local queue
            j1939::process_incoming(lcmd_source, loopback, frame, context{now});
            // sink receives from physical bus
            j1939::process_incoming(lcmd_sink, loopback, frame);
            j1939::process_incoming(nca, primary, frame, context{now});
        }

        nca.process_outgoing(primary, context{now});
        lcmd_source.process_outgoing(primary, context{now});
    }
}
