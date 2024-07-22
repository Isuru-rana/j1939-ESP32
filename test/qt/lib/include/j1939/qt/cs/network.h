#pragma once

#include <QTimer>

#include <j1939/cs/base.h>
#include <j1939/cas/internal/prng_address_manager.h>
#include <j1939/state-machines/network/network.hpp>

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

    result process_incoming(can::qt_transport&, const pdu<pgns::address_claimed>&);

    Q_OBJECT

    ExternalAddressObserver(QObject* parent);

    void frameReceived(QCanBusDevice*, const QCanBusFrame&) override;

signals:
    // In addition to primary address acquisition duties, we also announce
    // when external addresses appear (external Network cs's operating)
    void addressObserved(addr_type, qt::v1::NAME);
};

// Network address acquisition mechanism
class Network : public Base
{
    using base_type = Base;
    using state_type = sm::v1::network_enum::states;
    using substates = sm::v1::network_enum::substates;
    using substate_type = substates;
    // DEBT: Eventually we want true RNG version here
    using addrmgr_type = internal::prng_address_manager;
    using sm_type = sm::v1::network<addrmgr_type, time_point>;
    using context_type = sm_type::context<time_point>;

    //layer1::NAME name_;
    sm_type sm_;
    transport_type transport_;

    // To retain last state so we can notify when state changes
    j1939::sm::v0::network_cached cached_;

    // Human readable indicator which CA is associated with this address
    QString tag_;

    void updateState();

    void schedule()
    {
        // DEBT: Need better way to determine if a future schedule
        // is requested.  This ought to do for the short term though
        base_type::schedule(sm_.next_event());
    }

    Q_OBJECT

    Q_PROPERTY(addr_type address READ address NOTIFY addressChanged)
    Q_PROPERTY(state_type state READ state NOTIFY stateChanged)
    Q_PROPERTY(substates substate READ substate NOTIFY stateChanged)
    Q_PROPERTY(bool isClaimed READ isClaimed NOTIFY stateChanged)
    Q_PROPERTY(QString tag READ tag CONSTANT)

public:
    // DEBT: Dedup these two constructors
    Network(QObject* parent = nullptr) :
        Base(parent),
        sm_{addrmgr_type{}, layer1::NAME{j1939::null_t{}}}
    {
        connect(&timer_, &QTimer::timeout, this, &Network::handler);
    }

    Network(layer1::NAME name, QObject* parent = nullptr) :
        Base(parent),
        sm_{addrmgr_type{}, name}
    {
        connect(&timer_, &QTimer::timeout, this, &Network::handler);
    }

    void start(QCanBusDevice* device);

    addr_type address() const { return *sm_.address(); }
    state_type state() const { return sm_.state(); }
    substates substate() const { return sm_.substate(); }
    bool isClaimed() const { return sm_.state() == state_type::claimed; }
    layer1::NAME& name() { return sm_.name(); }
    const layer1::NAME& name() const { return sm_.name(); }
    transport_type& transport() { return transport_; }

    // NOTE: Awkwardness here, we almost never value-assign transport.  However, it's
    // acceptable usage.
    void setTransport(const transport_type& v)
    {
        transport_ = v;
    }

    void setTag(const QString& v)
    {
        tag_ = v;
        qDebug() << this << "tag" << v;
    }
    QString tag() const { return tag_; }

    void frameReceived(QCanBusDevice*, const QCanBusFrame&) override;

signals:
    void stateChanged(state_type, substate_type);
    void addressChanged(addr_type);

private slots:
    void handler();
};

}}
