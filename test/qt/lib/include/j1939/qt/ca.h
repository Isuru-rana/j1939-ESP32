#pragma once

#include <QObject>

#include "cs/network.h"

namespace embr::j1939::qt { inline namespace v1 {

class ControllerApplication : public cs::v1::Base
{
protected:
    cs::v1::Network network_;

    template <pgns pgn>
    bool send(const pdu<pgn>& p);

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

template <pgns pgn>
bool ControllerApplication::send(const pdu<pgn>& p)
{
    using traits = j1939::transport_traits<can::qt_transport>;

    return traits::send(network_.transport(), p);
}

}}
