#pragma once

#include <j1939/data_field/lighting_command.hpp>
#include <j1939/pdu.h>

#include "lcmd_sink.h"

#include <driver/gpio.h>

namespace app {

template <class Transport>
bool lcmd_sink::process_incoming(Transport&, const pdu<pgns::lighting_command>& p)
{
    set_level(CONFIG_GPIO_BRAKE, p.center_stop());
    set_level(CONFIG_GPIO_LEFT_BLINKER, p.left_turn_signal());
    set_level(CONFIG_GPIO_RIGHT_BLINKER, p.right_turn_signal());

    return true;
}

}
