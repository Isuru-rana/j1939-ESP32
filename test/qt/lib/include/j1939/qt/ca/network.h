#pragma once

#include <QTimer>

#include <j1939/cas/internal/prng_address_manager.h>
#include <j1939/state-machines/network.hpp>

#include "../transport.h"

namespace embr::j1939::qt::ca {

class Network
{
    using clock = std::chrono::system_clock;

    layer1::NAME name_;
    QTimer timer_;
    using addrmgr_type = internal::prng_address_manager;
    sm::v1::network<addrmgr_type, clock::time_point> sm_;
    can::qt_transport transport_;

public:
    Network() :
        sm_{addrmgr_type{}, name_}
    {
    }

    void start(QCanBusDevice* device)
    {
        transport_.device_ = device;
        sm_.start(transport_, clock::now());
    }
};

}
