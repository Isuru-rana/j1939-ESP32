#pragma once

#include <QTimer>

#include <j1939/cas/internal/prng_address_manager.h>
#include <j1939/state-machines/network.hpp>

#include "../transport.h"

namespace embr::j1939::qt::ca { inline namespace v1 {

class Network : public QObject
{
    using clock = std::chrono::system_clock;
    using milliseconds = std::chrono::milliseconds;

    layer1::NAME name_;
    QTimer timer_;
    using addrmgr_type = internal::prng_address_manager;
    sm::v1::network<addrmgr_type, clock::time_point> sm_;
    can::qt_transport transport_;

    void schedule()
    {
        milliseconds interval(
            std::chrono::duration_cast<milliseconds>(
                sm_.next_event() - clock::now()));
        timer_.start(interval);
    }

    Q_OBJECT

public:
    Network(QObject* parent = nullptr) :
        QObject(parent),
        timer_{parent},
        sm_{addrmgr_type{}, name_}
    {
        timer_.setSingleShot(true);
        connect(&timer_, &QTimer::timeout, this, &Network::handler);
    }

    void start(QCanBusDevice* device)
    {
        transport_.device_ = device;
        sm_.start(transport_, clock::now());
        schedule();
    }

private slots:
    void handler();
};

}}
