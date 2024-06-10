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

    Q_OBJECT

    Q_PROPERTY(cs::v1::Generic* generic READ generic CONSTANT)

public:
    Session(QObject* parent = nullptr);

    void setDevice(QCanBusDevice*);

    cs::v1::Generic* generic() { return &generic_; }
};

}}
