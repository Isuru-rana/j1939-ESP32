#include "transport.h"
#include "lcmd_sink.h"

namespace app {

void set_level(unsigned gpio, embr::j1939::spn::control_commands cmd)
{
    using ccmd = embr::j1939::spn::control_commands;

    // DEBT: Pay attention to errors

    switch(cmd)
    {
        case ccmd::enable:
            gpio_set_level(gpio_num_t(gpio), 1);
            break;

        case ccmd::disable:
            gpio_set_level(gpio_num_t(gpio), 0);
            break;

        default:
            break;
    }
}

}