#include "j1939/qt/cs/network.h"

#include <j1939/internal/dispatcher/incoming.hpp>

namespace embr::j1939::qt::cs { inline namespace v1 {


void Network::updateState()
{
    const state_type state = sm_.state();
    if(state != last_state_)
    {
        qDebug() << this << "state=" << int(state);

        emit stateChanged(state);

        if(state == state_type::claimed)
            emit addressChanged(address());

        last_state_ = state;
    }
    else if(sm_.substate() != last_substate_)
    {
        //qDebug() << this << "substate=" << int(sm_.substate());
        emit sm_.substate();
        last_substate_ = sm_.substate();
    }
}

void Network::handler()
{
    sm_.process_outgoing(transport_, clock::now());
    schedule();
    updateState();
}


void Network::frameReceived(QCanBusDevice* device, const QCanBusFrame& frame)
{
    //qDebug() << this << "frameReceived";
    context_type c(clock::now());

    can::qt_transport t{device};
    process_incoming(sm_, t, frame, c);
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
    emit stateChanged(sm_.state());
}



ExternalAddressObserver::ExternalAddressObserver(QObject* parent) :
    Base(parent)
{

}


void ExternalAddressObserver::frameReceived(QCanBusDevice*, const QCanBusFrame& frame)
{
    embr::can::qt_transport t;

    j1939::process_incoming(*this, t, frame);

    NAME test;

    test.value_ = pdu_.payload();

    //unsigned v = test.vehicleSystem();

    emit addressObserved(
        pdu_.source_address(),
        test);
}


bool ExternalAddressObserver::process_incoming(can::qt_transport&, const pdu<pgns::address_claimed>& p)
{
    pdu_ = p;

    return true;
}

}}
