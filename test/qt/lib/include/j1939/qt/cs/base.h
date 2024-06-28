#pragma once

#include <QObject>
#include <QtSerialBus>

#include "../transport.h"

namespace embr::j1939::qt::cs { inline namespace v1 {

class Base : public QObject
{
protected:
    using transport_type = can::qt_transport;
    using clock = std::chrono::system_clock;
    using milliseconds = std::chrono::milliseconds;
    using time_point = clock::time_point;
    using addr_type = uint8_t;

    QTimer timer_;

    void schedule(time_point next_event)
    {
        const time_point now = clock::now();

        if(next_event < now)  return;

        milliseconds interval(
            std::chrono::duration_cast<milliseconds>(
                next_event - now));
        timer_.start(interval);
    }

    Q_OBJECT

public:
    Base(QObject* parent) :
        timer_{parent},
        QObject(parent)
    {
        timer_.setSingleShot(true);
    }

public slots:
    virtual void frameReceived(QCanBusDevice*, const QCanBusFrame&) = 0;
};


}}
