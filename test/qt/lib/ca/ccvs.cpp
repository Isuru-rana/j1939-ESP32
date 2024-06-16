#include "j1939/qt/ca/ccvs.h"

namespace embr::j1939::qt::ca { inline namespace v1 {

CCVS::CCVS(QObject* parent) :
    ControllerApplication(parent)
{
    network_.name().arbitrary_address_capable(true);
    network_.name().function_instance(0);
    network_.name().function((int)function_fields::cab_controller);
}

void CCVS::frameReceived(QCanBusDevice*, const QCanBusFrame&)
{

}


void CCVS::brakeSwitchPressed()
{

}


void CCVS::brakeSwitchReleased()
{

}

void CCVS::start(QCanBusDevice* device)
{
    connect_network(device);
}

}}
