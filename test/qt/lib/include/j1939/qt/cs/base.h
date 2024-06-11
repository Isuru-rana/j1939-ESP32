#pragma once

#include <QObject>
#include <QtSerialBus>

#include "../transport.h"

namespace embr::j1939::qt::cs { inline namespace v1 {

class Base : public QObject
{
protected:
    using transport_type = can::qt_transport;

    Q_OBJECT

public:
    Base(QObject* parent) : QObject(parent) {}

public slots:
    virtual void frameReceived(QCanBusDevice*, const QCanBusFrame&) = 0;
};


}}
