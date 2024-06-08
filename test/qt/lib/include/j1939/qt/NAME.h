#pragma once

#include <QObject>

#include <j1939/NAME/name.h>

namespace embr::j1939::qt { inline namespace v1 {

class NAME
{
public: // DEBT
    layer1::NAME value_;

    Q_GADGET

    //Q_PROPERTY(unsigned vehicleSystem MEMBER value_.vehicle_system CONSTANT)
};

}}
