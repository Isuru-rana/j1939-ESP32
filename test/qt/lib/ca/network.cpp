#include "j1939/qt/ca/network.h"

namespace embr::j1939::qt::ca { inline namespace v1 {

void Network::handler()
{
    sm_.process_outgoing(transport_, clock::now());
    schedule();
}

}}
