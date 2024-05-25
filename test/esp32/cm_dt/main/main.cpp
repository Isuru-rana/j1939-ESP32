#include <esp_log.h>
#include <nvs_flash.h>
#include <driver/twai.h>

#include <j1939/ca.hpp>     // gets us dispatcher process_incoming
#include <j1939/state-machines/transport_protocol.hpp>

#include "main.h"

const char* TAG = "cm_dt::main";

extern void twai_init();

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

    embr::j1939::sm::v0::transport_protocol tp;
    using context = embr::j1939::sm::v0::transport_protocol::context;
    transport_type t;

    for(;;)
    {
        uint32_t alerts = 0;
        context ctx{0, 0x77};

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
                embr::j1939::process_incoming(tp, t, frame, ctx);
            }
        }
        else
            // 50ms kind of a magic number for CM DT modes
            vTaskDelay(50 / portTICK_PERIOD_MS);

        tp.process_outgoing(t, ctx);
    }
}