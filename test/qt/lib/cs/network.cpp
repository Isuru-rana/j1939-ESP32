#include "j1939/qt/cs/network.h"

#include <j1939/dispatcher.hpp>

namespace embr::j1939::qt::cs { inline namespace v1 {



void Network::updateState()
{
    if(cached_.state(sm_))
    {
        const state_type state = sm_.state();

        qDebug()
            << this << "state:"
            << to_string(state)
            << to_string(sm_.substate())
            << "addr:" << Qt::hex << sm_.address().value();

        emit stateChanged(state, sm_.substate());

        if(state == state_type::claimed &&
            sm_.substate() == substates::elapsed)
            emit addressChanged(sm_.address().value());
    }
}

void Network::handler()
{
    time_point now = clock::now();
    //while(sm_.next_event() <= now)
    {
        //sm::v1::result r =
        sm_.process_outgoing(transport_, now);
    }
    schedule();
    updateState();
}


void Network::frameReceived(QCanBusDevice* device, const QCanBusFrame& frame)
{
    //qDebug() << this << "frameReceived";
    context_type c(clock::now());

    can::qt_transport t{device};
    v2::process_incoming(sm_, t, frame, c);
    updateState();

    // DEBT: Consider if next_event_ gets accellerated
}


void Network::start(QCanBusDevice* device)
{
    /*
        connect(device, &QCanBusDevice::framesReceived, this, [&, device]
        {
            frameReceived(device->readFrame());
        }); */
    transport_.device_ = device;
    sm_.start(transport_, clock::now());
    schedule();
    updateState();
}



ExternalAddressObserver::ExternalAddressObserver(QObject* parent) :
    Base(parent)
{

}


void ExternalAddressObserver::frameReceived(QCanBusDevice*, const QCanBusFrame& frame)
{
    embr::can::qt_transport t;

    j1939::v2::process_incoming(*this, t, frame);

    NAME test;

    test.value_ = pdu_.payload();

    //unsigned v = test.vehicleSystem();

    emit addressObserved(
        pdu_.source_address(),
        test);
}


auto ExternalAddressObserver::process_incoming(can::qt_transport&, const pdu<pgns::address_claimed>& p) -> result
{
    pdu_ = p;

    return result::ok();
}

}}
