#pragma once

#include <QTimer>

#include <j1939/cs/base.h>
#include <j1939/cas/internal/prng_address_manager.h>
#include <j1939/state-machines/network.hpp>

#include "../transport.h"
#include "../NAME.h"

#include "base.h"

// cs = controller service/subsystem
// not a full CA, but a requisite peiece of helping a CA function

namespace embr::j1939::qt::cs { inline namespace v1 {

struct ExternalAddressObserver :
    Base,
    j1939::cs::v1::base
{
    using base_type = j1939::cs::v1::base;
    using addr_type = uint8_t;

    using base_type::process_incoming;

    pdu<pgns::address_claimed> pdu_;
    bool observed_ = false;

    bool process_incoming(can::qt_transport&, const pdu<pgns::address_claimed>&);

    Q_OBJECT

    ExternalAddressObserver(QObject* parent);

    void frameReceived(const QCanBusFrame&) override;

signals:
    // In addition to primary address acquisition duties, we also announce
    // when external addresses appear (external Network cs's operating)
    void addressObserved(addr_type, qt::v1::NAME);
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

    void updateState();

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
