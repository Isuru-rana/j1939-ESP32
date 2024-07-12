#include <estd/chrono.h>
#include <estd/sstream.h>
#include <estd/thread.h>

#include <j1939/cas/internal/prng_address_manager.h>
#include <j1939/cas/diagnostic.hpp>

#include <j1939/state-machines/lcmd.hpp>
#include <j1939/state-machines/network/network.hpp>
#include <j1939/NAME/name.h>

#include <j1939/dispatcher.hpp>

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

using ostream_type = estd::layer1::ostringstream<256>;
using address_manager = j1939::internal::prng_address_manager;

static j1939::sm::v0::lighting_command<time_point> lcmd_source;
static app::lcmd_sink lcmd_sink;
static j1939::sm::v1::network<
    address_manager,
    time_point> nca(address_manager{}, NAME::sparse(j1939::null_t{}));
// DEBT: A true-blue serial/usb ostream is in the works.  See esp_idf::log_ostream
// In the meantime, emit to and log local string will serve us.  
static ostream_type diag_out;
static j1939::v1::diagnostic_ca<transport_type, ostream_type> dca(diag_out);


#define GPIO_OUTPUT_PIN_SEL  \
(1ULL<<CONFIG_GPIO_BRAKE |  \
 1ULL<<CONFIG_GPIO_LEFT_BLINKER | \
 1ULL<<CONFIG_GPIO_RIGHT_BLINKER)

static const char* TAG = "lcmd::main";


void gpio_init()
{
    ESP_LOGD(TAG, "gpio_init: entry");

    gpio_config_t io_conf = {};

    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;

    ESP_ERROR_CHECK(gpio_config(&io_conf));
}

extern "C" void app_main(void)
{
    transport_type primary;
    loopback_type loopback;

    primary.init();

    gpio_init();

    using context = decltype(lcmd_source)::context;

    transport_type::frame frame;

    for(;;)
    {
        const time_point now = clock_type::now();

        while(loopback.receive(&frame))
        {
#if CONFIG_LCMD_SINK
            // sink receives directly from loopback, which only carries lcmd generated from lcmd_source
            j1939::process_incoming(lcmd_sink, loopback, frame);
#endif
            // retransmit loopback->physical bus
            primary.send(frame);
        }

        while(primary.receive(&frame))
        {
            // DEBT: Probably prefer a direct cascade psuedo-transport which has an aggregated list of
            // cs/ca/sm's attached.  That would obviate the need for a full loopback w/ local queue
#if CONFIG_LCMD_SOURCE
            j1939::process_incoming(lcmd_source, loopback, frame, context{now});
#endif
            // sink receives here from physical bus (in addition to loopback above)
#if CONFIG_LCMD_SINK
            j1939::process_incoming(lcmd_sink, loopback, frame);
#endif
            j1939::process_incoming(nca, primary, frame, context{now});

            diag_out.rdbuf()->clear();

            j1939::process_incoming(dca, primary, frame);

            // diagnostic_ca emits eol into 'out'
            esp_log_write(ESP_LOG_INFO, TAG, diag_out.rdbuf()->str().data());
        }

        nca.process_outgoing(primary, context{now});
        // Self-contained lcmd source + sink requires we "send to ourself"
        lcmd_source.process_outgoing(loopback, context{now});

        // Just to keep WDT off our backs and CPU from spinning like crazy
        estd::this_thread::sleep_for(estd::chrono::milliseconds(50));
    }
}
