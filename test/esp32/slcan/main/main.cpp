#include <driver/twai.h>
#include <esp_check.h>
#include <esp_log.h>
#include <nvs_flash.h>

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

    bitrates_enum bitrate_ { BITRATE_UNSET };

    static constexpr const char* TAG = "slcan::twai_impl";

    static bool set_bitrate(bitrates_enum v, twai_timing_config_t* config)
    {
        switch(v)
        {
            case BITRATE_125K:
                *config = TWAI_TIMING_CONFIG_125KBITS();
                return true;

            case BITRATE_250K:
                *config = TWAI_TIMING_CONFIG_250KBITS();
                return true;

            case BITRATE_500K:
                *config = TWAI_TIMING_CONFIG_500KBITS();
                return true;

            case BITRATE_800K:
                *config = TWAI_TIMING_CONFIG_800KBITS();
                return true;

            case BITRATE_1000K:
                *config = TWAI_TIMING_CONFIG_1MBITS();
                return true;

            default:
                ESP_LOGW(TAG, "Unrecognized bitrate: #%d", v);
                return false;
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

        if(r == false) return ERROR;
        
        // DEBT: Do validation style soft error checking
        ESP_ERROR_CHECK_WITHOUT_ABORT(
            twai_driver_install(&g_config, &t_config, &f_config));

        if(twai_start() != ESP_OK)   return ERROR;

        opened_ = true;

        return OK;
    }

    const char* close()
    {
        esp_err_t ret;

        ESP_GOTO_ON_ERROR(twai_stop(), err, TAG, "Cannot stop TWAI");
        ESP_GOTO_ON_ERROR(twai_driver_uninstall(), err, TAG, "Cannot uninstall TWAI");

        opened_ = false;

        return OK;
    
    err:
        return ERROR;
    }

    const char* bitrate(unsigned idx, unsigned rate)
    {
        bitrate_ = bitrates_enum(idx);

        return OK;
    }

    const char* autostart(int mode)
    {
        return ERROR;
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

    nvs_handle_t nvh;

    err = nvs_open("slcan::v1", NVS_READWRITE, &nvh);

    // Not ready yet
    if(err == ESP_OK)
    {
        uint8_t v;

        err = nvs_get_u8(nvh, "speed", &v);
        err = nvs_get_u8(nvh, "autostart", &v);

        switch(v)
        {
            case 0:         // default, waits for host
                break;

            case 1:         // auto opens in normal mode
                break;

            case 2:         // auto opens in listen mode
                break;

            default: break;
        }
    }

    unsigned counter = 0, frame_counter = 0;
    char input[60];
    int input_pos = 0;

    parser.autopoll(true);

    for(;;)
    {
        twai_message_t frame;
        esp_err_t ret;

        // DEBT: Needs cleanup along with delay down below
        if(parser.cimpl().opened())
        {
            ret = twai_receive(&frame, 0);
            if(ret == ESP_OK)
            {
                ++frame_counter;
                parser.on_receive(frame, cout);
            }
        }

        if(++counter % 10 == 0)
            ESP_LOGI(TAG, "counter: %u frames: %u autopoll: %u",
                counter, frame_counter, parser.autopoll());

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

                parser.parse(input, cout);

                //ESP_LOGI(TAG, "Got result: %s", out);

                //cout << out << estd::endl;
            }
            else
                input[input_pos++] = c;
        }

        estd::this_thread::sleep_for(100ms);
    }
}
