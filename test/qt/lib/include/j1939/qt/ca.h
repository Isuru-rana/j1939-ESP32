#pragma once

#include <QObject>

#include "cs/network.h"

namespace embr::j1939::qt { inline namespace v1 {

class ControllerApplication : public cs::v1::Base
{
    cs::v1::Network network_;

public:
    ControllerApplication(QObject* parent = nullptr) :
        cs::v1::Base(parent),
        network_{parent}
    {}
};

}}
