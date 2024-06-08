#pragma once

#include <QObject>
#include <QtSerialBus>

namespace embr::j1939::qt::cs { inline namespace v1 {

class Base : public QObject
{
    Q_OBJECT

public:
    Base(QObject* parent) : QObject(parent) {}

public slots:
    virtual void frameReceived(const QCanBusFrame&) = 0;
};


}}
