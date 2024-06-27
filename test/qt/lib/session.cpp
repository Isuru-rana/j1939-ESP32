#include "j1939/qt/session.h"
#include "j1939/qt/pdu.h"

namespace embr::j1939::qt { inline namespace v1 {

Session::Session(QObject* parent) :
    QObject(parent),
    generic_(parent),
    network_(parent)
{
    connect(&generic_, &cs::v1::Generic::pduReceived, this, [&](const v1::Pdu* pdu)
    {
        if(frameLog_.size() > 100)
        {
            frameLog_.removeFirst();

            // DEBT: Likely is an expensive operation, only do this once in a while
            frameLog_.squeeze();
        }

        frameLog_.append(pdu);
        // DEBT: Clunky since QList doesn't do change events
        emit frameLogChanged();
    });
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
            tp_.frameReceived(can_, frame);

            for(cs_type cs : css_)
            {
                cs->frameReceived(can_, frame);
            }
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
