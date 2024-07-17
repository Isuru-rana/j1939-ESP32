#pragma once

#include <QObject>
#include <QCanBusDevice>

#include "cs/generic.h"
#include "cs/network.h"
#include "cs/tp.h"

#include "runtime.h"

namespace embr::j1939::qt { inline namespace v1 {

// Represents one primary transport, one cs::Generic and a set of dynamic CS/CAs.
// An optional network address resolver is available
// Bridging is TBD, but should coexist with Session
class Session : public QObject
{
    QCanBusDevice* can_ = nullptr;

    using cs_type = cs::v1::Base*;

    cs::v1::Generic generic_;
    //cs::v1::Network network_;
    cs::v1::TransportProtocol tp_;
    Runtime* const runtime_;

    QList<cs_type> css_;
    QList<const QObject*> frameLog_;

    Q_OBJECT

    Q_PROPERTY(cs::v1::Generic* generic READ generic CONSTANT)
    //Q_PROPERTY(cs::v1::Network* network READ network CONSTANT)
    Q_PROPERTY(cs::v1::TransportProtocol* tp READ tp CONSTANT)
    Q_PROPERTY(QList<cs_type> clients READ clients CONSTANT)
    Q_PROPERTY(QList<const QObject*> frameLog READ frameLog NOTIFY frameLogChanged)
    Q_PROPERTY(Runtime* runtime MEMBER runtime_ CONSTANT)

public:
    Session(Runtime* runtime);

    void setDevice(QCanBusDevice*);

    cs::v1::Generic* generic() { return &generic_; }
    //cs::v1::Network* network() { return &network_; }
    cs::v1::TransportProtocol* tp() { return &tp_; }
    QList<cs_type>& clients() { return css_; }
    QList<const QObject*> frameLog() const { return frameLog_; }

signals:
    void frameLogChanged();
};

}}
