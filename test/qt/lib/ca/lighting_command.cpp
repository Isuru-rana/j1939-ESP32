#include <j1939/internal/dispatcher/incoming.hpp>
#include <j1939/internal/dispatcher/incoming2.hpp>
#include <j1939/state-machines/lcmd.hpp>

#include <j1939/NAME/function.h>

#include "j1939/qt/ca/lighting_command.h"


namespace embr::j1939::qt::ca { inline namespace v1 {

LightingCommand::LightingCommand(QObject* parent) :
    ControllerApplication(parent),
    timer_(parent)
{
    network_.name().arbitrary_address_capable(true);
    network_.name().function(int(function_fields::body_controller));
    network_.name().industry_group(int(industry_groups::on_highway));

    network_.setTag("LCMD");

    connect(&timer_, &QTimer::timeout, this, &LightingCommand::handler);
    timer_.setSingleShot(true);
}


void LightingCommand::schedule()
{
    auto interval = lcmd_.next_event() - clock::now();
    estd::chrono::milliseconds i2(interval);
    timer_.start(i2);
}

void LightingCommand::frameReceived(QCanBusDevice* device, const QCanBusFrame& frame)
{
    transport_type t{device};
    network_.frameReceived(device, frame);
    context c(clock::now(), network_.address());
    v2::process_incoming(lcmd_, t, frame, c);
    if(lcmd_.state() == lcmd_.STATE_FLASH_ON && last_state_ != states::STATE_IDLE)
    {
        schedule();
    }
    updateState();
}


void LightingCommand::handler()
{
    context c(clock::now(), network_.address());
    lcmd_.process_outgoing(network_.transport(), c);
    schedule();
    updateState();
}


void LightingCommand::start(QCanBusDevice* device)
{
    connect_network(device);
}

}}
