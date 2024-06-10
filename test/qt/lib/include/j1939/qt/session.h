#pragma once

#include <QObject>
#include <QCanBusDevice>

#include "cs/generic.h"
#include "cs/network.h"

namespace embr::j1939::qt { inline namespace v1 {

// Represents one primary transport, one cs::Generic and a set of dynamic CS/CAs.
// An optional network address resolver is available
// Bridging is TBD, but should coexist with Session
class Session : public QObject
{
    QCanBusDevice* can_ = nullptr;

    cs::v1::Generic generic_;
    cs::v1::Network network_;

    QList<cs::v1::Base*> css_;

    Q_OBJECT

    Q_PROPERTY(cs::v1::Generic* generic READ generic CONSTANT)
    Q_PROPERTY(cs::v1::Network* network READ network CONSTANT)
    Q_PROPERTY(QList<cs::v1::Base*> clients READ clients CONSTANT)

public:
    Session(QObject* parent = nullptr);

    void setDevice(QCanBusDevice*);

    cs::v1::Generic* generic() { return &generic_; }
    cs::v1::Network* network() { return &network_; }
    QList<cs::v1::Base*> clients() { return css_; }
};

}}
