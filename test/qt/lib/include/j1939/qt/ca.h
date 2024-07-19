#pragma once

#include <QObject>

#include "cs/network.h"

namespace embr::j1939::qt { inline namespace v1 {

class ControllerApplication : public cs::v1::Base
{
protected:
    cs::v1::Network network_;
    bool network_active_ = true;

    template <pgns pgn>
    bool send(const pdu<pgn>& p);

    // DEBT: Feels wrong on the whole
    void connect_network(QCanBusDevice* device)
    {
        // Generally a CA always wants its own address.  However, it may make sense
        // to stack multiple CA behaviors into one, so address acquisition can be
        // disabled.  In that case, one needs to force-set the address for this CA
        if(network_active_ == false)
        {
            network_.setTransport(transport_type{device});
            return;
        }

        QObject::connect(device, &QCanBusDevice::stateChanged, this, [&, device]
            (QCanBusDevice::CanBusDeviceState state)
        {
            if(state == QCanBusDevice::ConnectedState)
            {
                network_.start(device);
            }
        });

    }

    using clock = std::chrono::system_clock;

    Q_OBJECT

    Q_PROPERTY(cs::v1::Network* network READ network CONSTANT)

public:
    ControllerApplication(QObject* parent = nullptr) :
        cs::v1::Base(parent),
        network_{parent}
    {}

    cs::v1::Network* network()
    {
        return &network_;
    }

    const cs::v1::Network* network() const
    {
        return &network_;
    }
};

template <pgns pgn>
bool ControllerApplication::send(const pdu<pgn>& p)
{
    using traits = j1939::transport_traits<transport_type>;
    transport_type& t = network_.transport();

    if(!t.good())   return false;

    return traits::send(t, p);
}

}}
