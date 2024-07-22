#include <j1939/internal/dispatcher/incoming2.hpp>

#include <j1939/NAME/vehicle_systems.h>
#include <j1939/NAME/function.h>
#include <j1939/NAME/industry_groups.h>

#include <j1939/pdu.h>
#include <j1939/data_field/cm1.hpp>

#include "j1939/qt/ca/cm1.h"


namespace embr::j1939::qt::ca { inline namespace v1 {

CM1::CM1(QObject* parent) :
    ControllerApplication(parent)
{
    network_.name().arbitrary_address_capable(true);
    network_.name().industry_group(int(industry_groups::on_highway));
    network_.name().function_instance(0);
    network_.name().function((int)function_fields::cab_controller);

    network_.setTag("CM1");
}

void CM1::requestFanSpeed(float percent)
{
    pdu<pgns::cm1> p(network_.address(), addresses::global, null_t{});

    //using m = j1939::spn::measured;
    auto pct = embr::units::percent<float>(percent);
    p.requested_percent_fan_speed(pct);

    send(p);
}

void CM1::frameReceived(QCanBusDevice* device, const QCanBusFrame& frame)
{
    //transport_type t{device};
    network_.frameReceived(device, frame);
}


void CM1::start(QCanBusDevice* device)
{
    connect_network(device);
}

}}
