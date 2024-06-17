#pragma once

#include <j1939/cs/base.h>

namespace app {

class lcmd_sink : public embr::j1939::cs::v1::base
{
    using base_type = embr::j1939::cs::v1::base;

public:
    using base_type::process_incoming;
    
    template <class Transport>
    bool process_incoming(Transport&, const pdu<pgns::lighting_command>&);
};

}
