#pragma once

#include <QObject>

#include <j1939/state-machines/lcmd.hpp>

#include "../ca.h"

namespace embr::j1939::qt::ca { inline namespace v1 {

// Listens for OEL and emits LCMD
class LightingCommand : public ControllerApplication
{
    using clock = std::chrono::system_clock;
    using sm_type = sm::v0::lighting_command<clock::time_point>;
    using states = sm_type::states;

    QTimer timer_;
    sm_type lcmd_;
    using context = sm_type::context;
    states last_state_;

    void handler();
    void schedule();
    void updateState()
    {
        if(last_state_ == lcmd_.state())    return;

        emit stateChanged(lcmd_.state());

        last_state_ = lcmd_.state();
    }

    Q_OBJECT

    Q_PROPERTY(states state READ state NOTIFY stateChanged)

public:
    LightingCommand(QObject* parent = nullptr);

    void frameReceived(QCanBusDevice*, const QCanBusFrame&) override;

    void start(QCanBusDevice* device);

    states state() const { return lcmd_.state(); }

signals:
    void stateChanged(states);
};

}}
