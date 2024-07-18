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

void BJM::buttonPress(unsigned group, unsigned num, bool down)
{
    pdu<pgns::bjm1> p(network_.address(), null_t{});

    using m = j1939::spn::measured;

    m cmd = down ? m::enabled : m::disabled;

    /*
    j1939::spn::control_commands cmd = down ?
        j1939::spn::control_commands::enable :
        j1939::spn::control_commands::disable;
*/

    p.button1_pressed(cmd);

    send(p);
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
