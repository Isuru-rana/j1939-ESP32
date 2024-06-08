#include "j1939/qt/cs/network.h"

#include <j1939/internal/dispatcher/incoming.hpp>

namespace embr::j1939::qt::cs { inline namespace v1 {


void Network::updateState()
{
    const state_type state = sm_.state();
    if(state != last_state_)
    {
        emit stateChanged(state);

        if(state == state_type::claimed)
            emit addressChanged(address());

        last_state_ = state;
    }
}

void Network::handler()
{
    sm_.process_outgoing(transport_, clock::now());
    schedule();
    updateState();
}


void Network::frameReceived(const QCanBusFrame& frame)
{
    process_incoming(sm_, transport_, frame);
    updateState();

    // DEBT: Consider if next_event_ gets accellerated
}


ExternalAddressObserver::ExternalAddressObserver(QObject* parent) :
    Base(parent)
{

}


void ExternalAddressObserver::frameReceived(const QCanBusFrame& frame)
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
