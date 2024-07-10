#include <driver/twai.h>
#include <esp_check.h>
#include <esp_log.h>

#include <estd/istream.h>
#include <estd/ostream.h>
#include <estd/thread.h>

#include <embr/platform/esp-idf/usb-serial-jtag/streambuf.h>

#include "twai_slcan.h"


static embr::can::slcan::v0::parser<twai_impl> parser;

using namespace estd::chrono_literals;

estd::detail::basic_istream<embr::esp_idf::usj_streambuf<char> > cin;
estd::detail::basic_ostream<embr::esp_idf::usj_streambuf<char> > cout;

static const char* TAG = "slcan::main";

#ifdef CONFIG_ESP_CONSOLE_SECONDARY_NONE
#define ENABLE_USJ 1
#endif

extern "C" void app_main(void)
{
    // Don't enable usj mode unless secondary console is disabled
#if ENABLE_USJ
    static auto config = USB_SERIAL_JTAG_DRIVER_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&config));
#endif

    // Guidance from
    // https://github.com/espressif/esp-idf/blob/v5.2.2/examples/storage/nvs_rw_value/main/nvs_value_example_main.c

    esp_err_t err = nvs_flash_init();

    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    unsigned counter = 0, frame_counter = 0;
    char input[60];
    int input_pos = 0;

    parser.autopoll(true);
    parser.init();

    ESP_ERROR_CHECK(twai_reconfigure_alerts(
        TWAI_ALERT_ALL,     // | TWAI_ALERT_AND_LOG,
        nullptr));

    for(;;)
    {
        twai_message_t frame;
        esp_err_t ret;

        // DEBT: Needs cleanup along with delay down below
        if(parser.cimpl().opened())
        {
            do
            {
                ret = twai_receive(&frame, 0);
                if(ret == ESP_OK)
                {
                    ++frame_counter;
                    parser.on_receive(frame, cout);
                }

            }   while(ret == ESP_OK);
        }

        // FIX: Eating app all alerts right away as we diagnose hardware
        // issues
        auto a = parser.cimpl().alerts();

        if(a != 0)
        {
            estd::layer1::string<128> s;
            to_string(a, s.data());
            ESP_LOGW(TAG, "alert: %s", s.data());
        }

        if(++counter % 20 == 0)
            ESP_LOGI(TAG, "counter: %u frames: %u autopoll: %u",
                counter, frame_counter, parser.autopoll());

#if ENABLE_USJ
        int c = cin.get();
#else
        // DEBT: Non-USJ not yet supported except in pure diagnostic mode (no USB comms at all)
        int c = -1;
#endif

        if(c != -1)
        {
            if(c == 10)
            {
                // DEBT: Ignore LFs
            }
            else if(c == 13)
            {
                input[input_pos] = 0;
                input_pos = 0;
                
                ESP_LOGI(TAG, "Got command: %s", input);

                parser.parse(input, cout);

                //ESP_LOGI(TAG, "Got result: %s", out);

                //cout << out << estd::endl;
            }
            else
                input[input_pos++] = c;
        }
        else
            estd::this_thread::sleep_for(50ms);
    }
}
