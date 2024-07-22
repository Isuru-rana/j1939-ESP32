#pragma once

#include <QObject>

#include "../ca.h"

namespace embr::j1939::qt::ca { inline namespace v1 {

class CM1 : public ControllerApplication,
    public j1939::cs::v1::base
{
    Q_OBJECT

public:
    CM1(QObject* parent = nullptr);

    void frameReceived(QCanBusDevice*, const QCanBusFrame&) override;

    Q_INVOKABLE void requestFanSpeed(float percent);

    void start(QCanBusDevice*);
};

}}
