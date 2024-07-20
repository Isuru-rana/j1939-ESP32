#include <j1939/internal/dispatcher/incoming2.hpp>

#include <j1939/NAME/vehicle_systems.h>
#include <j1939/NAME/function.h>
#include <j1939/NAME/industry_groups.h>

#include <j1939/pdu.h>
#include <j1939/data_field/bjm1.hpp>

#include "j1939/qt/ca/bjm.h"


namespace embr::j1939::qt::ca { inline namespace v1 {

BJM::BJM(QObject* parent) :
    ControllerApplication(parent)
{
    network_.name().arbitrary_address_capable(true);
    network_.name().industry_group(int(industry_groups::construction));
    network_.name().function_instance(0);
    network_.name().function((int)function_fields::joystick_control);

    network_.setTag("BJM");
}

static void adjust(pdu<pgns::bjm1>& p, unsigned group)
{
    switch(group)
    {
    case 0:
        break;

    case 1:
        p.range(uint32_t(pgns::bjm2));
        break;

    case 2:
        p.range(uint32_t(pgns::bjm3));
        break;

    default:
        break;
    }
}

void BJM::buttonPress(unsigned group, unsigned num, bool down)
{
    pdu<pgns::bjm1> p(network_.address(), null_t{});

    using m = j1939::spn::measured;

    //p.range(p.range() + 1);
    adjust(p, group);

    const m cmd = down ? m::enabled : m::disabled;

    switch(num)
    {
        case 0:
            p.button1_pressed(cmd);
            break;

        case 1:
            p.button2_pressed(cmd);
            break;

        default:
            return;
    }


    send(p);
}


void BJM::updateAxis(unsigned group, double x, double y)
{
    pdu<pgns::bjm1> p(network_.address(), null_t{});

    adjust(p, group);
}


void BJM::frameReceived(QCanBusDevice* device, const QCanBusFrame& frame)
{
    transport_type t{device};
    network_.frameReceived(device, frame);
}


void BJM::start(QCanBusDevice* device)
{
    connect_network(device);
}

}}
