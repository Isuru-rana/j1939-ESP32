#include <driver/twai.h>
#include <esp_log.h>

#include <estd/istream.h>
#include <estd/ostream.h>
#include <estd/thread.h>

#include <embr/platform/esp-idf/usb-serial-jtag/streambuf.h>
#include <can/internal/slcan/parser.h>


#include <stdio.h>

static embr::can::slcan::v0::parser<> parser;

using namespace estd::chrono_literals;

estd::detail::basic_istream<embr::esp_idf::usj_streambuf<char> > cin;
estd::detail::basic_ostream<embr::esp_idf::usj_streambuf<char> > cout;

static const char* TAG = "slcan::main";

extern "C" void app_main(void)
{
    static auto config = USB_SERIAL_JTAG_DRIVER_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&config));

    unsigned counter = 0;
    char input[60];
    int input_pos = 0;

    for(;;)
    {
        if(++counter % 10 == 0)
            ESP_LOGI(TAG, "counter: %u", counter);

        int c = cin.get();

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

                const char* out = parser.parse(input);

                ESP_LOGI(TAG, "Got result: %s", out);

                cout << out << estd::endl;
            }
            else
                input[input_pos++] = c;
        }

        estd::this_thread::sleep_for(100ms);
    }
}
