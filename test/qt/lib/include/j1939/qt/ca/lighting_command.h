#pragma once

#include <QObject>

#include <j1939/state-machines/lcmd.hpp>

#include "../ca.h"

namespace embr::j1939::qt::ca { inline namespace v1 {

class LightingCommand : public ControllerApplication
{
    using clock = std::chrono::system_clock;

    QTimer timer_;
    sm::v0::lighting_command<clock::time_point> lcmd_;

public:
    LightingCommand(QObject* parent = nullptr) :
        ControllerApplication(parent),
        timer_(parent)
    {}

    void frameReceived(QCanBusDevice*, const QCanBusFrame&) override;
};

}}
