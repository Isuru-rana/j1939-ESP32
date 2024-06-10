#pragma once

#include <QObject>
#include <QtSerialBus>

#include <j1939/cs/base.h>
#include <j1939/pgn/ostream.h>  // DEBT: For traits_wrapper

#include "../transport.h"
#include "../data_field.h"
#include "base.h"

namespace embr::j1939::qt::cs { inline namespace v1 {

// Wraps up incoming PDUs into something QML can sort of recognize.  Does no other processing
class Generic : public Base,
    public j1939::cs::v1::base
{
    using base_type = j1939::cs::v1::base;

    using base_type::process_incoming;

    Q_OBJECT

public:
    Generic(QObject* parent = nullptr) : Base(parent)   {}

    void frameReceived(const QCanBusFrame&) override;

    template <pgns pgn>
    bool process_incoming(can::qt_transport&, const pdu<pgn>& p);

signals:
    void pduReceived(const DataField&);
};

}}
