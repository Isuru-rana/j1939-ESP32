#pragma once

#include <QObject>

#include <j1939/state-machines/lcmd.hpp>

#include "../ca.h"

namespace embr::j1939::qt::ca { inline namespace v1 {

class CCVS : public ControllerApplication,
    public j1939::cs::v1::base
{
    Q_OBJECT

public:
    CCVS(QObject* parent = nullptr);

    void frameReceived(QCanBusDevice*, const QCanBusFrame&) override;

    Q_INVOKABLE void brakeSwitchPressed();
    Q_INVOKABLE void brakeSwitchReleased();

    void start(QCanBusDevice*);
};

}}
