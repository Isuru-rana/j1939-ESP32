#include "j1939/qt/cs/network.h"

#include <j1939/dispatcher.hpp>

namespace embr::j1939::qt::cs { inline namespace v1 {


const char* to_string(j1939::sm::network_base::states v)
{
    using s = j1939::sm::network_base::states;

    switch(v)
    {
        case s::unstarted:      return "Unstarted";
        case s::requesting:     return "Requesting";
        case s::claiming:       return "Claiming";
        case s::claimed:        return "Claimed";
        case s::claim_failed:   return "Claim Failed";
        default:                return "N/A";
    }
}


const char* to_string(j1939::sm::network_base::substates v)
{
    using s = j1939::sm::network_base::substates;

    switch(v)
    {
        case s::claim_waiting:      return "claim_waiting";
        case s::contending:     return "contending";
        case s::expired:       return "expired";
        case s::waiting:        return "waiting";
        case s::cannot_claim_waiting:   return "Claim cannot_claim_waiting";
        default:                return "N/A";
    }
}


void Network::updateState()
{
    const state_type state = sm_.state();
    if(state != last_state_)
    {
        qDebug() << this << "state" << to_string(state);

        emit stateChanged(state);

        if(state == state_type::claimed)
            emit addressChanged(address());

        last_state_ = state;
    }
    else if(sm_.substate() != last_substate_)
    {
        qDebug() << this << "substate" << to_string(sm_.substate());
        emit sm_.substate();
        last_substate_ = sm_.substate();
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
    emit stateChanged(sm_.state());
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
