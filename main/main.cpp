#include <esp_log.h>
#include <nvs_flash.h>
#include <driver/twai.h>

#include <estd/thread.h>

#include <j1939/ca.hpp>     // gets us dispatcher process_incoming
#include <j1939/state-machines/transport_protocol.hpp>

#include <j1939/data_field/disp1.hpp>

#include "main.h"

const char* TAG = "cm_dt::main";

extern void twai_init();

static constexpr uint8_t sa = 0x77;

using namespace estd::chrono_literals;

static const char component_id[] =
    "Make*"
    "Model*"
    "S/N*"
    "Unit Number";

// DEBT: Not fully vetted if this is 100% proper way to emit software ID.  Close, though
static const char software_id[] =
    "\1ESP32 cm_dt firmware v0.0.0*";

embr::j1939::sm::v0::transport_protocol tp, tp2;

template <class Transport>
bool component_identification_ca::process_incoming(Transport&, pdu<pgns::request>& p)
{
    uint32_t pgn = p.payload().pgn();

    switch((pgns)pgn)
    {
        case pgns::component_identification:
            ESP_LOGI(TAG, "component_id initiating");
            tp.initiate_originator(sizeof(component_id), p.source_address(), pgn);
            return true;

        default: break;
    }

    return {};
}


template <class Transport>
bool component_identification_ca::process_outgoing(Transport&)
{
    if(tp.ready_for_payload())
    {
        const auto& ctp = tp;
        unsigned pos = ctp.originator().last_position();

        ESP_LOGD(TAG, "Prepping chunk (ECUID): pos=%u", pos);

        tp.payload((uint8_t*)component_id + pos);
    }

    return {};
}


extern "C" void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    twai_init();

    uint32_t prev_alerts = 0;
    using clock = estd::chrono::freertos_clock;
    using time_point = clock::time_point;

    time_point last_bam;

    component_identification_ca ca;
    embr::j1939::sm::v0::transport_protocol::states state = tp.state(),
        state2 = tp2.state();
    using context = embr::j1939::sm::v0::transport_protocol::context;
    transport_type t;

    for(;;)
    {
        const time_point now = clock::now();
        uint32_t alerts = 0;
        auto now_ms = estd::chrono::milliseconds(now.time_since_epoch()).count();
        context ctx{uint32_t(now_ms), sa};

        twai_read_alerts(&alerts, 0);

        if(alerts != prev_alerts)
        {
            // NOTE: There's a built in ESP-IDF logging facility for
            // TWAI alerts IIRC
            ESP_LOGD(TAG, "alerts: %" PRIx32, alerts);
            prev_alerts = alerts;
        }

        if(alerts & TWAI_ALERT_RX_DATA)
        {
            transport_type::frame frame;

            while(transport_type::receive(&frame))
            {
                embr::j1939::process_incoming(ca, t, frame);
                embr::j1939::process_incoming(tp, t, frame, ctx);
                embr::j1939::process_incoming(tp2, t, frame, ctx);
            }
        }
        else
            // 50ms kind of a magic number for CM DT modes
            //vTaskDelay(50 / portTICK_PERIOD_MS);
            estd::this_thread::sleep_for(50ms);

        ca.process_outgoing(t);
        tp.process_outgoing(t, ctx);
        tp2.process_outgoing(t, ctx);
        tp2.process_outgoing(t, ctx);       // DEBT: Needing to double these up, in this case so it can transition to SENT_ALL

        if(tp.state() != state)
        {
            state = tp.state();
            ESP_LOGI(TAG, "tp state=%s (%d)", embr::j1939::to_string(state), state);
        }

        if(tp2.state() != state2)
        {
            state2 = tp2.state();
            ESP_LOGI(TAG, "tp2 state=%s (%d)", embr::j1939::to_string(state2), state2);
        }

        if(tp2.ready_for_payload())
        {
            const auto& ctp = tp2;
            unsigned pos = ctp.originator().last_position();

            ESP_LOGD(TAG, "Prepping chunk (SOFT): pos=%u", pos);

            tp2.payload((uint8_t*)software_id + pos);
        }

        if(now - last_bam > 10s)
        {
            ESP_LOGI(TAG, "software_id (bam) initiating");
            tp2.initiate_originator(sizeof(software_id) - 1, 0xFF,
                (uint32_t)embr::j1939::pgns::software_identification);
            last_bam = now;
        }
    }
}