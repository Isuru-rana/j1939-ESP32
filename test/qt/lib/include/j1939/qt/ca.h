#pragma once

#include <QObject>

#include "cs/network.h"

namespace embr::j1939::qt { inline namespace v1 {

class ControllerApplication : public cs::v1::Base
{
protected:
    cs::v1::Network network_;

    Q_PROPERTY(cs::v1::Network* network READ network CONSTANT)

public:
    ControllerApplication(QObject* parent = nullptr) :
        cs::v1::Base(parent),
        network_{parent}
    {}

    const cs::v1::Network* network() const
    {
        return &network_;
    }
};

}}
