#pragma once

#include <j1939/data_field/lighting_command.hpp>
#include <j1939/pdu.h>

#include "lcmd_sink.h"

namespace app {

template <class Transport>
bool lcmd_sink::process_incoming(Transport&, const pdu<pgns::lighting_command>& p)
{
    return true;
}

}
