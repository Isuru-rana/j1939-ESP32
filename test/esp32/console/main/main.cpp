#include <estd/thread.h>

#include <embr/scheduler.hpp>

#include <j1939/ca.hpp>
#include <j1939/state-machines/transport_protocol.hpp>
#include <j1939/cas/diagnostic.hpp>

#include "nca.h"
#include "streambuf.h"
#include "transport.h"
#include "tp.h"

using namespace estd::chrono_literals;

using namespace embr::j1939;

void init_console();
void twai_init();

scheduler_type scheduler;

using proto_name = embr::j1939::layer0::NAME<true,
    industry_groups::process_control,
    vehicle_systems::ig5_not_available, // DEBT: Change to a better IG/Veh Sys,
    function_fields::ig5_not_available>;

extern esp_idf::log_ostream clog;
extern bool dca_enabled;

using dca_type = diagnostic_ca<transport_type, esp_idf::log_ostream>;

tp_type tp;
nca_type nca(proto_name::sparse{3, 2, 1}, scheduler);
static sm::v0::network_cached network_cached;

transport_type t;

dca_type dca(clog);

extern "C" void app_main(void)
{
    twai_init();

    // Enable logging in addition to default ALL
    // 16JUL24 NOTE: We don't get full send error status for some
    // reason w/o logging (ESP32C6)
    ESP_ERROR_CHECK(twai_reconfigure_alerts(
        TWAI_ALERT_ALL | TWAI_ALERT_AND_LOG,
        nullptr));

    init_console();

    for(;;)
    {
        transport_type::frame frame;
        tp_type::context ctx{time_point::clock::now(), 0};
        
        while(t.receive(&frame))
        {
            process_incoming(tp, t, frame, ctx);
            
            if(dca_enabled)
                process_incoming(dca, t, frame);
        }

        tp.process_outgoing(t, ctx);

        if(network_cached.state(nca))
        {
            clog << "nca:" << to_string(nca.state()) << ':';
            clog << to_string(nca.substate()) << estd::endl;
        }

        estd::this_thread::sleep_for(50ms);
        scheduler.process();
    }
}
