#include <j1939/qt/session.h>

namespace embr::j1939::qt { inline namespace v1 {

Session::Session(QObject* parent) :
    QObject(parent),
    generic_(parent),
    network_(parent)
{

}


void Session::setDevice(QCanBusDevice* device)
{
    can_ = device;

    QObject::connect(device, &QCanBusDevice::framesReceived, device, [&]
    {
        while(can_->framesAvailable() > 0)
        {
            QCanBusFrame frame = can_->readFrame();

            //qDebug() << "Got frame:" << Qt::hex << frame.frameId();

            generic_.frameReceived(can_, frame);
            network_.frameReceived(can_, frame);
        }
    });

    QObject::connect(device, &QCanBusDevice::stateChanged, [&]
        (QCanBusDevice::CanBusDeviceState state)
    {
        if(state == QCanBusDevice::ConnectedState)
        {
            network_.start(can_);
        }
    });
}

}}
