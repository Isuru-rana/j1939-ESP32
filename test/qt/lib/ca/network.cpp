#include "j1939/qt/ca/network.h"

//#include <j1939/cas/internal/dispatcher.h>
#include <j1939/cas/internal/dispatcher.hpp>
//#include <j1939/ca.hpp> // DEBT: Way to get at process_incoming wrappers

namespace embr::j1939::qt::ca { inline namespace v1 {

void Network::handler()
{
    sm_.process_outgoing(transport_, clock::now());
    schedule();
    const state_type state = sm_.state();
    if(state != last_state_)
    {
        emit stateChanged(state);

        if(state == state_type::claimed)
            emit addressChanged(address());

        last_state_ = state;
    }
}


void Network::frameReceived(const QCanBusFrame& frame)
{
    //process_incoming(sm_, t, frame);
}


}}
