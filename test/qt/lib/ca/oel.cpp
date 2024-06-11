#include <j1939/internal/dispatcher/incoming.hpp>
#include <j1939/state-machines/lcmd.hpp>

#include <j1939/NAME/vehicle_systems.h>
#include <j1939/NAME/function.h>
#include <j1939/NAME/industry_groups.h>

#include <j1939/pdu.h>
#include <j1939/data_field/oel.hpp>

#include "j1939/qt/ca/oel.h"


namespace embr::j1939::qt::ca { inline namespace v1 {

OEL::OEL(QObject* parent) :
    ControllerApplication(parent)
{
    network_.name().arbitrary_address_capable(true);
    network_.name().function_instance(0);
    network_.name().function((int)function_fields::cab_controller);
}

void OEL::frameReceived(QCanBusDevice* device, const QCanBusFrame& frame)
{
    transport_type t{device};
    network_.frameReceived(device, frame);
    //process_incoming(lcmd_, t, frame);
}

void OEL::leftSignal()
{
    pdu<pgns::oel> p(network_.address());

    send(p);
}


void OEL::rightSignal()
{
    pdu<pgns::oel> p(network_.address());

    send(p);
}


void OEL::start(QCanBusDevice* device)
{
    QObject::connect(device, &QCanBusDevice::stateChanged, [&, device]
        (QCanBusDevice::CanBusDeviceState state)
    {
        if(state == QCanBusDevice::ConnectedState)
        {
            network_.start(device);
        }
    });
}

}}
