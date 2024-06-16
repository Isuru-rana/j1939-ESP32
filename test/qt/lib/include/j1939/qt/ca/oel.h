#pragma once

#include <QObject>

#include <j1939/state-machines/lcmd.hpp>

#include "../ca.h"

namespace embr::j1939::qt::ca { inline namespace v1 {

class OEL : public ControllerApplication,
    public j1939::cs::v1::base
{
    using clock = std::chrono::system_clock;

    Q_OBJECT

public:
    OEL(QObject* parent = nullptr);

    using j1939::cs::v1::base::process_incoming;

    void frameReceived(QCanBusDevice*, const QCanBusFrame&) override;

    Q_INVOKABLE void hazardPressed();
    Q_INVOKABLE void leftSignal();
    Q_INVOKABLE void rightSignal();

    void start(QCanBusDevice*);

    bool process_incoming(can::qt_transport&, const pdu<pgns::lcmd>&);
};

}}
