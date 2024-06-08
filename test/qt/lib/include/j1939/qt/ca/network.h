#pragma once

#include <QTimer>

#include <j1939/cas/internal/prng_address_manager.h>
#include <j1939/state-machines/network.hpp>

#include "../transport.h"

namespace embr::j1939::qt::ca { inline namespace v1 {

// DEBT: On reflection, this isn't really a CA.  This is more a frequent prerequisite
// to a CA's normal operation.  Perhaps we need a new term like CS for support pieces
// which a particular CA may need (controller services? controller support?).  As far
// as Qt is concerned these aren't exactly state machines, since they are more
// self sufficient

class Base : public QObject
{
    Q_OBJECT

public:
    Base(QObject* parent) : QObject(parent) {}

public slots:
    virtual void frameReceived(const QCanBusFrame&) = 0;
};

class Network : public Base
{
    using clock = std::chrono::system_clock;
    using milliseconds = std::chrono::milliseconds;
    using addr_type = uint8_t;
    using state_type = sm::v1::network_enum::states;

    layer1::NAME name_;
    QTimer timer_;
    using addrmgr_type = internal::prng_address_manager;
    sm::v1::network<addrmgr_type, clock::time_point> sm_;
    can::qt_transport transport_;

    void schedule()
    {
        const clock::time_point now = clock::now();

        // DEBT: Need better way to determine if a future schedule
        // is requested.  This ought to do for the short term though
        if(sm_.next_event() < now)  return;

        milliseconds interval(
            std::chrono::duration_cast<milliseconds>(
                sm_.next_event() - now));
        timer_.start(interval);
    }

    state_type last_state_ = state_type::unstarted;

    Q_OBJECT

    Q_PROPERTY(addr_type address READ address NOTIFY addressChanged)
    Q_PROPERTY(state_type state READ state NOTIFY stateChanged)

public:
    Network(QObject* parent = nullptr) :
        Base(parent),
        timer_{parent},
        sm_{addrmgr_type{}, name_}
    {
        timer_.setSingleShot(true);
        connect(&timer_, &QTimer::timeout, this, &Network::handler);
    }

    void start(QCanBusDevice* device)
    {
        connect(device, &QCanBusDevice::framesReceived, this, [&, device]
        {
            frameReceived(device->readFrame());
        });
        transport_.device_ = device;
        sm_.start(transport_, clock::now());
        schedule();
        emit stateChanged(sm_.state());
    }

    addr_type address() const { return *sm_.address(); }
    state_type state() const { return sm_.state(); }

    void frameReceived(const QCanBusFrame&) override;

signals:
    void stateChanged(state_type);
    void addressChanged(addr_type);

private slots:
    void handler();
};

}}
