#include "platform/transport.hpp"

#if defined(ESP_PLATFORM)
namespace embr { namespace can { namespace esp_idf {

namespace internal {

// DEBT: This really ought to go into embr proper (and perhaps is there
// already -- though 'service' area does not quite count... unless we
// wanna wrap all that up here)
void twai_transport::init()
{
    static twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(
        (gpio_num_t)CONFIG_GPIO_TWAI_TX,
        (gpio_num_t)CONFIG_GPIO_TWAI_RX,
        TWAI_MODE_NORMAL);  // DEBT: Pull this in from config
    static const twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    static const twai_timing_config_t t_config =
#if CONFIG_TWAI_TIMING == 125
        TWAI_TIMING_CONFIG_125KBITS();
#elif CONFIG_TWAI_TIMING == 250
        TWAI_TIMING_CONFIG_250KBITS();
#elif CONFIG_TWAI_TIMING == 500
        TWAI_TIMING_CONFIG_500KBITS();
#elif CONFIG_TWAI_TIMING == 800
        TWAI_TIMING_CONFIG_800KBITS();
#elif CONFIG_TWAI_TIMING == 1000
        TWAI_TIMING_CONFIG_1MBITS();
#else
#error Unsupported TWAI timing
#endif

    ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config));
    ESP_ERROR_CHECK(twai_start());
}

}

}}}
#endif
