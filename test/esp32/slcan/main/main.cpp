#include <driver/twai.h>
#include <esp_check.h>
#include <esp_log.h>

#include <estd/istream.h>
#include <estd/ostream.h>
#include <estd/thread.h>

#include <embr/platform/esp-idf/usb-serial-jtag/streambuf.h>
#include <embr/platform/esp-idf/nvs.h>
#include <can/internal/slcan/parser.hpp>
#include <can/platform/esp-idf/transport.hpp>

#include "twai_slcan.h"


struct twai_impl : embr::can::slcan::v0::impl::base
{
    using transport_type = embr::can::esp_idf::twai_transport<true>;

    // DEBT: Make this actual instance for TWAI v2 API
    transport_type transport() { return {}; }

    static constexpr const char* TAG = "slcan::twai_impl";

    alerts_type alerts() const
    {
        uint32_t v;
        
        twai_read_alerts(&v, 0);

        return {};
    }

    static bool config_bitrate(bitrates_enum v, twai_timing_config_t* config)
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
            listen_only ? TWAI_MODE_LISTEN_ONLY : TWAI_MODE_NORMAL);

        static const twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

        twai_timing_config_t t_config;

        bool r = config_bitrate(bitrate_, &t_config);

        if(r == false) return ERROR;
        
        // DEBT: Do validation style soft error checking
        ESP_ERROR_CHECK_WITHOUT_ABORT(
            twai_driver_install(&g_config, &t_config, &f_config));

        if(twai_start() != ESP_OK)   return ERROR;

        openmode_ = listen_only ? OPEN_LISTENONLY : OPEN_NORMAL;

        return OK;
    }

    const char* close()
    {
        esp_err_t ret;

        (void)ret;

        ESP_GOTO_ON_ERROR(twai_stop(), err, TAG, "Cannot stop TWAI");
        ESP_GOTO_ON_ERROR(twai_driver_uninstall(), err, TAG, "Cannot uninstall TWAI");

        openmode_ = CLOSED;

        return OK;
    
    err:
        return ERROR;
    }

    static constexpr const char* nvs_ns = "slcan::v1";

    const char* autostart(autostart_modes mode)
    {
        embr::esp_idf::nvs::Handle nvh;
        esp_err_t err;

        err = nvh.open(nvs_ns, NVS_READWRITE);

        if(err != ESP_OK)   return ERROR;

        nvh.set("speed", uint8_t(bitrate_));
        nvh.set("autostart", uint8_t(mode));

        nvh.close();

        return OK;
    }

    // Returns NONE if NVS errors occur
    autostart_modes autostart(embr::esp_idf::nvs::Handle nvh) const
    {
        uint8_t mode;
        
        switch(nvh.get("autostart", &mode))
        {
            case ESP_OK:
                return autostart_modes(mode);

            case ESP_ERR_NVS_NOT_FOUND:
            default:
                return AUTOSTART_NONE;
        }
    }

    autostart_modes autostart() const
    {
        embr::esp_idf::nvs::Handle nvh;

        esp_err_t err = nvh.open(nvs_ns, NVS_READONLY);

        autostart_modes mode = autostart(nvh);

        nvh.close();

        return mode;
    }

    void init()
    {
        embr::esp_idf::nvs::Handle nvh;

        esp_err_t err = nvh.open(nvs_ns, NVS_READONLY);

        autostart_modes mode = autostart(nvh);
        uint8_t v;

        ESP_LOGD(TAG, "init: autostart mode=%u", mode);

        // DEBT: Technically autostart seems to require disallowing of poll mode (demands autopoll/auto send)
        // but it seems also that linux slcand tools presume autopoll/autosend is always on anyway
        switch(mode)
        {
            case AUTOSTART_NORMAL:
                if((err = nvh.get("speed", &v)) == ESP_OK)
                {
                    bitrate(bitrates_enum(v), 0);
                    open(false);
                }
                else
                    ESP_LOGW(TAG, "init: unable to autostart");

                break;

            case AUTOSTART_LISTEN:
                if((err = nvh.get("speed", &v)) == ESP_OK)
                {
                    bitrate(bitrates_enum(v), 0);
                    open(true);
                }
                else
                    ESP_LOGW(TAG, "init: unable to autostart");

                break;

            case AUTOSTART_NONE:
            default:
                break;
        }

        nvh.close();
    }
};

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

        estd::this_thread::sleep_for(100ms);
    }
}
