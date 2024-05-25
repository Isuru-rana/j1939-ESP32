#include <esp_log.h>
#include <nvs_flash.h>
#include <driver/twai.h>

#include <j1939/ca.hpp>     // gets us dispatcher process_incoming
#include <j1939/state-machines/transport_protocol.hpp>

#include <j1939/data_field/disp1.hpp>

#include "main.h"

const char* TAG = "cm_dt::main";

extern void twai_init();

static constexpr uint8_t sa = 0x77;

static const char component_id[] =
    "Make*"
    "Model*"
    "S/N*"
    "Unit Number";

embr::j1939::sm::v0::transport_protocol tp;

template <class Transport>
bool component_identification_ca::process_incoming(Transport&, pdu<pgns::request>& p)
{
    uint32_t pgn = p.payload().pgn();

    switch((pgns)pgn)
    {
        case pgns::component_identification:
            ESP_LOGI(TAG, "component_id initiating");
            tp.initiate_originator(sizeof(component_id), {0, sa},
                p.source_address(), pgn);
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
        unsigned pos = ctp.originator().current_position();

        ESP_LOGD(TAG, "Prepping chunk: pos=%u", pos);

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

    component_identification_ca ca;
    embr::j1939::sm::v0::transport_protocol::states state = tp.state();
    using context = embr::j1939::sm::v0::transport_protocol::context;
    transport_type t;

    for(;;)
    {
        uint32_t alerts = 0;
        context ctx{0, sa};

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
            }
        }
        else
            // 50ms kind of a magic number for CM DT modes
            vTaskDelay(50 / portTICK_PERIOD_MS);

        ca.process_outgoing(t);
        tp.process_outgoing(t, ctx);

        if(tp.state() != state)
        {
            state = tp.state();
            ESP_LOGI(TAG, "tp state=%d", state);
        }
    }
}