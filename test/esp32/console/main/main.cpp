#include <estd/thread.h>

#include <embr/scheduler.hpp>

#include <j1939/ca.hpp>
#include <j1939/state-machines/transport_protocol.hpp>

#include "nca.h"
#include "transport.h"

using namespace estd::chrono_literals;

using namespace embr::j1939;

void init_console();
void twai_init();

scheduler_type scheduler;

using proto_name = embr::j1939::layer0::NAME<true,
    industry_groups::process_control,
    vehicle_systems::ig5_not_available, // DEBT: Change to a better IG/Veh Sys,
    function_fields::ig5_not_available>;

sm::transport_protocol tp;
nca_type nca(proto_name::sparse{3, 2, 1}, scheduler);

transport_type t;

extern "C" void app_main(void)
{
    twai_init();
    init_console();

    for(;;)
    {
        transport_type::frame frame;
        sm::transport_protocol::context ctx{0, 0};
        
        while(t.receive(&frame))
        {
            process_incoming(tp, t, frame, ctx);
        }

        tp.process_outgoing(t, ctx);

        estd::this_thread::sleep_for(50ms);
        scheduler.process();
    }
}
