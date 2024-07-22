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

    void schedule(time_point next_event);

    Q_OBJECT

public:
    // DEBT: Clumsy, but it will do
    static time_point startup;

    Base(QObject* parent) :
        timer_{parent},
        QObject(parent)
    {
        // DEBT: Would be nice to use coarse timer to save cycles.  However, it frequently wakes up a little early
        // which causes a small loop when we try to reschedule, it wakes up early again, etc.
        timer_.setTimerType(Qt::PreciseTimer);
        timer_.setSingleShot(true);
    }

public slots:
    virtual void frameReceived(QCanBusDevice*, const QCanBusFrame&) = 0;
};


}}
