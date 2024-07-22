#include <driver/twai.h>
#include <esp_check.h>
#include <esp_log.h>

#include <estd/ostream.h>

#include <embr/platform/esp-idf/nvs.h>
#include <can/internal/slcan/parser.hpp>
#include <can/platform/esp-idf/transport.hpp>

struct twai_impl : embr::can::slcan::v0::impl::base
{
    using transport_type = embr::can::esp_idf::twai_transport<true>;

    transport_type transport_;

    // DEBT: Make this actual instance for TWAI v2 API
    transport_type& transport() { return transport_; }
    const transport_type& transport() const { return transport_; }

    static constexpr const char* TAG = "slcan::twai_impl";

    alerts_type alerts() const
    {
        uint32_t v = 0;
        uint8_t alerts = 0;

        esp_err_t ret = twai_read_alerts(&v, 0);

        // DEBT: Kind of a lie, likely twai driver isn't even online
        if(ret != ESP_OK && ret != ESP_ERR_TIMEOUT)   return ALERT_BUS_ERROR;

        if(v & TWAI_ALERT_BUS_ERROR)
        {
            alerts |= ALERT_BUS_ERROR;
        }
        if(v & TWAI_ALERT_ERR_PASS)
        {
            alerts |= ALERT_BUS_PASSIVE;
        }
        if(v & TWAI_ALERT_RX_FIFO_OVERRUN)
        {
            alerts |= ALERT_RX_FIFO_FULL;
        }
        if(v & TWAI_ALERT_ARB_LOST)
        {
            alerts |= ALERT_ARBITRATION_LOST;
        }

        return alerts_type(alerts);
    }

    static bool config_bitrate(bitrates_enum v, twai_timing_config_t* config)
    {
        ESP_LOGD(TAG, "config_bitrate: %u", bitrates_[v]);

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

        open_mode_ = listen_only ? OPEN_LISTEN : OPEN_NORMAL;

        return OK;
    }

    const char* close()
    {
        esp_err_t ret;

        (void)ret;

        ESP_GOTO_ON_ERROR(twai_stop(), err, TAG, "Cannot stop TWAI");
        ESP_GOTO_ON_ERROR(twai_driver_uninstall(), err, TAG, "Cannot uninstall TWAI");

        open_mode_ = CLOSED;

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

        nvh.set("speed", uint8_t(bitrate()));
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
