#pragma once

#include <QObject>

#include <j1939/state-machines/lcmd.hpp>

#include "../ca.h"

namespace embr::j1939::qt::ca { inline namespace v1 {

class OEL : public ControllerApplication
{
    using clock = std::chrono::system_clock;

public:
    OEL(QObject* parent = nullptr) :
        ControllerApplication(parent)
    {}

    void frameReceived(QCanBusDevice*, const QCanBusFrame&) override;
};

}}
