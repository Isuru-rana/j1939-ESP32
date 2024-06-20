#include <driver/twai.h>
#include <esp_log.h>

#include <estd/istream.h>
#include <estd/ostream.h>
#include <estd/thread.h>

#include <embr/platform/esp-idf/usb-serial-jtag/streambuf.h>
#include <can/internal/slcan/parser.hpp>
#include <can/platform/esp-idf/transport.hpp>


#include <stdio.h>

struct twai_impl : embr::can::slcan::v0::impl::base
{
    using transport_type = embr::can::esp_idf::twai_transport<true>;

    // DEBT: Make this actual instance for TWAI v2 API
    transport_type transport() { return {}; }

    bitrates_enum bitrate_;

    bool set_bitrate(bitrates_enum v, twai_timing_config_t* config)
    {
        switch(v)
        {
            case BITRATE_250K:
                *config = TWAI_TIMING_CONFIG_250KBITS();
                return true;

            case BITRATE_500K:
                *config = TWAI_TIMING_CONFIG_500KBITS();
                return true;

            case BITRATE_1000K:
                *config = TWAI_TIMING_CONFIG_1MBITS();
                return true;

            default: return false;
        }
    }

    const char* open(bool listen_only)
    {
        twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(
            (gpio_num_t)CONFIG_GPIO_TWAI_TX,
            (gpio_num_t)CONFIG_GPIO_TWAI_RX,
            TWAI_MODE_NORMAL);

        static const twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

        twai_timing_config_t t_config;

        bool r = set_bitrate(bitrate_, &t_config);
        
        // DEBT: Do validation style soft error checking
        ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config));
        ESP_ERROR_CHECK(twai_start());

        opened_ = true;

        return "\r";
    }

    const char* close()
    {
        ESP_ERROR_CHECK(twai_stop());
        ESP_ERROR_CHECK(twai_driver_uninstall());

        return "\r";
    }

    const char* bitrate(unsigned idx, unsigned rate)
    {
        // "This command is only active if the CAN channel is closed."
        if(opened_)    return "\7";

        bitrate_ = bitrates_enum(idx);

        //return r ? "\r" : "\7";
        return "\r";
    }
};

static embr::can::slcan::v0::parser<twai_impl> parser;

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
