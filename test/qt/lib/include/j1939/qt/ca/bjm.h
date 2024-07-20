#pragma once

#include <QObject>

#include <j1939/state-machines/lcmd.hpp>

#include "../ca.h"

namespace embr::j1939::qt::ca { inline namespace v1 {

class BJM : public ControllerApplication,
    public j1939::cs::v1::base
{
    Q_OBJECT

public:
    BJM(QObject* parent = nullptr);

    using j1939::cs::v1::base::process_incoming;

    void frameReceived(QCanBusDevice*, const QCanBusFrame&) override;

    // DEBT: Needs better variable names
    Q_INVOKABLE void buttonPress(unsigned group, unsigned num, bool down);
    ///
    /// @brief updateAxis
    /// @param group
    /// @param x -100 to 100
    /// @param y -100 to 100
    Q_INVOKABLE void updateAxis(unsigned group, double x, double y);

    void start(QCanBusDevice*);

    result process_incoming(can::qt_transport&, const pdu<pgns::lcmd>&);
};

}}
